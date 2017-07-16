/* handler.c: HTTP Request Handlers */

#include "spidey.h"

#include <errno.h>
#include <limits.h>
#include <string.h>

#include <dirent.h>
#include <unistd.h>

/* Internal Declarations */
http_status handle_browse_request(struct request *request);
http_status handle_file_request(struct request *request);
http_status handle_cgi_request(struct request *request);
http_status handle_error(struct request *request, http_status status);

/**
 * Handle HTTP Request
 *
 * This parses a request, determines the request path, determines the request
 * type, and then dispatches to the appropriate handler type.
 *
 * On error, handle_error should be used with an appropriate HTTP status code.
 **/
http_status
handle_request(struct request *r)
{
    http_status result = HTTP_STATUS_OK;

    /* Parse request */
    parse_request(r);

    /* Determine request path */
    r->path = determine_request_path(r->uri);
    debug("HTTP REQUEST PATH: %s", r->path);

    /* Dispatch to appropriate request handler type */
    switch(determine_request_type(r->path)){
        case REQUEST_BROWSE:
            debug("BROWSE");
            result = handle_browse_request(r);
            break;
        case REQUEST_FILE:
            debug("FILE");
            result = handle_file_request(r);
            break;
        case REQUEST_CGI:
            debug("CGI");
            result = handle_cgi_request(r);
            break;
        default:
            debug("ERROR");
            handle_error(r, HTTP_STATUS_NOT_FOUND);
            break;
    }

    log("HTTP REQUEST STATUS: %s", http_status_string(result));
    return result;
}

/**
 * Handle browse request
 *
 * This lists the contents of a directory in HTML.
 *
 * If the path cannot be opened or scanned as a directory, then handle error
 * with HTTP_STATUS_NOT_FOUND.
 **/
http_status
handle_browse_request(struct request *r)
{
    debug("handle_browse_request");
    struct dirent **entries;

    /* Open a directory for reading or scanning */
    debug("scandir");
    int nfiles = scandir(r->path, &entries, NULL, alphasort);
    if(nfiles <= 0){
        fprintf(stderr, "Directory could not be scanned: %s", strerror(errno));
        return HTTP_STATUS_NOT_FOUND;
    }

    /* Write HTTP Header with OK Status and text/html Content-Type */
    debug("header");
    fprintf(r->file, "HTTP/1.0 %s\r\nContent-Type: text/html\r\n\r\n", http_status_string(HTTP_STATUS_OK));

    /* For each entry in directory, emit HTML list item */
    debug("entries");
    fprintf(r->file, "<html><head>");
    fprintf(r->file, "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\" integrity=\"sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u\" crossorigin=\"anonymous\">");
    fprintf(r->file, "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap-theme.min.css\" integrity=\"sha384-rHyoN1iRsVXV4nD0JutlnGaslCJuC7uwjduW9SVrLvRYooPp2bWYgmgJQIXwl/Sp\" crossorigin=\"anonymous\">");
    fprintf(r->file, "<script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js\" integrity=\"sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa\" crossorigin=\"anonymous\"></script>");
    fprintf(r->file, "<style>body { background-image: url(\"http://wallpapercave.com/wp/02IiKRa.jpg\"); }\n.well { margins: 20px 50px 0px 50px; } </style>");
    fprintf(r->file, "</head><body><ul class=\"well\" style=\"margins: 20px 50px 0px 50px;\">");
    for(int i=0; i<nfiles; i++){
        if(streq(entries[i]->d_name, ".")) continue;
        if(streq(r->uri, "/")){
            fprintf(r->file, "<li><a href=\"/%s\">%s</a></li>", entries[i]->d_name, entries[i]->d_name);
        }else{
            fprintf(r->file, "<li><a href=\"%s/%s\">%s</a></li>", r->uri, entries[i]->d_name, entries[i]->d_name);
        }
        free(entries[i]);
    }
    fprintf(r->file, "</ul></body></html>");
    free(entries);
    /* Flush socket, return OK */
    debug("flush");
    fflush(r->file);

    return HTTP_STATUS_OK;
}

/**
 * Handle file request
 *
 * This opens and streams the contents of the specified file to the socket.
 *
 * If the path cannot be opened for reading, then handle error with
 * HTTP_STATUS_NOT_FOUND.
 **/
http_status
handle_file_request(struct request *r)
{
    FILE *fs;
    char buffer[BUFSIZ];
    char *mimetype = NULL;
    size_t nread;

    /* Open file for reading */
    fs = fopen(r->path, "r");

    /* Determine mimetype */
    mimetype = determine_mimetype(r->path);

    /* Write HTTP Headers with OK status and determined Content-Type */
    fprintf(r->file, "HTTP/1.0 %s\r\nContent-Type: %s\r\n\r\n", http_status_string(HTTP_STATUS_OK), mimetype);

    /* Read from file and write to socket in chunks */
    while((nread = fread(buffer, sizeof(char), BUFSIZ, fs))){
        fwrite(buffer, sizeof(char), nread, r->file);
    }

    /* Close file, flush socket, deallocate mimetype, return OK */
    fclose(fs);
    fflush(r->file);
    free(mimetype);
    return HTTP_STATUS_OK;
}

/**
 * Handle file request
 *
 * This popens and streams the results of the specified executables to the
 * socket.
 *
 *
 * If the path cannot be popened, then handle error with
 * HTTP_STATUS_INTERNAL_SERVER_ERROR.
 **/
http_status
handle_cgi_request(struct request *r)
{
    FILE *pfs;
    char buffer[BUFSIZ];
    struct header *header;

    /* Export CGI environment variables from request:
    * http://en.wikipedia.org/wiki/Common_Gateway_Interface */
    setenv("REQUEST_METHOD", r->method, 1);
    setenv("REQUEST_URI", r->uri, 1);
    setenv("QUERY_STRING", r->query, 1);
    setenv("REMOTE_PORT", r->port, 1);
    setenv("REMOTE_ADDR", r->host, 1);
    setenv("DOCUMENT_ROOT", RootPath, 1);
    setenv("SCRIPT_FILENAME", r->path, 1);
    setenv("SERVER_PORT", Port, 1);

    /* Export CGI environment variables from request headers */
    header = r->headers;
    while(header){
        if(streq(header->name, "Host")){
            setenv("HTTP_HOST", header->value, 1);
        }else if(streq(header->name, "User-Agent")){
            setenv("HTTP_USER_AGENT", header->value, 1);
        }else if(streq(header->name, "Accept")){
            setenv("HTTP_ACCEPT", header->value, 1);
        }else if(streq(header->name, "Accept-Language")){
            setenv("HTTP_ACCEPT_LANGUAGE", header->value, 1);
        }else if(streq(header->name, "Accept-Encoding")){
            setenv("HTTP_ACCEPT_ENCODING", header->value, 1);
        }else if(streq(header->name, "Connection")){
            setenv("HTTP_CONNECTION", header->value, 1);
        }
        header = header->next;
    }

    /* POpen CGI Script */
    pfs = popen(r->path, "r");
    if(pfs==NULL){
        fprintf(stderr, "popen Failed: %s\n", strerror(errno));
        return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

    /* Copy data from popen to socket */
    while(fgets(buffer, BUFSIZ, pfs)){
        fputs(buffer, r->file);
    }

    /* Close popen, flush socket, return OK */
    pclose(pfs);
    fflush(r->file);
    return HTTP_STATUS_OK;
}

/**
 * Handle displaying error page
 *
 * This writes an HTTP status error code and then generates an HTML message to
 * notify the user of the error.
 **/
http_status
handle_error(struct request *r, http_status status)
{
    const char *status_string = http_status_string(status);

    /* Write HTTP Header */
    fprintf(r->file, "HTTP/1.0 %s\r\n", status_string);
    fprintf(r->file, "Content-Type: text/html\r\n\r\n");

    /* Write HTML Description of Error*/
    fprintf(r->file, "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\" integrity=\"sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u\" crossorigin=\"anonymous\">");
    fprintf(r->file, "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap-theme.min.css\" integrity=\"sha384-rHyoN1iRsVXV4nD0JutlnGaslCJuC7uwjduW9SVrLvRYooPp2bWYgmgJQIXwl/Sp\" crossorigin=\"anonymous\">");
    fprintf(r->file, "<script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js\" integrity=\"sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa\" crossorigin=\"anonymous\"></script>");
    fprintf(r->file, "<style>body { background-image: url(\"http://wallpapercave.com/wp/02IiKRa.jpg\"); }\n.well { margins: 20px 50px 0px 50px; } </style>");
    fprintf(r->file, "<h1>%s</h1>\n", status_string);
    fprintf(r->file, "<br><img src=\"http://www.ohnitsch.net/wordpress/wp-content/uploads/2007/11/ohoh.png\">");
    fflush(r->file);

    /* Return specified status */
    return status;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */

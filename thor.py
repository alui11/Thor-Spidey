#!/usr/bin/env python2.7

import multiprocessing
import os
import requests
import sys
import time

# Globals

PROCESSES = 1
REQUESTS  = 1
VERBOSE   = False
URL       = None

# Functions

def usage(status=0):
    print '''Usage: {} [-p PROCESSES -r REQUESTS -v] URL
    -h              Display help message
    -v              Display verbose output

    -p  PROCESSES   Number of processes to utilize (1)
    -r  REQUESTS    Number of requests per process (1)
    '''.format(os.path.basename(sys.argv[0]))
    sys.exit(status)

def do_request(pid):
    total_time = 0
    for i in range(0, REQUESTS):
        t = time.time()
        r = requests.get(URL)
        if VERBOSE:
            print r.text
        t = time.time() - t
        print "Process: {}, Request: {}, Elapsed Time: {}".format(pid, i, round(t,2))
        total_time += t
    average = total_time/REQUESTS
    print "Process: {}, AVERAGE   , Elapsed Time: {}".format(pid, round(average,2))
    return average

# Main execution

if __name__ == '__main__':
    # Parse command line arguments
    args = sys.argv[1:]
    while len(args) and args[0].startswith('-') and len(args[0]) > 1:
        arg = args.pop(0)
        if arg == '-v':
            VERBOSE = True
        elif arg == '-p':
            PROCESSES = int(args.pop(0))
            if PROCESSES<1:
                print "Processes must be greater than 0"
                sys.exit(1)
        elif arg == '-r':
            REQUESTS = int(args.pop(0))
            if REQUESTS<1:
                print "Requests must be greater than 0"
                sys.exit(1)
        elif arg == '-h':
            usage(0)
        else:
            usage(1)

    if len(args) != 1:
        usage(1)

    URL = args.pop(0);

    # Create pool of workers and perform requests
    AVG = 0
    if PROCESSES > 1:
        pool = multiprocessing.Pool(PROCESSES)
        averages = pool.map(do_request, range(PROCESSES))
        AVG = sum(averages)/len(averages)
    else:
        AVG = do_request(0)

    print "TOTAL AVERAGE ELAPSED TIME: {}".format(round(AVG,2))

# vim: set sts=4 sw=4 ts=8 expandtab ft=python:

Thor-Spidey
===================

A class project to practice system calls related to sockets and networking. Includes a basic HTTP server that supports directory listings, static files, and CGI scripts (spidey.c) and a basic HTTP client (thor.py).

Members
-------

- Alison Lui (alui@nd.edu)
- Ivy Wang (jwang28@nd.edu)

Summary
-------
We got everything working, including single and forking, and handling each file type. We pair programmed the entire assignment, so didn't divide up the work.

Latency
-------
To test the latency we used thor to hammer spidey by calling thor on the appropriate url: 10 requests each for a directory, static file, and CGI script. We did that twice, once with Spidey running in single mode, and once with spidey running in forking mode. Thor gave us the average time taken for the requests.

| Mode     | Type      | Average time (10 requests) |
|----------|-----------|----------------------------|
| Single   | Directory | 0.02                       |
|          | Static    | 0.02                       |
|          | CGI       | 0.68                       |
| Forking  | Directory | 0.02                       |
|          | Static    | 0.02                       |
|          | CGI       | 1.16                       |

Throughput
----------
We used dd to create three static files, of size 1KB, 1MB, and 1GB. We used thor to request each of these files 10 times each, and did that process twice, once with spidey in single mode, and once with spidey in forking mode. We then divided number of bytes by the average time taken to determine bytes per second.

| Mode     | Type      | Throughput (10 requests)   |
|----------|-----------|----------------------------|
| Single   | 1 KB      | 1024/0.02 = 51200 bytes/s  |
|          | 1 MB      | 1.049e6/0.03 = 3.50e7      |
|          | 1 GB      | 1e9/9.32 = 1.07e8          |
| Forking  | 1 KB      | 1024/0.02 = 51200          |
|          | 1 MB      | 1.049e6/0.03 = 3.50e8      |
|          | 1 GB      | 1e9/9.62 = 1.04e8          |

Analysis
--------
For latency, directory and static are so quick that there is not an observable difference between them and between single and forking. The CGI scripts are slower, and forking seems to be slower for that. The cost of forking must overcome the cost of making a request on the CGI script. There is enough overhead that it doesn't actually make it faster. If it was a huge intense CGI script that might be different.
For throughput, there isn't an observable difference between single and forking for the smaller two files sizes since they are pretty small. 1MB is slightly slower than 1KB, but since it is so much larger, the throughput apppears to be much better. Single and forking are also similar for 1GB (although forking seems to be slightly worse) and the throughput seems to be best of all for the largest file. Throughput may be better for larger files since there is some overhead for accepting and handling the request, but once the file is open and being read, it isn't that much more work to read a bigger file than a smaller file, so it would make sense that the average throughput would be better for a big file since the initial connection is a smaller portion of the total time taken. The cost of forking is apparently still not worth it for a 1GB file, but it is about the same. Forking would be able to get more things done at once, but there is a certain amount of overhead that goes along with cloning a process and dealing with child processes and such.

Conclusion
----------
We learned that forking, even though it sounds great since you can deal with more requests at once, is not always worth the overhead when you aren't dealing with that many requests. We also learned about how exactly the client and the server communicate with each other by learning exactly how to parse the request and execute the appropriate action based on the request, and how to send the client the information it needs in the right format so it can deal with it. It was very neat to see how the http protocal works and see how could send html to a browswer sending to strings to the socket stream.

Contributions
-------------
We pair programmed the entire assignment.

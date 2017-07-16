#!/usr/bin/env python2.7

import os

# addr = ['http://student00.cse.nd.edu:9998', 'http://student00.cse.nd.edu:9998/text/hackers.txt', 'http://student00.cse.nd.edu:9998/scripts/cowsay.sh?message=Hello&template=default']

addr = ['http://student00.cse.nd.edu:9998/upload_test.txt', 'http://student00.cse.nd.edu:9998/upload_test1.txt', 'http://student00.cse.nd.edu:9998/upload_test2.txt'] 

for a in addr:
	for i in range(0, 10):
		os.system('./thor.py '+a)
	print


#!/usr/bin/env python2.7

import multiprocessing

def f (pid):
	print "Process: {}".format(pid)


p = multiprocessing.Pool()
p.map(f, range(6))

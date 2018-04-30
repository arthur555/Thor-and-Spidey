#!/usr/bin/env python3

import multiprocessing
import os
import requests
import sys
import time
import platform
# Globals

PROCESSES = 1
REQUESTS  = 1
VERBOSE   = False
URL       = None

args= sys.argv[1:]
# Functions
TOTALL = None
def usage(status=0):
    print('''Usage: {} [-p PROCESSES -r REQUESTS -v] URL
    -h              Display help message
    -v              Display verbose output

    -p  PROCESSES   Number of processes to utilize (1)
    -r  REQUESTS    Number of requests per process (1)
    '''.format(os.path.basename(sys.argv[0])))
    sys.exit(status)

def do_request(pid):
    total = 0
    p = os.path.basename(URL)
    for i in range(REQUESTS):
        time_start = time.time()
        r = requests.get(URL)
        if(VERBOSE):
            print(r.text)
        time_end = time.time()
        time_elapsed = time_end - time_start
        total+= time_elapsed
        print('Process: {}, Request: {}, Elapsed Time: {}'.format( pid, i, round(time_elapsed, 2)))
    print("Process: {}, AVERAGE   , Elapsed Time: {}".format( pid, round(total/REQUESTS,2)))
#    print("here: total{}".format(total))
  #  with TOTALL.get_lock():
   #     TOTALL.value+= total
    #print("here:TOTALL{}".format(TOTALL.value))
    return total/REQUESTS

    
    ''' Perform REQUESTS HTTP requests and return the average elapsed time. '''
#def init(args):
#    global TOTALL
#    TOTALL = args
# Main execution

if __name__ == '__main__':
    if (not args):
        usage(1)
    while args and args[0].startswith('-') and len(args[0]) > 1:
        arg = args.pop(0)
        if arg == '-h':
            usage(0)
        elif arg == '-v':
            VERBOSE = True
        elif arg == '-p':
            PROCESSES = int(args.pop(0))
        elif arg == '-r':
            REQUESTS = int(args.pop(0))
        else:
            usage(1)
    if args:
        URL = args.pop(0)
   # if(VERBOSE):
   #     p = os.path.basename(URL)
   #     f = open(p, "r")
   #     print(f.read())
        #for i in strr:
        #    print(i)
        

    #TOTALL = Value('f',0)
    
    #p = Pool(initializer = init, initargs =(TOTALL, ))
    #i = p.map_async(do_request, range(REQUESTS), chunksize = PROCESSES)
    #i.wait()

    pool = multiprocessing.Pool(PROCESSES)
    listt =(pool.map(do_request, range(PROCESSES)))
    
    print("TOTAL AVERAGE ELAPSED TIME: {0:.2f}".format((sum(listt)/PROCESSES)))
    #i.get()
    #print (i.get())
#    with TOTALL.get_lock():
    
#        print(TOTALL.value)
#        print("TOTAL AVERAGE ELAPSED TIME: {}".format(TOTALL.value/PROCESSES))
    # Parse command line arguments

    # Create pool of workers and perform requests
    

# vim: set sts=4 sw=4 ts=8 expandtab ft=python:

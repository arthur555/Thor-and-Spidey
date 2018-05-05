#!/usr/bin/env python3

import os
import time
import subprocess

# Variables


#nitems = ["/html/1k.txt","/html/1M.txt","/html/1G.txt","/html/index.html", "/scripts/cowsay.sh", "/scripts/env.sh", "/text/hackers.txt", "/text/lyrics.txt" ]
nitems = ["/html/index.html", "/scripts/cowsay.sh", "/scripts/env.sh", "/text/hackers.txt", "/text/lyrics.txt" ]
processes = [1,2,3,4]
requests = [2,3]
# Heading
print("| {:7}| {:7}| {:21}| {:11}| {:14}|".format("process","request","uri", "latency","throughput"))
print("|{:-<8}|{:-<8}|{:-<22}|{:-<12}|{:-<15}|".format("-","-","-", "-", "-"))

# Running
max_run_time = 60
FAILED = False

for nitem in nitems:
	for i in processes:
		for j in requests:
			start = time.time()
			end = start + max_run_time
			interval = max_run_time / 1000.0

		#Run Commanda
			command = "./thor.py -r " + str(j) + " -p "+ str(i) +" http://student06.cse.nd.edu:9004" + nitem

			p = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
		#Check if still waiting
			while True:
				result = p.poll()
				if result is not None:
					break
				if time.time() >= end:
					p.kill()
					print("| {:21}| {:11}| {:14}|".format(nitem, "inf", "inf"))
					FAILED = True
					break
				time.sleep(interval)
		
			if not FAILED:
				endd = time.time()
				size = os.path.getsize("/afs/nd.edu/user40/lyokum/gitlab_projects/systems_programming/cse-20289-sp18-project/www"+nitem)
				stream, err  = p.communicate()#.stdout.read().split()
				stream = stream.decode().split()
				timme = round(endd-start,5)
				print("| {:7}| {:7}| {:21}| {:11}| {:14}M|".format(i, j, nitem, timme, round(size/timme/1024/1024,5)))

			FAILED = False

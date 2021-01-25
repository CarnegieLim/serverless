from __future__ import print_function
from bcc import BPF
from ctypes import c_ushort, c_int, c_ulonglong
from time import sleep
from sys import argv

import subprocess

#def usage():
#	print("USAGE: %s [interval [count]]" % argv[0])
#	exit()

# arguments
#interval = 5
#count = -1
#if len(argv) > 1:
#	try:
#		interval = int(argv[1])
#		if interval == 0:
#			raise
#		if len(argv) > 2:
#			count = int(argv[2])
#	except:	# also catches -h, --help
#		usage()

# load BPF program
b = BPF(src_file = "timing.c")
b.attach_kprobe(event="dup_mm", fn_name="do_dup_mm_entry")
b.attach_kretprobe(event="dup_mm", fn_name="do_dup_mm_return")

#b.attach_kprobe(event="_do_fork", fn_name="do_clone_entry")
#b.attach_kretprobe(event="_do_fork", fn_name="do_clone_return")


sizes = ['0', '1', '2', '4', '8', '16', '32', '64', '128', '256', '512', '1024', '2048']
modes = ['copy', 'share']

for size in sizes:
    for mode in modes:

        args_perf = ["/usr/bin/perf", "record", "-F", "4999", "-g", "-o", "out.record", "./my_clone", mode, size]
        proc_perf = subprocess.Popen(args_perf, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=None)
        proc_perf.stdin.write('\n'.encode())
        proc_perf.stdin.flush()
        stdout, stderr = proc_perf.communicate()
        print(stdout)

        f = open("out.perf", "w")
        args_script = ["/usr/bin/perf", "script", "-i", "out.record"]
        proc_script = subprocess.Popen(args_script, stdin=None, stdout=f, stderr=None)
        #proc_perf.stdin.write('\n'.encode())
        #proc_perf.stdin.flush()
        #stdout, stderr = proc_perf.communicate()
        #print(stdout)

        proc_script.wait()

        f = open("out.folded", "w")
        args_folded = ["./FlameGraph/stackcollapse-perf.pl", "out.perf"]
        proc_folded = subprocess.Popen(args_folded, stdin=None, stdout=f, stderr=None)
        proc_folded.wait()


        f = open("out_%s_%s.svg" % (mode, size), "w")
        args_flame = ["./FlameGraph/flamegraph.pl", "out.folded"]
        proc_flame = subprocess.Popen(args_flame, stdin=None, stdout=f, stderr=None)
        proc_flame.wait()
    
        args = ['./my_clone', mode, size]
        print("%s mode, %s MB Heap Size" % (mode, size))
        proc = subprocess.Popen(args, 
                            stdin=subprocess.PIPE, 
                            stdout=subprocess.PIPE, 
                            stderr=subprocess.PIPE)

        proc.stdin.write('\n'.encode())
        proc.stdin.flush()

        stdout, stderr = proc.communicate()
        print(stdout)
        #print(stderr)

# header
#print("Tracing... Hit Ctrl-C to end.")

# output
#loop = 0
#do_exit = 0
#while (1):
#	if count > 0:
#		loop += 1
#		if loop > count:
#			exit()
#	try:
#		sleep(interval)
#	except KeyboardInterrupt:
#		pass; do_exit = 1

        sleep(1)

#        print("clone_dist")
#        b["clone_dist"].print_linear_hist("usecs")
        #b["clone_dist"].clear()

        print("dup_mm_dist")
        b["dup_mm_dist"].print_linear_hist("usecs")

#        b["clone_dist"].clear()
        b["dup_mm_dist"].clear()

#	if do_exit:
#		exit()

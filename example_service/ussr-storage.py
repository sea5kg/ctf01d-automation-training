#!/usr/bin/env python
# -*- coding: utf-8 -*-
 
import socket
import threading
import sys
import math
import re
import os
import errno

host = ""
port = 4445
thrs = []

class Connect(threading.Thread):
	def __init__(self, sock, addr):
		self.sock = sock
		self.addr = addr
		self.bKill = False
		threading.Thread.__init__(self)
	def run (self):
		help_s = """
USSR Storage v1.0.0
Commands:
    HELP              This list of commands
    PUT <id> <data>   Put item to storage (data: [A-Za-z0-9_\\-]{1,})
    GET <id>          Get item by id from storage
    LIST              List of items
    EXIT              Exit
"""

		ptrn = re.compile(r""".*(?P<name>\w*?).*""", re.VERBOSE)
		self.sock.send(help_s)
		while True:
			if self.bKill == True:
				break
			self.sock.send("> ");
			buf = self.sock.recv(1024)
			if buf == "":
				break
			buf = buf.strip()
			pattern_put = re.compile("^[ ]*PUT[ ]*([A-Za-z0-9]{1,})[ ]*([A-Za-z0-9_\\-]{1,})[ ]*$")
			pattern_get = re.compile("^[ ]*GET[ ]*([A-Za-z0-9]{1,})[ ]*$")
			pattern_del = re.compile("^[ ]*DEL[ ]*([A-Za-z0-9]{1,})[ ]*$")
			pattern_list = re.compile("^[ ]*LIST[ ]*$")
			pattern_help = re.compile("^[ ]*HELP[ ]*$")
			pattern_exit = re.compile("^[ ]*EXIT[ ]*$")

			if pattern_exit.match(buf):
				self.sock.send("BYE-BYE\n")
				break
			elif pattern_help.match(buf):
				self.sock.send(help_s)
				continue
			elif pattern_list.match(buf):
				c = 0
				for f in os.listdir('flags/'):
					self.sock.send("    " + f + "\n")
					c = c + 1
				self.sock.send("  FOUND " + str(c) + " ITEM(S)\n")
				continue
			elif pattern_put.match(buf):
				f_id = pattern_put.match(buf).group(1);
				f_data = pattern_put.match(buf).group(2);
				if f_id == "":
					self.sock.send("FAIL incorrect id\n")
					continue
				f = open('flags/'+f_id, 'w')
				f.write(f_data)
				f.close()
				self.sock.send("OK\n")
				continue
			elif pattern_get.match(buf):
				f_id = pattern_get.match(buf).group(1);
				if f_id == "":
					self.sock.send("FAIL incorrect id\n")
					continue
				if os.path.exists('flags/' + f_id):
					f = open('flags/' + f_id, 'r')
					line = f.readline()
					f.close()
					self.sock.send("DATA " + line + "\n")
				else:
					self.sock.send("FAIL id not found\n")
				continue
			elif pattern_del.match(buf):
				f_id = pattern_del.match(buf).group(1);
				if f_id == "":
					self.sock.send("FAIL incorrect id\n")
					continue
				if os.path.exists('flags/' + f_id):
					os.remove('flags/' + f_id);
					self.sock.send("OK\n")
				else:
					self.sock.send("FAIL id not found\n")
				continue
			else:
				self.sock.send("FAIL ["+ buf.strip() + "] Unknown command, look 'help'\n")
				continue
				
		self.bKill = True
		self.sock.close()
		thrs.remove(self)
	def kill(self):
		if self.bKill == True:
			return
		self.bKill = True
		self.sock.close()
		# thrs.remove(self)
 
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind((host, port))
s.listen(10)
print('Started server on ' + str(port) + ' port.');

if not os.path.exists("flags"):
	os.makedirs("flags")
        
try:
	while True:
		sock, addr = s.accept()
		thr = Connect(sock, addr)
		thrs.append(thr)
		thr.start()
except KeyboardInterrupt:
    print('Bye! Write me letters!')
    s.close()
    for thr in thrs:
		thr.kill()
    

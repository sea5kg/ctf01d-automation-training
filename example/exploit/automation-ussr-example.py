#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys, traceback, math, socket, re, random, os
import httplib2
from os import listdir
from os.path import isfile, join
import time
from user_token import user_token

def timep():
	return time.strftime('%X %x %Z');

host="automation-ussr.freehackquest.com"
port=4445
jury_sys="http://automation-ussr.freehackquest.com/"


print("Connecting...")
# search new flag id
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host, port))
s.recv(2048)
print("Connected")
print("send LIST")
s.send("LIST\n")
ids=""
# search for example: "FOUND 4 ITEM(S)"
pattern_found = re.compile(".*FOUND \d+ ITEM\(S\).*", re.MULTILINE)
while(True):
	part = s.recv(2048)
	print("Recived " + part)
	# print part
	ids = ids + part
	matches = [m.groups() for m in pattern_found.finditer(ids)]
	if(len(matches) > 0):
		break;
#print ids
print(ids)
s.send("send EXIT\n")
print("EXIT")
s.close()

if not os.path.exists("flags/processed"):
    os.makedirs("flags/processed")

if not os.path.exists("flags/doget"):
    os.makedirs("flags/doget")
    
if not os.path.exists("flags/sended"):
    os.makedirs("flags/sended")

pattern_id = re.compile(r'\s{4}([A-Za-z0-9]+)[\n\r]+', re.MULTILINE)
matches = [m.groups() for m in pattern_id.finditer(ids)]
# print matches
for m in matches:
	id = m[0].strip();
	if (not os.path.exists("flags/processed/"+id)
		and not os.path.exists("flags/doget/"+id)
		and not os.path.exists("flags/sended/"+id)
	):
		# create new file in do get
		print 'Found new id: %s' % id
		file = open('flags/doget/' + id, 'w+')
		file.close();

# processed all flags in folder doget
ids_doget = [f for f in listdir("flags/doget/") if isfile(join("flags/doget/", f))]

print("Processing... (get flags by id)")
pattern_data = re.compile(".*DATA (.*)")
pattern_fail = re.compile(".*FAIL .*")
if(len(ids_doget) > 0):
	print("Connecting...")
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((host, port))
	print("Connected")
	s.recv(2048)
	for id_doget in ids_doget:
		os.remove("flags/doget/"+id_doget)
		# print id_doget
		s.send("GET " + id_doget + "\n")
		response = s.recv(1024).strip();
		if(not pattern_data.match(response) and not pattern_fail.match(response)):
			response = s.recv(1024).strip()

		if(pattern_fail.match(response)):
			print response
		
		if(pattern_data.match(response)):
			flag = pattern_data.match(response).group(1).strip();
			print("FLAG: " + id_doget + " => " + flag);
			file = open('flags/processed/' + id_doget, 'w+')
			file.write(flag);
			file.close();
	s.send("EXIT\n");
	s.close();	

ids_processed = [f for f in listdir("flags/processed/") if isfile(join("flags/processed/", f))]
print("Sending... (send flags by id)")
if(len(ids_processed) > 0):
	for id_processed in ids_processed:
		file = open('flags/processed/' + id_processed, 'r')
		flag=file.read().replace('\n', '')
		file.close();
		s_url=jury_sys + "api/v1/flag/?token=" + user_token + "&flag=" + flag
		print("request " + s_url)
		h = httplib2.Http(".cache")
		resp, content = h.request(s_url, "GET")
		if(resp.status == 200):
			print("FLAG SENDED: " + id_processed + " => " + flag);
			os.remove("flags/processed/"+id_processed)
			file = open('flags/sended/' + id_processed, 'w+')
			file.write(flag);
			file.close();
		else:
			print("FLAG NOT SENDED: " + id_processed + " => " + flag + " resp.status " + str(resp.status) + "");

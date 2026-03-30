#!/usr/bin/env python3
# The MIT License (MIT)
#
# Copyright (c) 2015-2026 Evgenii Sopov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# https://github.com/sea5kg/ctf01d-automation-training
#

"""
    Example checker-script
"""

import sys, traceback
import math
import socket
import re
import random
import signal

if len(sys.argv) < 5:
    sys.exit('Usage: %s (PUT|CHECK) <host> <id> <flag>' % sys.argv[0])

command = sys.argv[1]
host = sys.argv[2]
id = sys.argv[3]
flag = sys.argv[4]
port = 4445

if(command == "PUT"):
    code = 200;
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((host, port))
        s.recv(2048)
        s.send("PUT " + id + " " + flag + "\n")
        result = s.recv(1024)
        pattern_fail = re.compile(".*FAIL .*")
        pattern_ok = re.compile(".*OK.*")

        if(not pattern_ok.match(result) and not pattern_fail.match(result)):
            result = s.recv(1024).strip();

        if(not pattern_ok.match(result)):
            s.close()
            raise Exception
        print("[OK]")
        s.close()
    except:
        print("[CURRUPT]")
        code = 500
    sys.exit(code)

if(command == "CHECK"):
    code = 200;
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((host, port))
        s.recv(1024)
        s.send("GET " + id + "\n")
        result = s.recv(1024).strip();
        pattern_fail = re.compile(".*FAIL .*")
        pattern_flag = re.compile(".*DATA (.*)")

        if(not pattern_flag.match(result) and not pattern_fail.match(result)):
            result = s.recv(1024).strip();

        if(pattern_fail.match(result)):
            s.close()
            raise Exception

        if(pattern_flag.match(result)):
            flag2 = pattern_flag.match(result).group(1).strip();
            if(flag2 != flag):
                s.close()
                raise Exception
            else:
                print("[OK]")
        s.close()
    except:
        print("[CURRUPT]")
        code = 500
    sys.exit(code)

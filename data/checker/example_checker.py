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

import sys
import socket
import re
import errno
import traceback

SERVICE_PORT = 4445
ENABLE_DEBUG = False
RE_FAIL = re.compile(".*FAIL .*")
RE_OK = re.compile(".*OK.*")
RE_FLAG = re.compile(".*DATA (.*)")


def service_up():
    """
    put-get flag to service success
    """
    print("[service is worked] - 101")
    sys.exit(101)


def service_corrupt():
    """
    service is available (available tcp connect),
    but protocol wrong could not put/get flag
    """
    print("[service is corrupt] - 102")
    sys.exit(102)


def service_mumble():
    """
    waited time (for example: 5 sec) but service did not have time to reply
    """
    print("[service is mumble] - 103")
    sys.exit(103)


def service_down():
    """
    service is not available (maybe blocked port or service is down)
    """
    print("[service is down] - 104")
    sys.exit(104)


def debug(_err):
    """ debug function """
    if ENABLE_DEBUG:
        if isinstance(err, str):
            err = Exception(err)
        traceback.print_exc()
        raise err


def put_flag(_host, _flag_id, _flag_value):
    """ put flag to service """
    try:
        cli_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        cli_sock.settimeout(1)
        cli_sock.connect((_host, SERVICE_PORT))
        cli_sock.recv(2048)  # read welcome
        _put_cmd = "PUT " + str(_flag_id) + " " + str(_flag_value) + "\n"
        cli_sock.send(_put_cmd.encode("utf-8"))
        result = cli_sock.recv(1024).decode("utf-8")
        if not RE_FAIL.match(result) and not RE_OK.match(result):
            result = cli_sock.recv(1024).decode("utf-8").strip()
        if not RE_OK.match(result):
            cli_sock.close()
            service_corrupt()
        cli_sock.close()
        service_up()
    except socket.timeout:
        service_down()
    except socket.error as _err:
        if _err.errno == errno.ECONNREFUSED:
            service_down()
        else:
            debug(_err)
            service_corrupt()
    except Exception as _err:  # pylint: disable=broad-exception-caught
        debug(_err)
        service_corrupt()


def check_flag(_host, _flag_id, _flag_value):
    """ check flag on service """
    try:
        cli_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        cli_sock.settimeout(1)
        cli_sock.connect((_host, SERVICE_PORT))
        cli_sock.recv(2048)  # read welcome
        _put_cmd = "GET " + str(_flag_id) + "\n"
        cli_sock.send(_put_cmd.encode("utf-8"))
        result = cli_sock.recv(1024).decode("utf-8")
        if not RE_FLAG.match(result) and not RE_FAIL.match(result):
            result = cli_sock.recv(1024).decode("utf-8").strip()
        if RE_FAIL.match(result):
            cli_sock.close()
            print(result)
            service_corrupt()

        if RE_FLAG.match(result):
            resp_flag = RE_FLAG.match(result).group(1).strip()
            if resp_flag != _flag_value:
                cli_sock.close()
                print("Flag overrode or some thing else")
                service_corrupt()
        cli_sock.close()
        service_up()
    except socket.timeout:
        service_down()
    except socket.error as _err:
        if _err.errno == errno.ECONNREFUSED:
            service_down()
        else:
            debug(_err)
            service_corrupt()
    except Exception as _err:  # pylint: disable=broad-exception-caught
        debug(_err)
        service_corrupt()


if len(sys.argv) < 5:
    sys.exit(f'Usage: {sys.argv[0]} (PUT|CHECK) <host> <id> <flag>')

command = sys.argv[1]
host = sys.argv[2]
flag_id = sys.argv[3]
flag_value = sys.argv[4]

if command == "PUT":
    put_flag(host, flag_id, flag_value)
elif command == "CHECK":
    check_flag(host, flag_id, flag_value)
else:
    sys.exit("Unknown command '" + command + "'")

sys.exit(1)

#!/usr/bin/env python3
#
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

import socket
import threading
import sys
import math
import re
import os
import errno
from enum import Enum

SERVICE_LISTEN_HOST = ""
SERVICE_LISTEN_PORT = 4445
THREADS_WITH_CLIENT = []

class ClientSocket:
    """ Implementation under work with socket """

    def __init__(self, accepted_sock, _addr):
        self.__accepted_sock = accepted_sock
        self.__addr = _addr
        print(self.get_address_info())

    def get_address_info(self):
        """ return src ip:port """
        return self.__addr[0] + ":" + str(self.__addr[1])

    def read(self):
        """ read data from socket """
        return self.__accepted_sock.recv(1024).decode("utf-8")

    def write(self, msg):
        """ read data from socket """
        return self.__accepted_sock.send(msg.encode("utf-8"))

    def close(self):
        """ close socket """
        self.__accepted_sock.close()


class CommandResultState(Enum):
    """ CommandResultState """
    NOT_FOUND = 1
    EXECUTED_SUCCESS = 2
    EXECUTED_FAILED = 3
    USER_EXIT = 3


class ClientProtocolCommandHelp:
    """ ClientProtocolCommandHelp """

    def __init__(self, _sock):
        self.__sock = _sock
        self.__command_name = "HELP"
        self.__command_args = []
        self.__command_description = "This list of commands"
        self.__help_text = [
            "Example Service CTF01D Automation Training v1.0.0",
        ]

    def command_args(self):
        """ command_args """
        return self.__command_args

    def command_name(self):
        """ command_name """
        return self.__command_name

    def command_description(self):
        """ command_name """
        return self.__command_description

    def init_help(self, commands):
        """ init_help """
        self.__help_text.extend([
            "You connected from: " + self.__sock.get_address_info() + "",
            "Commands:",
        ])
        for cmd in commands:
            self.__help_text.append(
                "    " + cmd.command_name() + "            " + cmd.command_description()
            )
        self.__help_text = "\n".join(self.__help_text) + "\n"

    def help_text(self):
        """ help_text """
        return self.__help_text

    def execute(self, _):
        """ execute command """
        self.__sock.write(self.__help_text)
        return CommandResultState.EXECUTED_SUCCESS

class ClientProtocolCommandList:
    """ ClientProtocolCommandList """
    def __init__(self, _sock):
        self.__sock = _sock
        self.__command_name = "LIST"
        self.__command_args = []
        self.__command_description = "List of items"

    def command_name(self):
        """ command_name """
        return self.__command_name

    def command_args(self):
        """ command_args """
        return self.__command_args

    def command_description(self):
        """ command_name """
        return self.__command_description

    def execute(self, _):
        """ execute """
        _items_counter = 0
        if os.path.isdir('flags'):
            for filename in os.listdir('flags/'):
                self.__sock.write("    " + filename + "\n")
                _items_counter += 1
        self.__sock.write("  FOUND " + str(_items_counter) + " ITEM(S)\n")
        return CommandResultState.EXECUTED_SUCCESS


class ClientProtocolCommandPut:
    """ ClientProtocolCommandPut """
    def __init__(self, _sock):
        self.__sock = _sock
        self.__command_name = "PUT"
        # PUT <id> <data>   Put item to storage (data: [A-Za-z0-9_\\-]{1,})
        self.__command_args = ["id", "data"]
        self.__command_description = "Put item to storage (data: [A-Za-z0-9_\\-]{1,})e"

    def command_name(self):
        """ command_name """
        return self.__command_name

    def command_args(self):
        """ command_args """
        return self.__command_args

    def command_description(self):
        """ command_name """
        return self.__command_description

    def execute(self, _args):
        """ execute """
        if len(_args) > 3:
            self.__sock.write("FAIL expected only one arg like a 'PUT <id> <flag_val>'\n")
            return CommandResultState.EXECUTED_FAILED
        _flag_id = ""
        if len(_args) > 1:
            _flag_id = _args[1].strip()
        # TODO validate _flag_id
        if _flag_id == "":
            self.__sock.write("FAIL incorrect id\n")
            return CommandResultState.EXECUTED_FAILED
        _flag_data = ""
        if len(_args) > 2:
            _flag_data = _args[2].strip()
        # if not os.path.exists('flags/' + _flag_id):
        #     self.__sock.write("FAIL id not found\n")
        #     return CommandResultState.EXECUTED_FAILED
        if os.path.isdir('flags/' + _flag_id):
            self.__sock.write("FAIL id not found (dir?)\n")
            return CommandResultState.EXECUTED_FAILED
        with open('flags/' + _flag_id, "wb") as _file:
            _file.write(_flag_data.encode("utf-8"))
            self.__sock.write("OK\n")
            # self.__sock.write("DATA " + _file.readline().strip() + "\n")
        return CommandResultState.EXECUTED_SUCCESS


class ClientProtocolCommandGet:
    """ ClientProtocolCommandGet """
    def __init__(self, _sock):
        self.__sock = _sock
        self.__command_name = "GET"
        # GET <id>          Get item by id from storage
        self.__command_args = ["id"]
        self.__command_description = "Get item by id from storage"

    def command_args(self):
        """ command_args """
        return self.__command_args

    def command_name(self):
        """ command_name """
        return self.__command_name

    def command_description(self):
        """ command_name """
        return self.__command_description

    def execute(self, _args):
        """ execute """
        if len(_args) > 2:
            self.__sock.write("FAIL expected only one arg like a 'GET some'\n")
            return CommandResultState.EXECUTED_FAILED
        _flag_id = ""
        if len(_args) > 1:
            _flag_id = _args[1].strip()
        if _flag_id == "":
            self.__sock.write("FAIL incorrect id\n")
            return CommandResultState.EXECUTED_FAILED
        # TODO validate _flag_id
        if not os.path.exists('flags/' + _flag_id):
            self.__sock.write("FAIL id not found\n")
            return CommandResultState.EXECUTED_FAILED
        if os.path.isdir('flags/' + _flag_id):
            self.__sock.write("FAIL id not found (dir?)\n")
            return CommandResultState.EXECUTED_FAILED
        with open('flags/' + _flag_id, "rt", encoding="utf-8") as _file:
            self.__sock.write("DATA " + _file.readline().strip() + "\n")
        return CommandResultState.EXECUTED_SUCCESS


class ClientProtocolCommandDel:
    """ ClientProtocolCommandDel """
    def __init__(self, _sock):
        self.__sock = _sock
        self.__command_name = "DEL"
        self.__command_args = ["id"]
        # DEL <id>          Get item by id from storage
        self.__command_description = "hidden"

    def command_args(self):
        """ command_args """
        return self.__command_args

    def command_name(self):
        """ command_name """
        return self.__command_name

    def command_description(self):
        """ command_name """
        return self.__command_description

    def execute(self, _args):
        """ execute """
        if len(_args) > 2:
            self.__sock.write("FAIL expected only one arg like a 'DEL some'\n")
            return CommandResultState.EXECUTED_FAILED
        _flag_id = ""
        if len(_args) > 1:
            _flag_id = _args[1].strip()
        if _flag_id == "":
            self.__sock.write("FAIL incorrect id\n")
            return CommandResultState.EXECUTED_FAILED
        _filepath = 'flags/' + _flag_id
        # TODO validate _flag_id
        if not os.path.exists(_filepath):
            self.__sock.write("FAIL id not found\n")
            return CommandResultState.EXECUTED_FAILED
        if os.path.isdir(_filepath):
            self.__sock.write("FAIL id not found (dir?)\n")
            return CommandResultState.EXECUTED_FAILED
        os.remove(_filepath)
        self.__sock.write("OK\n")
        return CommandResultState.EXECUTED_SUCCESS


class ClientProtocolCommandExit:
    """ ClientProtocolCommandExit """
    def __init__(self, _sock):
        self.__sock = _sock
        self.__command_name = "EXIT"
        # GET <id>          Get item by id from storage
        self.__command_args = []
        self.__command_description = "Exit"

    def command_args(self):
        """ command_args """
        return self.__command_args

    def command_name(self):
        """ command_name """
        return self.__command_name

    def command_description(self):
        """ command_name """
        return self.__command_description

    def execute(self, _):
        """ execute """
        self.__sock.write("BYE-BYE\n")
        return CommandResultState.USER_EXIT


class ClientProtocol(threading.Thread):
    """ Connect """

    def __init__(self, _sock: ClientSocket):
        self.__sock = _sock
        print("Connected ", self.__sock.get_address_info())
        self.__cmd_help = ClientProtocolCommandHelp(self.__sock)
        self.__cmd_del = ClientProtocolCommandDel(self.__sock)
        self.__commands = [
            self.__cmd_help,
            ClientProtocolCommandList(self.__sock),
            ClientProtocolCommandGet(self.__sock),
            ClientProtocolCommandPut(self.__sock),
            ClientProtocolCommandExit(self.__sock),
        ]
        self.__cmd_help.init_help(self.__commands)

        self.__kill = False
        threading.Thread.__init__(self)

    def run (self):
        self.__sock.write(self.__cmd_help.help_text())
        while True:
            if self.__kill:
                break
            self.__sock.write("> ")
            buf = self.__sock.read()
            if buf == "":
                break
            buf = buf.strip()
            cmd = buf.upper()
            if " " in cmd:
                cmd = cmd.split(" ")[0]
            _args =  buf.split(" ")
            _state = CommandResultState.NOT_FOUND
            for _cmd_h in self.__commands:
                if _cmd_h.command_name() == cmd:
                    _state = _cmd_h.execute(_args)
                    break
            if cmd == self.__cmd_del.command_name():
                _state = self.__cmd_del.execute(_args)
            if _state == CommandResultState.USER_EXIT:
                self.__kill = True
                break
            if _state == CommandResultState.EXECUTED_SUCCESS:
                # self.__sock.write("> \n")
                continue
            if _state == CommandResultState.EXECUTED_FAILED:
                # self.__sock.write("> \n")
                continue

            self.__sock.write("FAIL ["+ cmd + "] Unknown command, look 'help'\n")
            continue

        self.__kill = True
        self.__sock.close()
        THREADS_WITH_CLIENT.remove(self)

    def kill(self):
        if self.__kill:
            return
        self.__kill = True
        self.__sock.close()
        # THREADS_WITH_CLIENT.remove(self)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind((SERVICE_LISTEN_HOST, SERVICE_LISTEN_PORT))
s.listen(10)
print('Started server on ' + str(SERVICE_LISTEN_PORT) + ' port.')

if not os.path.exists("flags"):
    os.makedirs("flags")
try:
    while True:
        sock, addr = s.accept()
        thr = ClientProtocol(ClientSocket(sock, addr))
        THREADS_WITH_CLIENT.append(thr)
        thr.start()
except KeyboardInterrupt:
    print('Bye! Write me letters!')
    s.close()
    for thr in THREADS_WITH_CLIENT:
        thr.kill()

#!/usr/bin/env python
# coding: utf-8

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

import string
import sys
import socket
import time
import re

"""
Tester for RUM2 RTSP Module.
"""

################ AUX functions and tests ##################

def usage():

    """
    Print usage on stdout.
    """

    print "\nUsage:\n"
    print "   ", sys.argv[0], "HOST PORT TEST_NUM REPEATS_NUM\n"
    print """
    HOST        - RTSP server IP address (IPv4)
    PORT        - RTSP server port <1, 65535>
    TEST_NUM    - test ID <1, 5>
    REPEATS_NUM - number of iterations <1, 1000>
    """

def valid_address(addr):

    """
    Check IP address validity (regexp).
    """

    if addr == "localhost": return True

    pattern = r"\b(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\b"

    if re.match(pattern, addr):
        return True
    else:
        return False

def parse_session_id(response):

    """
    Parse session ID from SETUP response msg.
    """

    lines = response.split("\r\n");
    for i in lines:
        this_line = i.split(" ");
        if (this_line[0] == "Session:") and (len(this_line) >= 2):
            session_line = this_line[1].split(";")
            if len(session_line) >=1 : return session_line[0]
            else: return None

    return None

def run_test_ok():

    """
    Run test TYPE1 - send MSG_NUM messages, expect reply and active connection.
    """

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    cseq = 1
    session = None

    try:
        s.connect((host,port))
    except socket.error, (value,message):
        if s:
            s.close()
        print "Could not open socket: " + message
        sys.exit(-1)

    for y in range(msg_num):
        s.send(msg[y].format(cseq,session))
        cseq = cseq + 1
        data = s.recv(buffSize)

        if y == 0: print ""
        print "Sending:", msg_sem[y]
        print '\n', data[:len(data)-3]

        if not session:
            session = parse_session_id(data)
            if session:
                print "\n>>> Parsed session ID:", session

        print "*"*80
        if y == msg_num - 1: print ""

    s.close()

def run_test_err():

    """
    Run test TYPE2 - send MSG_NUM messages, expect error msgs and terminated connection.
    """

    for y in range(msg_num):

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            s.connect((host,port))
        except socket.error, (value,message):
            if s:
                s.close()
                print "Could not open socket: " + message
                sys.exit(-1)
                    
        s.send(msg[y])
        data = s.recv(buffSize)

        if y == 0: print ""
        print "Sending:", msg_sem[y]
        print '\n', data[:len(data)-3]

        print "*"*80
        if y == msg_num - 1: print ""

def run_test_timeout():

    """
    Run test TYPE3 - send msg and wait for timeout (RUM2 RTSP default is 60s).
    """

    socket_list = []

    for y in range(msg_num-1):

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        try:
            s.connect((host,port))
        except socket.error, (value,message):
            if s:
                s.close()
                print "Could not open socket: " + message
                sys.exit(-1)

        socket_list.append(s)

    for x in range(msg_num-1):

        cseq = 1
        session = None

        for y in range (x+1):
            socket_list[x].send(msg[y].format(cseq,session))
            cseq = cseq + 1
            data = socket_list[x].recv(buffSize)

            if y == 0: print ""
            print "Sending:", msg_sem[y]
            print '\n', data[:len(data)-3]

            if not session:
                session = parse_session_id(data)
                if session:
                    print ">>> Parsed session ID:", session, "\n"
                else:
                    print "\n"

    print "*"*80
    time.sleep(62)

################ Check arguments ##################

if len(sys.argv) < 5:
    usage()
    sys.exit(-1);

host = sys.argv[1]
port = int(sys.argv[2])
test = int(sys.argv[3])
repeats = int(sys.argv[4])
buffSize = 600

if (port < 1) or (port > 65535):
    print "\nArgument(s) out of bounds! (check PORT)"
    usage()
    sys.exit(-2)
elif (test < 1) or (test > 5) or (repeats < 1) or (repeats > 1000):
    print "\nArgument(s) out of bounds! (check TEST_NUM and REPEATS_NUM)"
    usage()
    sys.exit(-2)
elif not valid_address(host):
    print "\nIP address is not valid!"
    usage()
    sys.exit(-3)

################ Select MSG template ##################

if (test == 1) or (test == 3):
    msg = ["OPTIONS rtsp://localhost:6666/ RTSP/1.0\r\nCSeq: {0}\r\nUser-Agent: VLC media player (LIVE555 Streaming Media v2008.07.24)\r\n\r\n",
"DESCRIBE rtsp://localhost:6666/ RTSP/1.0\r\nCSeq: {0}\r\nAccept: application/sdp\r\nUser-Agent: VLC media player (LIVE555 Streaming Media v2008.07.24)\r\n\r\n",
"SETUP rtsp://localhost:6666/ RTSP/1.0\r\nCSeq: {0}\r\nTransport: RAW/RAW/UDP;unicast;client_port=1234-1235\r\nUser-Agent: VLC media player (LIVE555 Streaming Media v2008.07.24)\r\n\r\n",
"PLAY rtsp://localhost:6666/ RTSP/1.0\r\nCSeq: {0}\r\nSession: {1}\r\nRange: npt=0,000-\r\nUser-Agent: VLC media player (LIVE555 Streaming Media v2008.07.24)\r\n\r\n",
"TEARDOWN rtsp://localhost:6666/ RTSP/1.0\r\nCSeq: {0}\r\nSession: {1}\r\nUser-Agent: VLC media player (LIVE555 Streaming Media v2008.07.24)\r\n\r\n"]
    msg_sem = ["OPTIONS - OK", "DESCRIBE - OK", "SETUP - OK", "PLAY - OK", "TEARDOWN - OK"]
elif test == 2:
    msg = ["OPTIONS rtsp://localhost:6666/ RTSP/1.0\r\nCSeq: 0\r\nUser-Agent: VLC media player (LIVE555 Streaming Media v2008.07.24)\r\n\r\n",
"DESCRIBE RTSP/1.0\r\nCSeq: 1\r\nAccept: application/sdp\r\nUser-Agent: VLC media player (LIVE555 Streaming Media v2008.07.24)\r\n\r\n",
"SETUP rtsp://localhost:6666/ RTSP/1.1\r\nCSeq: 1\r\nTransport: RAW/RAW/UDP;unicast;client_port=1234-1235\r\nUser-Agent: VLC media player (LIVE555 Streaming Media v2008.07.24)\r\n\r\n",
"PLAY rtsp://localhost:6666/ RTSP/1.0\r\nCSeq: 1\r\nRange: npt=0,000-\r\nUser-Agent: VLC media player (LIVE555 Streaming Media v2008.07.24)\r\n\r\n",
"TEARDOWN rtsp://localhost:6666/ RTSP/1.0\r\nCSeq: \n\r\r\nSession: 1\r\nUser-Agent: VLC media player (LIVE555 Streaming Media v2008.07.24)\r\n\r\n",
"OPTIONS rtsp://localhost:6666/ RTSP/1.0\r\nCSeq: 12354542656234265636234\r\nUser-Agent: VLC media player (LIVE555 Streaming Media v2008.07.24)\r\n\r\n",
"NULL NULL NULL", "1231231513515", "\n\e\r\t    \t                                        ", "\0\1\5\656\7\15\77\0\7",
"SETUP rtsp://localhost:6666/ RTSP/1.0\r\nCSeq: 1\r\nTransport: RAW/RAW/UDP;unicast;client_port=1234-1235User-Agent: VLC media player (LIVE555 Streaming Media v2008.07.24)"]
    msg_sem = ["OPTIONS - wrong CSeq (0)", "DESCRIBE - missing URL", "SETUP - wrong RTSP ver.", "PLAY - missing SessionID", "TEARDOWN - missing CSeq",
    "OPTIONS - extra large CSeq", "3x NULL", "random numbers", "control sequences + white spaces", "some ASCII chars from <1 - 31>", "msg without terminating chars"]
elif test == 4:
    pass
elif test == 5:
    pass
else:
    sys.exit(0)

msg_num = len(msg_sem)

if len(msg) != msg_num:
    print "\nList of msgs and list of msg descriptions have different number of items!"
    sys.exit(-4)

################ Run tests ##################

# there is no time to repeat timeouts
if test == 3 : repeats = 1

for x in range(repeats):

    print "\n", "#"*27, "Test {0} - {1}. iteration".format(repr(test).rjust(1), repr(x + 1).rjust(4)), "#"*27

    if test == 1:
        run_test_ok()
    elif test == 2:
        run_test_err()
    elif test == 3:
        run_test_timeout()

    time.sleep(0.5)


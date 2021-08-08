#!/usr/bin/env python

import time
import socket

host = "127.0.0.1"
port = 12000

timeout = 1
min_ping = 999999
max_ping = 0
ping_count = 0
ping_received = 0
avg_ping = 0
packt_loss = 0

clientSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
clientSocket.settimeout(timeout)

def show_summary():

    print('--- %s udp ping statistics ---' % (host))
    print('%d packets transmitted, %d received, %0.0f%% packet loss' % (ping_count, ping_received, packt_loss))
    print('RTT min ' + str(min_ping) + ' RTT max ' + str(max_ping) + ' RTT avg ' + str(avg_ping / ping_count))
    clientSocket.close()

for seq in range(1,11):
    try:
        start = time.time()
        message = 'Ping #' + str(seq) + " " + time.ctime(start)
        clientSocket.sendto(message.encode(), (host, port))
        ping_count += 1
        print("Sent " + message)
        data, server = clientSocket.recvfrom(1024)
        ping_received += 1
        print("Received " + str(data))
        end = time.time()
        time_ping = (end - start)
        print("RTT: " + str(time_ping) + " seconds\n")
        if time_ping < min_ping:
            min_ping = time_ping
        if time_ping > max_ping:
            max_ping = time_ping
        avg_ping += time_ping
    except socket.timeout as e:
        print('udp_seq=%d Request timed out\n' % (seq))
        packt_loss += 10
    except KeyboardInterrupt:
        show_summary()

show_summary()

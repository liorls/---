import sys
import time
from socket import *
import random

_host = "127.0.0.1"
_port = 12000
# Create a UDP socket
client_socket = socket(AF_INET, SOCK_DGRAM)

for i in range(30):
    rand = random.randint(0, 25)
    start_time = time.time()
    message = 'Heartbeat ' + str(i+1) + ' ' + str(start_time)

    # packet loss
    if rand in range(0,10):
        time.sleep(4)
        client_socket.sendto(message, (_host, _port))

    # not response from server
    elif rand in range(22,25):
        time.sleep(15)
        client_socket.sendto(message, (_host, _port))
        print("not response for 15 seconds - Server should assumed that client has shut down")
        sys.exit()

    # successfully
    else:
        client_socket.sendto(message, (_host, _port))

    message, addr = client_socket.recvfrom(12000)
    print("Message from server: " + message)
    
client_socket.close()
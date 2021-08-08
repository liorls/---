import time
import sys
from socket import *

_host = "127.0.0.1"
_port = 12000
# Create a UDP socket
serverSocket = socket(AF_INET, SOCK_DGRAM)

# Assign IP address and port number to socket
serverSocket.bind((_host, _port))

counter = 1

while True:
    # Receive the client packet
    message, addr = serverSocket.recvfrom(1024)
    messageFromClient = str(message).split(' ')
    # messageFromClient = message = Heartbeat 1 1628185475.92
    start_time = messageFromClient[2]
    total_time = time.time() - float(start_time)

    # if time is more than 5 seconds - client application stopped
    if total_time > 5:
        sys.exit('Client application has stopped.')

    # if time is 5 seconds - lost
    elif total_time > 4:
        message = 'Heartbeat ' + str(counter) + ' is missing'
        print(message)
        serverSocket.sendto(message, addr)

    else:     # if time is between 1 to 4 seconds - received
        message = 'Heartbeat ' + str(counter) + ' is received'
        print(message)
        serverSocket.sendto(message, addr)

    counter += 1
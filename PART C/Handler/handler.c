#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/socket.h>

#include "handler.h"
#include "../Parser/cli_parser.h"
#include "../Protocol/message.h"
#include "../Reactor/select.h"
#include "../Configuration/configuration.h"

bool handle(int32_t fd, Node *node) // Handle ready to read FD
{
    char buffer[sizeof(message)];
    int32_t new_sock = 0;
    if (fd == STDIN_FILENO) // If it is STDIN fgets the input
    {
        fgets(buffer, 200, stdin);
        parse_check_run(node, buffer);
    }
    else if (fd == LISTENING_FD) //listening socket is ready that means new connection to accept
    {
        new_sock = accept(fd, NULL, NULL);
        if (new_sock < 0)
        {
            perror("  accept() failed");
            goto Exit;
        }
        add_fd_to_monitoring(new_sock); //Add the new socket to the reactor
        if (!NODE_add_neighbor(node, -1, new_sock))
            return false;
    }
    else
    {
        size_t len = recv(fd, buffer, sizeof(message), 0); //If other fd is ready to read that means a message was sent to my node
        // printf(" recieved: %ld bytes\n", len);
        message_parse(node, buffer, len, fd);
    }

Exit:
    return false;
}
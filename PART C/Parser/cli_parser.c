#include "cli_parser.h"
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

/**
 * Parse the CLI input for the function and the parameters
 * Call the corresponding function
 */
int32_t parse_check_run(Node *node, char *input)
{
    if (node == NULL || input == NULL)
    {
        perror("NULL ARGS IN parse_check_run");
        goto Exit;
    }
    bool success = true;
    char *chunk = strtok(input, COMMA); //get the function
    if (strncmp(input, "peers", 5) == 0)
    {
        success = check_peers(node);
        if (!success)
        {
            return -1;
        }
        return 1;
    }

    if (chunk == NULL)
    {
        goto Exit;
    }
    else if (strcmp(chunk, "setid") == 0)
    {
        success = check_setid(node, chunk);
        if (!success)
        {
            return -1;
        }
    }
    else if (strcmp(chunk, "connect") == 0)
    {
        success = check_connect(node, chunk);
        if (!success)
        {
            return -1;
        }
    }
    else if (strcmp(chunk, "send") == 0)
    {
        success = check_send(node, input);
        if (!success)
        {
            return -1;
        }
    }
    else if (strcmp(chunk, "route") == 0)
    {
        success = check_route(node, input);
        if (!success)
        {
            return -1;
        }
    }
    else
    {
        goto Exit;
    }
    return 1;
Exit:
    printf("BAD INPUT\n");
    return -1;
}

bool check_setid(Node *node, char *string)
{
    if (node == NULL || string == NULL)
    {
        perror("NULL ARGS IN setid");
        goto Exit;
    }
    bool success = false;
    char *chr = strtok(NULL, COMMA); //ignore the function
    int32_t id = atoi(chr);
    if (NULL != strtok(NULL, COMMA))
    {
        return false;
    }
    return NODE_setid(node, id);
Exit:
    return success;
}

bool check_connect(Node *node, char *string)
{
    if (node == NULL || string == NULL)
    {
        perror("NULL ARGS IN check_connect");
        goto Exit;
    }
    char *ip = {0};
    uint32_t port = 0;
    bool success = false;

    ip = strtok(NULL, COLON);
    if (ip == NULL)
    {
        return false;
    }
    char *port_str = strtok(NULL, COLON); //parse the input for each argument
    if (port_str == NULL)
        return false;
    port = atoi(port_str);
    success = NODE_connect(node, ip, port);
Exit:
    return success;
}

bool check_send(Node *node, char *string)
{
    if (node == NULL || string == NULL)
    {
        perror("NULL ARGS IN check_send");
        goto Exit;
    }
    bool success = false;
    // strtok(NULL, COMMA); //ignore the function
    int32_t id = atoi(strtok(NULL, COMMA));
    uint32_t len = atoi(strtok(NULL, COMMA));
    char *message = strtok(NULL, COMMA);
    if (NULL != strtok(NULL, COMMA))
    {
        return false;
    }
    success = NODE_send(node, id, len, message);
Exit:
    return success;
}
bool check_route(Node *node, char *string)
{
    if (node == NULL || string == NULL)
    {
        perror("NULL ARGS IN check_route");
        goto Exit;
    }
    bool success = false;
    int32_t id = atoi(strtok(NULL, COMMA)); // get the id from ascii to int
    if (NULL != strtok(NULL, COMMA))
    {
        return false;
    }
    success = NODE_route(node, id);
Exit:
    return success;
}
bool check_peers(Node *node) // print all my current neighbors
{
    if (node == NULL)
    {
        perror("NULL ARGS IN check_route\n");
        return false;
    }
    for (int i = 0; i < node->neighbors_count; i++)
    {
        printf("%d", node->neighbors[i].id);
        if (i < node->neighbors_count - 1)
            printf(",");
    }
    printf("\n");
    return true;
}
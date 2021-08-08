#include "Neighbor.h"
#include <arpa/inet.h>

bool Neighbor_exists(Neighbor *nodes, int32_t size, int32_t id) //search for neighbors ID
{
    if (nodes == NULL)
    {
        perror("NULL args in Neighbor_exists\n");
        goto Exit;
    }
    for (size_t i = 0; i < size; i++)
    {
        if (nodes[i].id == id)
            return true;
    }
Exit:
    return false;
}

int32_t Neighbor_get_index_by_ip_port(Neighbor *neghibors, int32_t len, int32_t fd)
{
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(fd, (struct sockaddr *)&addr, &addr_size); //get the ip & port from a socket
    int32_t port = ntohs(addr.sin_port);

    for (int i = 0; i < len; i++)
    {
        if (neghibors[i].ip_addr == addr.sin_addr.s_addr && neghibors[i].port == ntohs(addr.sin_port))
        {
            return i;
        }
    }
    return -1;
}

short Neighbor_get_sock_by_id(Neighbor *nodes, size_t size, int32_t id)
{
    if (nodes == NULL)
    {
        goto Exit;
    }
    for (size_t i = 0; i < size; i++)
    {
        if (nodes[i].id == id)
            return nodes[i].connection; //return socket connection of Neighbor with id=id
    }
Exit:
    return -1;
}
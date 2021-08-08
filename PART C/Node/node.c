#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include "../Configuration/configuration.h"
#include "../Protocol/message.h"
#include "../Reactor/select.h"
#include "node.h"

bool NODE_init(Node *node, uint32_t port)
{
    if (node == NULL)
    {
        perror("NULL args in NODE_init");
        return false;
    }
    node->neighbors_count = 0; //initiate all the NODE fields
    node->neighbors = NULL;
    node->routing_count = 0;
    node->my_routing = NULL;
    node->connect_sent.amount = 0;
    node->connect_sent.ids = NULL;
    node->id = port;

    int32_t sock = socket(AF_INET, SOCK_STREAM, 0); //create the socket
    if (sock == -1)
    {
        printf("socket creation failed...\n");
        return -1;
    }

    node->sock = sock;
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(NODES_IP);
    servaddr.sin_port = htons(port);

    // Binding newly created socket to given IP and port.
    if ((bind(node->sock, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("socket bind failed. Node ID: %d\n", node->id);
        exit(0);
    }
    // Now server is ready to listen
    if ((listen(node->sock, NODE_MAX_NEIGHBORS)) != 0)
    {
        printf("Listen() failed. Node ID: %d\n", node->id);
        exit(0);
    }
    LISTENING_FD = node->sock;
    current_id = port;
    printf("port: %d is listening..\n", port);
}

bool NODE_setid(Node *node, int32_t id) //set my nodes ID
{
    if (node == NULL)
    {
        perror("NULL args in NODE_setid");
        return false;
    }
    current_id = id;
    node->id = id;
    printf("ACK\n");
    return true;
}

bool NODE_add_neighbor(Node *node, int32_t id, int32_t fd)
{
    node->neighbors_count++;
    node->neighbors = realloc(node->neighbors, node->neighbors_count * sizeof(Neighbor));
    node->neighbors[node->neighbors_count - 1].connection = fd;
    /*check if I can add here ip port set as well*/
    return true;
}

bool NODE_connect(Node *src_node, char *dst_ip, uint32_t dst_port)
{
    if (src_node == NULL || dst_ip == NULL)
    {
        perror("NULL args in NODE_connect");
        return false;
    }
    src_node->neighbors_count++; //Add the new connection as a neighbor and set its socket,port and ip
    src_node->neighbors = realloc(src_node->neighbors, src_node->neighbors_count * sizeof(Neighbor));
    short *src_sock_fd = &src_node->neighbors[src_node->neighbors_count - 1].connection;
    src_node->neighbors[src_node->neighbors_count - 1].ip_addr = inet_addr(dst_ip);
    src_node->neighbors[src_node->neighbors_count - 1].port = dst_port;

    *src_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*src_sock_fd == -1)
    {
        printf("socket creation failed...\n");
        return false;
    }

    struct sockaddr_in dst;                  //the destination I connect to..
    dst.sin_addr.s_addr = inet_addr(dst_ip); //ip address convertion
    dst.sin_port = htons(dst_port);          //set the port
    dst.sin_family = AF_INET;                //socket of type IPv4
    int8_t ret = connect(*src_sock_fd, (struct sockaddr *)&dst, sizeof(dst));
    if (ret == -1)
    {
        perror("Error in connect\n");
        NODE_disconnect_neighbor(src_node, *src_sock_fd); //if the connection failed, close the socket and delete the neighbor..
        return false;
    }

    add_fd_to_monitoring(*src_sock_fd);                                //add the new created socket to the Reactor
    int32_t msg_id = send_connect_message(*src_sock_fd, src_node->id); //send connection message to the destination
    if (msg_id < 0)
    {
        perror("Failed at send_connect_message\n");
        return false;
    }

    src_node->connect_sent.amount++;
    src_node->connect_sent.ids = realloc(src_node->connect_sent.ids,
                                         src_node->connect_sent.amount * sizeof(int32_t));
    src_node->connect_sent.ids[src_node->connect_sent.amount - 1] = msg_id;

    return true;
}

bool NODE_send(Node *node, int32_t id, uint32_t len, char *data) //TODO - use the routing to build the RELAY message
{
    if (node == NULL || data == NULL)
    {
        perror("NULL args in NODE_send");
        return false;
    }
    short dst_sock = Neighbor_get_sock_by_id(node->neighbors, node->neighbors_count, id); //try to get destination socket
    if (dst_sock == -1)                                                                   // routing...
    {
        if (node->neighbors_count > 0)
        {
            if (id == node->id) //sending message in loopback (To myself)
            {
                perror("Can not send to myself\n no socket found!\n");
            }
            printf("No such neighbor, discovering...\n");

            Route send_to;
            // NODE_route(node, id);
            /**
             *  Build Relay message using send_to route and data payload...
             *  Send...
             * 
             * - If len > PAYLOAD_SIZE:
             *  send in chunks using the trailing msg field!
             * 
             */
        }
        else
        {
            perror("No neghibors found!\n"); //If no neighbors to route to
            return false;
        }
    }
    else // Sending to my neighbor
    {
        if (len < PAYLOAD_SIZE)
        { //send only 1 chunk
            bool ret = send_message(dst_sock, node->id, id, len, data, 0);
            if (!ret)
            {
                perror("Failed in send_message");
                return false;
            }
        }
        /**
         * Split to chunks if needed
         */
        else
        {
            uint32_t chunk_num = 0;
            while (len - PAYLOAD_SIZE > 0)
            {
                bool ret = send_message(dst_sock, node->id, id, len, data, 0);
                if (!ret)
                {
                    perror("Failed in send_message");
                    return false;
                }
            }
        }
    }
    return true;
}

bool NODE_route(Node *node, int32_t id)
{
    if (node == NULL)
    {
        perror("NULL args in NODE_route\n");
        goto Exit;
    }
    if (node->neighbors_count < 1)
    {
        perror("Cant route... no neighbors\n");
        goto Exit;
    }
    node->routing_count++; //Create new RoutingInfo
    node->my_routing = realloc(node->my_routing, node->routing_count * sizeof(RoutingInfo));
    RoutingInfo *ri = &node->my_routing[node->routing_count - 1];
    ri->src_node_id = node->id;
    ri->og_id = 0;
    ri->responds_got = 0;
    ri->routes_got = 0;
    ri->routes = NULL;
    ri->discover_ids = NULL;
    ri->discover_ids = (int32_t *)malloc(node->neighbors_count * sizeof(int32_t)); //the discover messages ids

    for (int i = 0; i < node->neighbors_count; i++)
    {
        int64_t ret = send_discover_message(node->neighbors[i].connection, node->id, node->neighbors[i].id, id);
        if (ret < 0)
        {
            perror("Failed in send_discover_message\n");
            return false;
        }
        ri->discover_ids[i] = ret; //save the discover messages ids to count the nacks responses
    }
    return true;
Exit:
    return false;
}

size_t NODE_get_neighbor_index_by_fd(Node *node, short fd)
{
    for (int i = 0; i < node->neighbors_count; i++)
    {
        if (node->neighbors[i].connection == fd)
        {
            return i;
        }
    }
    return -1;
}

bool NODE_disconnect_neighbor(Node *node, short fd)
{
    remove_fd_from_monitoring(fd); //remove from the reactor...
    if (node == NULL)
    {
        perror("Null args in NODE_disconnect_neighbor");
        return false;
    }
    int to_rm = NODE_get_neighbor_index_by_fd(node, fd);
    if (to_rm < 0)
    {
        perror("Failed to find neighbor\n");
        return false;
    }
    if ((--node->neighbors_count) <= 0) //if last neighbor disconnected
    {
        free(node->neighbors);
    }
    else
    { //removing neighbor from Neighbors array
        Neighbor *temp_arr = (Neighbor *)malloc(node->neighbors_count * sizeof(Neighbor));
        for (int j = 0; j < to_rm; j++)
        {
            memcpy(&temp_arr[j], &node->neighbors[j], sizeof(Neighbor));
        }
        for (int k = to_rm + 1; k < node->neighbors_count; k++)
        {
            memcpy(&temp_arr[k], &node->neighbors[k], sizeof(Neighbor));
        }
        printf("closing connection...\n");
        close(node->neighbors[to_rm].connection); //close the socket
        free(node->neighbors);
        if (node->routing_count > 0) //free all the allocated RouteInfo
        {
            if (node->my_routing->routes_got > 0)
            {
                for (int i = 0; i < node->routing_count; i++)
                {
                    free(node->my_routing[i].routes); //free all the allocated routes..
                }
            }
            free(node->my_routing);
        }
        node->neighbors = temp_arr;
    }
    printf("Removed fd %d\n", fd);
    return true;
}
/**
 * find the route og_id and add to it the route
 */

bool NODE_add_route(Node *node, Route *new_route)
{
    if (node == NULL || new_route == NULL)
    {
        perror("NULL args in NODE_add_route\n");
    }
    bool added = false;

    for (int i = 0; i < node->routing_count; i++) // serach for exsiting og_id
    {
        if (node->my_routing[i].og_id == new_route->og_id || node->my_routing[i].og_id == 0) // 0 is if the route started from the cli
        {
            node->my_routing[i].og_id = new_route->og_id;
            node->my_routing[i].routes_got++;
            node->my_routing[i].responds_got++;
            node->my_routing[i].routes = realloc(node->my_routing[i].routes, node->my_routing[i].routes_got * sizeof(Route));
            node->my_routing[i].routes[node->my_routing[i].routes_got - 1].og_id = new_route->og_id;
            node->my_routing[i].routes[node->my_routing[i].routes_got - 1].route_len = new_route->route_len;
            node->my_routing[i].routes[node->my_routing[i].routes_got - 1].nodes_ids = (int32_t *)calloc(sizeof(int32_t), new_route->route_len); //like malloc just with initialization to 0s
            memcpy(node->my_routing[i].routes[node->my_routing[i].routes_got - 1].nodes_ids,
                   new_route->nodes_ids, (sizeof(int32_t) * new_route->route_len));
            added = true;
            break;
        }
    }

    if (!added) /* new routing info*/
    {
        node->routing_count++;
        node->my_routing = realloc(node->my_routing, node->routing_count * sizeof(RoutingInfo));
        node->my_routing[node->routing_count - 1].og_id = new_route->og_id;
        node->my_routing[node->routing_count - 1].responds_got = 1;
        node->my_routing[node->routing_count - 1].routes_got = 1;
        node->my_routing[node->routing_count - 1].routes = malloc(sizeof(Route) * new_route->route_len);
        node->my_routing[node->routing_count - 1].routes->og_id = new_route->og_id;
        node->my_routing[node->routing_count - 1].routes->route_len = new_route->route_len;
        node->my_routing[node->routing_count - 1].routes[0].nodes_ids = (int32_t *)malloc(new_route->route_len * sizeof(int32_t));
        memcpy(node->my_routing[node->routing_count - 1].routes->nodes_ids, new_route->nodes_ids, sizeof(int32_t) * new_route->route_len);
        added = true;
    }

    return true;
}

RoutingInfo *NODE_get_route_info(Node *node, int32_t route_id)
{
    if (node == NULL)
    {
        perror("NULL args in NODE_get_route_info");
        return NULL;
    }
    for (int i = 0; i < node->routing_count; i++)
    {
        if (node->my_routing[i].og_id = route_id || node->my_routing[i].og_id == 0)
        {                                         // og_id = 0 in the case that the route is a cli command!
            node->my_routing[i].og_id = route_id; //change the 0 to the real og_id
            return &node->my_routing[i];          //return RoutingInfo
        }
    }
    return NULL;
}

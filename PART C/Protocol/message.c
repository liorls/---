#include "message.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/socket.h>

static uint32_t id = 0;

/**
 * Check the fields of the packet, currently not used.
 */
bool message_check_format(message *msg)
{
    if (msg->dst_id < 0)
        return false;
    if (msg->src_id < 0)
        return false;
    if (msg->func_id < 0)
        return false;
    if (msg->trailing_msg < 0)
        return false;
    // if (msg->payload < 0)
    //     return false;

    return true;
}
/**
 * pretty print of a message... used for debugging packets
 */
static void debug_print_message(message *msg)
{
    printf("~~~~~ DEBUG ~~~~~\n");
    printf("msg id: %d\n", msg->msg_id);
    printf("src id: %d\n", msg->src_id);
    printf("dst id: %d\n", msg->dst_id);
    printf("func id: %d\n", msg->func_id);
    printf("trailing msg: %d\n", msg->trailing_msg);
    if (msg->func_id == FUNC_ID_ACK || msg->func_id == FUNC_ID_NACK)
    {
        printf("payload: %d\n", (int)msg->payload[0]);
    }
    if (msg->func_id == FUNC_ID_DISCOVER)
    {
        printf("payload: %d\n", ((int32_t *)msg->payload)[0]);
    }
    if (msg->func_id == FUNC_ID_ROUTE)
    {
        SerializedRoute *r = (SerializedRoute *)msg->payload;
        printf("route len: %d\n", r->route_len);
        printf("route og_id: %d\n", r->og_id);
        for (int i = 0; i < r->route_len; i++)
        {
            if (i < r->route_len - 1)
                printf("%d->", r->nodes_ids[i]);
            else
                printf("%d\n", r->nodes_ids[i]);
        }
    }
    else
    {
        printf("payload: %s\n", msg->payload);
    }
    printf("~~~~~~~~~~~~~~~~~\n");
}

int32_t create_connect_message(int32_t src_id, int32_t dst_id, message *msg)
{
    msg->src_id = src_id;
    msg->dst_id = dst_id;
    msg->trailing_msg = 0;
    msg->msg_id = id++;
    msg->func_id = FUNC_ID_CONNECT;
    return msg->msg_id;
}

bool create_ack_message(int32_t src_id, int32_t dst_id, int32_t payload, message *msg)
{
    msg->src_id = src_id;
    msg->dst_id = dst_id;
    msg->trailing_msg = 0;
    msg->msg_id = id++;
    msg->func_id = FUNC_ID_ACK;
    memcpy(msg->payload, &payload, 1);
    return true;
}

bool create_nack_message(int32_t src_id, int32_t dst_id, int32_t payload, message *msg)
{
    msg->src_id = src_id;
    msg->dst_id = dst_id;
    msg->trailing_msg = 0;
    msg->msg_id = id++;
    msg->func_id = FUNC_ID_NACK;
    memset(msg->payload, 0, sizeof(msg->payload));

    memcpy(&msg->payload[0], &payload, sizeof(int32_t));
    return true;
}

uint32_t create_discover_message(int32_t src_id, int32_t dst_id, int32_t target_id, message *msg)
{
    msg->msg_id = id++;
    msg->src_id = src_id;
    msg->dst_id = dst_id;
    msg->trailing_msg = 0; // This is not 0 only in data message
    msg->func_id = FUNC_ID_DISCOVER;
    memset(msg->payload, 0, sizeof(msg->payload));
    memcpy((int32_t *)msg->payload, &target_id, sizeof(int32_t));
    return msg->msg_id;
}

bool create_route_message(int32_t src_id, int32_t dst_id, Route *route, message *msg)
{
    msg->msg_id = id++;
    msg->src_id = src_id;
    msg->dst_id = dst_id;
    msg->trailing_msg = 0;
    msg->func_id = FUNC_ID_ROUTE;
    memset(msg->payload, 0, sizeof(msg->payload));
    SerializedRoute *sr = (SerializedRoute *)msg->payload;
    sr->og_id = route->og_id;
    sr->route_len = route->route_len;
    memcpy(sr->nodes_ids, route->nodes_ids, route->route_len * sizeof(int32_t));
    //copy the allocated ids to the message buffer
    return true;
}

bool create_send_message(int32_t src_id, int32_t dst_id, char *payload, int32_t len, uint32_t chunk_num, message *msg)
{
    msg->msg_id = id++;
    msg->src_id = src_id;
    msg->dst_id = dst_id;
    msg->trailing_msg = chunk_num; //What chunk of data it this? - based on payload len
    msg->func_id = FUNC_ID_SEND;
    memset(msg->payload, 0, sizeof(msg->payload));
    memcpy(msg->payload, payload, len);
    return true;
}

bool send_message(short sock, int32_t src_id, int32_t dst_id, size_t len, char *data, uint32_t chunk_num)
{
    message msg;
    bool ret = create_send_message(src_id, dst_id, data, len, chunk_num, &msg);
    if (!ret)
    {
        perror("Failed creating send message\n");
        return false;
    }
    int32_t sent = send(sock, &msg, sizeof(message), 0);
    if (sent < 0)
    {
        perror("Failed in Send!");
        return false;
    }
    return true;
}

/**
 * Add my ID to the given route.
 */
bool add_myself_to_route(int32_t my_id, Route *route)
{
    route->route_len++;
    route->nodes_ids = (int32_t *)realloc(route->nodes_ids, route->route_len * sizeof(int32_t));
    route->nodes_ids[route->route_len - 1] = my_id; //add my id in the last position
    return true;
}
int32_t send_connect_message(short sock, uint32_t src_node_id)
{
    message msg;
    int32_t msg_id = create_connect_message(src_node_id, 0, &msg);
    int ret = send(sock, &msg, sizeof(message), 0);
    if (ret < 0)
        return -1;
    return msg_id;
}

bool send_ack_message(short sock, int32_t src_node_id, int32_t current_node, int32_t payload)
{
    message msg;
    create_ack_message(src_node_id, current_node, payload, &msg);
    int32_t ret = send(sock, &msg, sizeof(message), 0);
    if (ret < 0)
        return false;
    return true;
}

bool send_nack_message(short sock, int32_t src_node_id, int32_t current_node, int32_t payload)
{
    message msg;
    bool ret;
    ret = create_nack_message(src_node_id, current_node, payload, &msg);
    if (!ret)
    {
        perror("Failed in create_nack_message\n");
        return false;
    }
    int32_t res = send(sock, &msg, sizeof(message), 0);
    if (res < 0)
        return false;
    return true;
}

int64_t send_discover_message(short sock, int32_t src_id, int32_t dst_id, int32_t target_id)
{
    printf("sent discovery to %d with target for %d\n", dst_id, target_id);

    message msg;
    uint32_t id = create_discover_message(src_id, dst_id, target_id, &msg);
    int32_t ret = send(sock, &msg, sizeof(message), 0);
    if (ret < 0)
        return -1;
    return id;
}

bool send_route_message(short sock, int32_t src_node_id, int32_t dst_node_id, Route *route)
{
    message msg;
    printf("send route message to %d\n", dst_node_id);
    create_route_message(src_node_id, dst_node_id, route, &msg);
    int32_t ret = send(sock, &msg, sizeof(message), 0);
    // debug_print_message(&msg);
    if (ret < 0)
        return false;
    return true;
}

static bool parse_ack(Node *node, message *msg, int32_t from_fd)
{
    if (node == NULL || msg == NULL)
    {
        perror("NULL args in parse_ack\n");
        return false;
    }
    printf("ACK\n");
    for (int i = 0; i < node->connect_sent.amount; i++) // check if it is an ack for connect message
    {
        if (node->connect_sent.ids[i] == (msg->payload)[0])
        {
            size_t new_neighbor = NODE_get_neighbor_index_by_fd(node, from_fd);
            node->neighbors[new_neighbor].id = msg->src_id;
            printf("%d\n", msg->src_id);
            node->connect_sent.amount--;
            if (node->connect_sent.amount == 0)
            {
                free(node->connect_sent.ids);
                node->connect_sent.ids = NULL;
            }

            return true;
        }
    }
    /** ADD HERE MORE FUNCTION ACKS HANDLERS **/
    if (node->id == msg->dst_id)
    {
        int32_t n = Neighbor_get_index_by_ip_port(node->neighbors, node->neighbors_count, from_fd);
        if (n == -1)
        {
            perror("didnt get the neghibor!!\n");
            return false;
        }
        else
        {
            node->neighbors[n].id = msg->src_id;
            return true;
        }
    }
    else //Ack not for my ID, what?
    {
        printf("dst id is: %d but my id is: %d\n", msg->dst_id, node->id);
        return false;
    }
    return true;
}

static bool parse_nack(Node *node, message *msg, int32_t fd)
{
    if (node == NULL || msg == NULL)
    {
        perror("NULL args in parse_ack");
    }
    bool ret;
    RoutingInfo *ri;
    int32_t id_for = ((int32_t *)msg->payload)[0]; //the msg_id nack is responding to
    for (int i = 0; i < node->neighbors_count; i++)
    {
        for (int j = 0; j < node->neighbors_count; j++)
        {
            //find the message that the nack is for
            if (node->my_routing[i].discover_ids[j] == id_for)
            {
                node->my_routing[i].responds_got++;
                if (node->my_routing[i].responds_got == node->neighbors_count - 1) //If I got all the responses, send the next message
                {                                                                  //-1 because Im not waiting for a response from the original sender
                    int32_t dst_node_id = node->my_routing[i].src_node_id;
                    short dst_sock = Neighbor_get_sock_by_id(node->neighbors, node->neighbors_count, dst_node_id);
                    //got all responses for this discover
                    if (node->my_routing[i].routes_got == 0)
                    { //send a nack to the routing source node
                        return send_nack_message(dst_sock, node->id, dst_node_id, node->my_routing->og_id);
                    }
                    else
                    { //I got all the discover responses... choose route and send a route message.
                        Route *r;
                        ret = NODE_choose_route(node->my_routing[i].routes, node->my_routing[i].routes_got, r);
                        if (!ret)
                        {
                            perror("Failed in NODE_choose_route\n");
                            return false;
                        }
                        ret = send_route_message(dst_sock, node->id, dst_node_id, r);
                        if (!ret)
                        {
                            perror("Failed in send_route_message\n");
                            return false;
                        }
                    }
                }
                return true;
            }
        }
    }
}
/**
 * Parse the connect message and add to myself a new Neighbor with the id from the msg.src_id
 * Save the Neighbors IP, Port, and connection that was created by the accept function!
 */
static bool parse_connect(Node *node, message *msg, int32_t fd)
{
    if (node == NULL || msg == NULL)
    {
        perror("NULL args in parse_ack");
        return false;
    }
    size_t new_neighbor_index = NODE_get_neighbor_index_by_fd(node, fd);
    if (new_neighbor_index < 0)
        return false;

    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(fd, (struct sockaddr *)&addr, &addr_size); // get the ip and port from a socket
    int32_t port = ntohs(addr.sin_port);

    node->neighbors[new_neighbor_index].port = port; //Add neighbor fields
    node->neighbors[new_neighbor_index].id = msg->src_id;
    node->neighbors[new_neighbor_index].ip_addr = addr.sin_addr.s_addr;

    send_ack_message(node->neighbors[new_neighbor_index].connection, node->id, msg->src_id, msg->msg_id);
    printf("sent connect ack to (node id) %d\n", node->neighbors[new_neighbor_index].id);

    return true;
}

static bool parse_discover(Node *node, message *msg, short from_fd)
{
    if (node == NULL || msg == NULL)
    {
        perror("NULL args in parse_discover\n");
        return false;
    }
    int32_t target_id = ((int32_t *)(msg->payload))[0]; // get first int32 from the payload
    Neighbor *got_from = &node->neighbors[NODE_get_neighbor_index_by_fd(node, from_fd)];
    if (node->id == target_id) // IF IM the discover destination
    {
        // printf("IM THE TATGET!!\n");
        Route route;
        route.og_id = msg->msg_id;
        route.route_len = 1;
        route.nodes_ids = (int32_t *)malloc(route.route_len * sizeof(int32_t));
        route.nodes_ids[0] = node->id;
        int32_t dst_id = got_from->id;
        printf("sending to sock %d\n", got_from->connection);
        bool ret = send_route_message(got_from->connection, node->id, dst_id, &route);
        if (!ret)
        {
            perror("Failed in send_route_message\n");
            return false;
        }
        free(route.nodes_ids);
        return true;
    }
    if (node->neighbors_count == 1) //no more neighbors to search
    {
        return send_nack_message(got_from->connection, node->id, got_from->id, msg->msg_id);
    }
    else //I am not the target so now im searching for it...
    {
        node->routing_count++;
        node->my_routing = realloc(node->my_routing, node->routing_count * sizeof(RoutingInfo));
        RoutingInfo *ri = &node->my_routing[node->routing_count - 1];
        ri->src_node_id = msg->src_id;
        ri->og_id = msg->msg_id;
        ri->responds_got = 0;
        ri->routes_got = 0;
        ri->routes = NULL;
        ri->discover_ids = NULL;
        ri->discover_ids = (int32_t *)malloc(node->neighbors_count * sizeof(int32_t));

        for (int i = 0; i < node->neighbors_count; i++)
        {
            if (node->neighbors[i].id != msg->src_id)
            {
                int64_t ret = send_discover_message(node->neighbors[i].connection, node->id, node->neighbors[i].id, target_id);
                if (ret < 0)
                {
                    perror("Failed in send_discover_message\n");
                    return false;
                }
                ri->discover_ids[i] = ret;
            }
        }
    }

    return true;
}

void print_route(Route *route)
{
    for (int i = 0; i < route->route_len; i++)
    {
        if (i < route->route_len - 1)
        {
            printf("%d->", route->nodes_ids[i]);
        }
        else
        {
            printf("%d\n", route->nodes_ids[i]);
        }
    }
}

static bool parse_route(Node *node, message *msg, short from_fd, Route *best_route)
{
    if (node == NULL || msg == NULL)
    {
        perror("NULL args in parse_route");
    }
    SerializedRoute *serialized_route = (SerializedRoute *)msg->payload;
    Route route;
    bool ret = ROUTE_deserialize(serialized_route, &route); // convert msg payload to Route struct
    bool res = NODE_add_route(node, &route);
    if (!res)
    {
        perror("Failed in adding route..\n");
        return false;
    }
    RoutingInfo *ri = NODE_get_route_info(node, route.og_id);
    if (ri == NULL)
    {
        perror("Failed in NODE_get_route_info\n");
        return false;
    }
    if (ri->src_node_id == node->id) //If I started to routing
    {
        if (ri->responds_got == node->neighbors_count)
        {
            //got all the route and nacks back
            //choose the best route
            ret = NODE_choose_route(ri->routes, ri->routes_got, best_route);
            if (!ret)
            {
                perror("Failed in NODE_choose_route\n");
                return false;
            }
            print_route(best_route);
            return best_route;
        }
    }

    else //routing through me...
    {
        bool ret = add_myself_to_route(node->id, &route);
        if (!ret)
        {
            perror("failed in add_myself_to_route\n");
            return false;
        }
        short dst_sock = Neighbor_get_sock_by_id(node->neighbors, node->neighbors_count, ri->src_node_id);
        if (dst_sock < 0)
        {
            perror("No such neighbor\n");
            return false;
        }
        ret = send_route_message(dst_sock, node->id, ri->src_node_id, &route);
        if (!ret)
        {
            perror("failed in send_route_message\n");
            return false;
        }
        // printf("debug 2.2\n");
    }
    return true;
}

static bool parse_send(Node *node, message *msg)
{
    if (node == NULL || msg == NULL)
    {
        perror("NULL args in parse_send");
    }
    if (msg->dst_id == node->id)
    {
        printf("GOT: %s\n", msg->payload);
        return true;
    }
}

bool message_parse(Node *node, char *buffer, size_t len, int32_t from_fd)
{
    if (len == 0)
    {
        bool ret = NODE_disconnect_neighbor(node, from_fd);
    }
    if (node == NULL || buffer == NULL)
    {
        perror("Null args in message_parse\n");
    }
    message *msg = (message *)buffer;
    // debug_print_message(msg);
    switch (msg->func_id)
    {
    case FUNC_ID_ACK:
        printf("%d Got an ack message\n", node->id);
        bool success = parse_ack(node, msg, from_fd);
        if (!success)
        {
            perror("Failed parsing ack");
            return false;
        }
        break;
    case FUNC_ID_NACK:
        printf("Got an nack message\n");
        success = parse_nack(node, msg, from_fd);
        if (!success)
        {
            perror("Failed parsing nack");
            return false;
        }
        break;
    case FUNC_ID_CONNECT:
        printf("%d Got a connect message\n", node->id);
        success = parse_connect(node, msg, from_fd);
        if (!success)
        {
            perror("Failed parsing connect");
            return false;
        }
        break;
    case FUNC_ID_DISCOVER:
        printf("Got an discover message\n");
        success = parse_discover(node, msg, from_fd);
        if (!success)
        {
            perror("Failed parsing discover");
            return false;
        }
        break;
    case FUNC_ID_ROUTE:
        printf("Got an route message\n");
        Route best;
        success = parse_route(node, msg, from_fd, &best);
        if (!success)
        {
            perror("Failed parsing route");
            return false;
        }
        break;
    case FUNC_ID_SEND:
        printf("Got an SEND message\n");
        success = parse_send(node, msg);
        if (!success)
        {
            perror("Failed parsing send\n");
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}
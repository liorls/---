#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>
#include "../Route/Route.h"
#include "../Neighbor/Neighbor.h"

typedef struct routing_now
{
    int32_t og_id; //original discover message id
    size_t responds_got;
    size_t routes_got;
    Route *routes; //different routes got for the each og_id
    int32_t src_node_id;
    int32_t *discover_ids; //discover messages Ids
} RoutingInfo;

typedef struct connect_info
{
    size_t amount;
    int32_t *ids;
} ConnectSent; //Keep track for the IDs of the connect messages IDs I sent

typedef struct node
{
    int32_t id;
    short sock; // The socket I accept connection on
    Neighbor *neighbors;
    uint8_t neighbors_count;  //how many neighbors I got
    int32_t routing_count;    // how many routings am I running
    RoutingInfo *my_routing;  //my current routings
    ConnectSent connect_sent; //Information about the connect packets I sent
} Node;

bool NODE_init(Node *node, uint32_t port);
bool NODE_setid(Node *node, int32_t id);
bool NODE_connect(Node *node, char *ip, uint32_t port);
bool NODE_send(Node *node, int32_t id, uint32_t len, char *message);
bool NODE_route(Node *node, int32_t id);
Node *NODE_get_by_id(Node *nodes, int32_t id);
bool NODE_add_neighbor(Node *node, int32_t id, int32_t fd);
bool NODE_disconnect_neighbor(Node *node, short fd);
size_t NODE_get_neighbor_index_by_fd(Node *node, short fd);
RoutingInfo *NODE_get_route_info(Node *node, int32_t route_id);

bool NODE_add_route(Node *node, Route *route);
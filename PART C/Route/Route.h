#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct route //dynamic allocation
{
    int32_t og_id; //for easy convertion of payload to route
    int32_t route_len;
    int32_t *nodes_ids; //the route itself
} Route;

typedef struct serialized_route //static allocation
{
    int32_t og_id; //for easy convertion of payload to route
    int32_t route_len;
    int32_t nodes_ids[]; //the route itself
} SerializedRoute;

bool NODE_choose_route(Route *routes, size_t len, Route *best);
bool ROUTE_deserialize(SerializedRoute *serialized_route, Route *route);
#include "Route.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool NODE_choose_route(Route *routes, size_t len, Route *best)
{
    if (routes == NULL)
    {
        perror("NULL args in NODE_choose_route\n");
        return false;
    }
    int32_t min_len = INT32_MAX;
    int32_t index = INT32_MAX;
    for (int i = 0; i < len; i++)
    {
        if (routes[i].route_len < min_len)
        {
            min_len = routes->route_len;
            index = i;
        }
        else if (routes[i].route_len == min_len)
        {
            //take the smaller route in lexicographical order
            for (int32_t j = 0; j < min_len; j++)
            {
                if (routes[i].nodes_ids[j] < routes[index].nodes_ids[j])
                {
                    //compare the values between current route and temporary best route
                    index = i;
                    break;
                }
            }
        }
    }
    best->og_id = routes[index].og_id;
    best->route_len = routes[index].route_len;
    best->nodes_ids = (int32_t *)malloc(sizeof(int32_t) * best->route_len);
    memcpy(best->nodes_ids, routes[index].nodes_ids, best->route_len * sizeof(int32_t));
    return true;
}

bool ROUTE_deserialize(SerializedRoute *serialized_route, Route *route)
{
    route->og_id = serialized_route->og_id;
    route->route_len = serialized_route->route_len;
    route->nodes_ids = (int32_t *)malloc(route->route_len * sizeof(int32_t));
    memcpy(route->nodes_ids, serialized_route->nodes_ids, route->route_len * sizeof(int32_t));
    return true;
}

//
//  commands.h
//  
//
//  Created by Madhav on 11/2/16.
//
//

#ifndef commands_h
#define commands_h

typedef struct {
    int16_t id;
    int32_t IP;
    int16_t port;
    int16_t cost;
    int isNeighbour;
    int16_t updatecount;
    int isDisabled;
} NODE;


typedef struct {
    int16_t id;
    int16_t cost;
    int16_t nexthop;
}routing_table;


typedef struct{
    char ip[32];
    int port;
}ip_port;

#endif /* commands_h */

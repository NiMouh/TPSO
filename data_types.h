#ifndef Custom_Data_Types_H
#define Custom_Data_Types_H

typedef struct timeval Time;

typedef enum
{
    FIFO, // First In First Out
    HPSC, // High Priority Static Content
    HPDC, // High Priority Dynamic Content
} SchedulingPolicy;

typedef struct
{
    pthread_t id;
    int http_request_executions;
    int http_static_content_executions;
    int http_dynamic_content_executions;
} Thread;

typedef enum
{
    STATIC,
    DYNAMIC
} ContentType;


typedef struct Request
{
    int client_fd;
    long arrival_time;
    long dispatch_time;
    struct Request * next;
} Request;

typedef Request * Requests;

#endif // Custom_Data_Types_H
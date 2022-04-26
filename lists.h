#include "OperacoesPrimarias.h"

Requests create_requests_list ()
{
    Requests list = NULL;
    return list;
}

Requests create_request (int client_fd, RequestType type)
{
    Requests request = (Requests) malloc (sizeof(Request));
    if (request == NULL)
        return NULL;
        
    request->client_fd = client_fd;
    request->type = type;
    request->next = NULL;

    return request;
}

void free_request (Requests request)
{
    free(request);
    request = NULL;
}

bool is_list_empty (Requests list)
{
    if (list == NULL)
        return true;
    else
        return false;
}

int size_of_list (Requests list)
{
    int size = 0;
    while (list != NULL)
    {
        size++;
        list = list->next;
    }
    return size;
}

Requests append_left (int client_fd, RequestType type, Requests list)
{
    Requests list_aux = create_request (client_fd, type);
    if (list_aux == NULL)
        return list;
    list_aux->next = list;
    list = list_aux;
    return list;
}

Requests append_right (int client_fd, RequestType type, Requests list)
{
    Requests new_request, list_aux;
    new_request = create_request (client_fd, type);
    if (new_request == NULL)
        return list;

    if (list == NULL)
        return new_request;

    list_aux = list;
    while (list_aux->next != NULL)
        list_aux = list_aux->next;

    list_aux->next = new_request;
    return list;
}

Requests remove_request (int client_fd, RequestType type, Requests list, Requests list_aux)
{
    Requests request;
    if (compare_request (list->client_fd, list->type, client_fd, type) == 0)
    {
        request = list;
        list = list->next;
        free_request (request);
        return list;
    }
    if (compare_request (list_aux->next->client_fd, list_aux->next->type, client_fd, type) == 0)
    {
        request = list_aux->next;
        list_aux->next = request->next;
        free_request (request);
        return list;
    }

    return remove_request (client_fd, type, list, list_aux->next);
}
#include "OperacoesPrimarias.h"

Requests create_requests_list ()
{
    Requests list = NULL;
    return list;
}

Requests create_request (int client_fd)
{
    Requests request = (Requests) malloc (sizeof(Request));
    if (request == NULL)
        return NULL;
        
    request->client_fd = client_fd;
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

Requests append_left (int client_fd, Requests list)
{
    Requests list_aux = create_request (client_fd);
    if (list_aux == NULL)
        return list;
    list_aux->next = list;
    list = list_aux;
    return list;
}

Requests append_right (int client_fd, Requests list)
{
    Requests new_request, list_aux;
    new_request = create_request (client_fd);
    if (new_request == NULL)
        return list;

    if (list == NULL){
        return new_request;   
    }

    list_aux = list;
    while (list_aux->next != NULL)
        list_aux = list_aux->next;

    list_aux->next = new_request;
    return list;
}

Requests search_antecessor (int client_fd, Requests list)
{
    Requests ant = NULL;
    while (list != NULL && list->client_fd != client_fd)
    {
        ant = list;
        list = list->next;
    }
    return ant;
}

Requests remove_request (int client_fd, Requests list)
{
    Requests list1, list_antecessor;
    list_antecessor = search_antecessor (client_fd, list);
    if (list_antecessor == NULL)
    {
        list1 = list;
        list = list->next;
    }
    else
    {
        list1 = list_antecessor->next;
        list_antecessor->next = list1->next;
    }
    free_request (list1);
    return list;
}
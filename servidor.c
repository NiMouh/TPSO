#include <stdlib.h>
#include <stdbool.h>

#include "csapp.h"

#include "data_types.h"
#include "lists.h"

// Declaracoes de Variaveis Globais
Time start;
// Time end;

int server_port;
int max_threads;
int max_requests_handled;
SchedulingPolicy scheduling_policy;

pthread_mutex_t lock_list_request;

Thread * threads;
Requests requests;

// Declaracoes de Funcoes
bool parse_arguments (int ARGUMENTS_AMOUNT, char ** arguments)
{
    if (ARGUMENTS_AMOUNT != 5)
    {
        fprintf (stderr, "Uso: server <porta> <nr_threads> <max_pedidos> <alg_escalonamento>\n");
        return true;
    }

    server_port = atoi (arguments[1]);
    if (server_port <= 0)
    {
        fprintf (stderr, "Porta invalida\n");
        return true;
    }
    
    max_threads = atoi (arguments[2]);
    if (max_threads <= 0)
    {
        fprintf (stderr, "Numero de threads deve ser maior que 0!\n");
        return true;
    }

    max_requests_handled = atoi(arguments[3]);
    if (max_requests_handled <= 0)
    {
        fprintf (stderr, "Numero de pedidos possiveis deve ser maior que 0!\n");
        return true;
    }

    if (strcmp(arguments[4], "ANY") == 0)
        scheduling_policy = FIFO;
    else if (strcmp(arguments[4], "FIFO") == 0)
        scheduling_policy = FIFO;
    else if (strcmp(arguments[4], "HPSC") == 0)
        scheduling_policy = HPSC;
    else if (strcmp(arguments[4], "HPDC") == 0)
        scheduling_policy = HPDC;
    else
    {
        fprintf (stderr, "Algoritmo de escalonamento inválido!\nOpcoes possiveis sao: ANY, FIFO, HPSC ou HPDC.\n");
        return true;
    }

    return false;
}

void * thread_startup (void * position)
{
    int thread_position = (int) position;
    fprintf (stderr, "Thread %d iniciada\n", thread_position);
    return NULL;
}

bool create_threads ()
{
    threads = malloc (max_threads * sizeof (Thread)); // TODO: libertar memoria
    if (threads == NULL) {
        fprintf (stderr, "Erro ao alocar memoria para threads!\n");
        return false;
    }

    for (int index = 0; index < max_threads; index++)
    {
        Thread thread_prototype;
        int position = index;
        pthread_create (&(thread_prototype.id), NULL, thread_startup, (void * ) position);

        thread_prototype.http_request_executions         = 0;
        thread_prototype.http_dynamic_content_executions = 0;
        thread_prototype.http_static_content_executions  = 0;
        threads[index] = thread_prototype;
    }

    return true;
}

int main (int ARGUMENTS_AMOUNT, char ** arguments)
{
    gettimeofday (&start, NULL);

    bool exists_invalid_arguments = parse_arguments (ARGUMENTS_AMOUNT, arguments);
    if (exists_invalid_arguments)
        exit (EXIT_FAILURE);

    bool threads_created = create_threads ();
    if (threads_created == false)
        exit (EXIT_FAILURE);

    fprintf (stderr, "Servidor : executando na porta <%d>\n", server_port);

    int listener_fd = Open_listenfd(server_port);

    struct sockaddr_in client_address;

    while (true)
    {
        if ( size_of_list (requests) < max_requests_handled)
        {
            /*
            socklen_t client_address_size = sizeof(client_address);
            int client_fd = Accept(listener_fd, (SA *) &client_address, &client_address_size);

            Requests request = create_request (client_fd, type);

            pthread_mutex_lock(&lock_list_request);
            append_right (request->client_fd, request->type, requests);
            pthread_mutex_unlock(&lock_list_request);
            */
        }
    }
}

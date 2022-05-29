#include <stdlib.h>
#include <stdbool.h>

#include "csapp.h"

#include "data_types.h"
#include "lists.h"


Time start;
Time end;

int server_port;
int max_threads;
int max_requests_handled;
SchedulingPolicy scheduling_policy;

pthread_mutex_t lock_list_request;
pthread_mutex_t lock_statistics;

Thread * threads;
Requests * requests;

// Estatisticas
int stat_req_arrival_count = 0; // Quantidade de requisicoes recebidas
int stat_req_dispatch_count = 0; // Quantidade de requisicoes despachadas
int stat_req_completed_count = 0; // Quantidade de requisicoes concluidas

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

void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    while (strcmp(buf, "\r\n")) {                //line:netp:readhdrs:checkterm
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}

int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;

    if (!strstr(uri, "cgi-bin")) {                /* Static content *///line:netp:parseuri:isstatic
        strcpy(cgiargs, "");    //line:netp:parseuri:clearcgi
        strcpy(filename, ".");    //line:netp:parseuri:beginconvert1
        strcat(filename, uri);    //line:netp:parseuri:endconvert1
        if (uri[strlen(uri) - 1] == '/')    //line:netp:parseuri:slashcheck
            strcat(filename, "home.html");    //line:netp:parseuri:appenddefault
        return 1;
    } else {                /* Dynamic content *///line:netp:parseuri:isdynamic
        ptr = index(uri, '?');    //line:netp:parseuri:beginextract
        if (ptr) {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        } else
            strcpy(cgiargs, "");    //line:netp:parseuri:endextract
        strcpy(filename, ".");    //line:netp:parseuri:beginconvert2
        strcat(filename, uri);    //line:netp:parseuri:endconvert2
        return 0;
    }
}

void serve_static(int fd, char *filename, int filesize, int thread_index, Requests *request)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    pthread_mutex_lock (&lock_statistics);
        stat_req_completed_count++;
    pthread_mutex_unlock (&lock_statistics);

     gettimeofday (&end, NULL);

    long time_difference = (end.tv_sec - start.tv_sec);


    /* Send response headers to client */
    get_filetype(filename, filetype);    //line:netp:servestatic:getfiletype
    sprintf(buf, "HTTP/1.0 200 OK\r\n");    //line:netp:servestatic:beginserve
    sprintf(buf, "%sServidor: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sStatisticas \r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n", buf, filetype);
    pthread_mutex_lock (&lock_statistics);
        sprintf(buf, "%sServidor:\r\n" , buf);
        sprintf(buf, "%sRequisicoes recebidas: %d\r\n", buf, stat_req_arrival_count);
        sprintf(buf, "%sRequisicoes despachadas: %d\r\n", buf, stat_req_dispatch_count);
        sprintf(buf, "%sRequisicoes concluidas: %d\r\n", buf, stat_req_completed_count);

        sprintf(buf, "%sRequisicao:\r\n", buf);
        sprintf(buf, "%sTempo de chegada: %d\r\n", buf, (*request)->arrival_time);
        sprintf(buf, "%sHora de Despacho: %d\r\n", buf, (*request)->dispatch_time);
        sprintf(buf, "%sHora de Conclusao: %d\r\n", buf, time_difference);

        sprintf(buf, "%sThread %d:\r\n", buf, thread_index);
        sprintf(buf, "%sRequisicoes de Http recebidas: %d\r\n", buf, threads[thread_index].http_request_executions);
        sprintf(buf, "%sRequisicoes de Http Estaticas recebidas: %d\r\n", buf, threads[thread_index].http_static_content_executions);
        sprintf(buf, "%sRequisicoes de Http Dinamicas recebidas: %d\r\n\r\n", buf, threads[thread_index].http_dynamic_content_executions);

    pthread_mutex_unlock (&lock_statistics);
    Rio_writen(fd, buf, strlen(buf));    //line:netp:servestatic:endserve

    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);    //line:netp:servestatic:open
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);    //line:netp:servestatic:mmap
    Close(srcfd);        //line:netp:servestatic:close
    Rio_writen(fd, srcp, filesize);    //line:netp:servestatic:write
    Munmap(srcp, filesize);    //line:netp:servestatic:munmap
}

void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}

void serve_dynamic(int fd, char *filename, char *cgiargs, int thread_index, Requests *request)
{
    char buf[MAXLINE], *emptylist[] = {NULL};

    printf("Filename: %s\n", filename);

    int pipefd[2];

    /*Paul Crocker
        Changed so that client content is piped back to parent
    */
    Pipe(pipefd);

    if (Fork() == 0) {                /* child *///line:netp:servedynamic:fork
        /* Real server would set all CGI vars here */
        setenv("QUERY_STRING", cgiargs, 1);    //line:netp:servedynamic:setenv
        //Dup2 (fd, STDOUT_FILENO);	/* Redirect stdout to client *///line:netp:servedynamic:dup2
        Dup2(pipefd[1], STDOUT_FILENO);

        Execve(filename, emptylist, environ);    /* Run CGI program *///line:netp:servedynamic:execve
    }
    close(pipefd[1]);
    char content[1024]; //max size that cgi program will return

    int contentLength = read(pipefd[0], content, 1024);
    Wait(NULL);            /* Parent waits for and reaps child *///line:netp:servedynamic:wait

    pthread_mutex_lock (&lock_statistics);
        stat_req_completed_count++;
    pthread_mutex_unlock (&lock_statistics);

     gettimeofday (&end, NULL);

    long time_difference = (end.tv_sec - start.tv_sec);


    /* Generate the HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");    //line:netp:servestatic:beginserve
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, contentLength);
    sprintf(buf, "%sContent-type: text/html\r\n\r\n", buf);
    pthread_mutex_lock (&lock_statistics);
        sprintf(buf, "%sServidor:\r\n" , buf);
        sprintf(buf, "%sRequisicoes recebidas: %d\r\n", buf, stat_req_arrival_count);
        sprintf(buf, "%sRequisicoes despachadas: %d\r\n", buf, stat_req_dispatch_count);
        sprintf(buf, "%sRequisicoes concluidas: %d\r\n", buf, stat_req_completed_count);

        sprintf(buf, "%sRequisicao:\r\n", buf);
        sprintf(buf, "%sTempo de chegada: %d\r\n", buf, (*request)->arrival_time);
        sprintf(buf, "%sHora de Despacho: %d\r\n", buf, (*request)->dispatch_time);
        sprintf(buf, "%sHora de Conclusao: %d\r\n", buf, time_difference);

        sprintf(buf, "%sThread %d:\r\n", buf, thread_index);
        sprintf(buf, "%sRequisicoes de Http recebidas: %d\r\n", buf, threads[thread_index].http_request_executions);
        sprintf(buf, "%sRequisicoes de Http Estaticas recebidas: %d\r\n", buf, threads[thread_index].http_static_content_executions);
        sprintf(buf, "%sRequisicoes de Http Dinamicas recebidas: %d\r\n\r\n", buf, threads[thread_index].http_dynamic_content_executions);

    pthread_mutex_unlock (&lock_statistics);
    Rio_writen(fd, buf, strlen(buf));    //line:netp:servestatic:endserve

    Rio_writen(fd, content, contentLength);

}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    /* Fazer primeiro visto que precisamos de saber o tamanho do body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=" "ffffff" ">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));

    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    Rio_writen(fd, buf, strlen(buf));

    sprintf(buf, "Content-length: %d\r\n\r\n", (int) strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));


    Rio_writen(fd, body, strlen(body));
}

void doit(int fd, int thread_index, Requests *requests) {
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    /* Read request line and headers */
    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);    //line:netp:doit:readrequest
    sscanf(buf, "%s %s %s", method, uri, version);    //line:netp:doit:parserequest
    
    if (strcasecmp(method, "GET")) {                //line:netp:doit:beginrequesterr
        clienterror(fd, method, "501", "Not Implemented", "Tiny does not implement this method");
        return;
    }                //line:netp:doit:endrequesterr
    read_requesthdrs(&rio);    //line:netp:doit:readrequesthdrs

    /* Parse URI from GET request */
    is_static = parse_uri(uri, filename, cgiargs);    //line:netp:doit:staticcheck
    if (stat(filename, &sbuf) < 0) {                //line:netp:doit:beginnotfound
        clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
        return;
    }                //line:netp:doit:endnotfound

    if (is_static) {                /* Serve static content */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {            //line:netp:doit:readable
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
            return;
        }
        Thread t = threads[thread_index];
        t.http_static_content_executions++;
        threads[thread_index] = t;
        serve_static(fd, filename, sbuf.st_size, thread_index, requests);    //line:netp:doit:servestatic
    } else {                /* Serve dynamic content */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {            //line:netp:doit:executable
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
            return;
        }
        Thread t = threads[thread_index];
        t.http_dynamic_content_executions++;
        threads[thread_index] = t;
        serve_dynamic(fd, filename, cgiargs, thread_index, requests);    //line:netp:doit:servedynamic
    }
}

ContentType get_content_type (int client_fd)
{
    rio_t rio;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];

    Rio_readinitb(&rio, client_fd);
    Rio_readlineb(&rio, buf, MAXLINE);    //line:netp:doit:readrequest
    sscanf(buf, "%s %s %s", method, uri, version);    //line:netp:doit:parserequest

    if (!strstr(uri, "cgi-bin"))
        return DYNAMIC;
    else
        return STATIC;
}

void * thread_startup (void * position)
{
    while (true)
    {
        pthread_mutex_lock (&lock_list_request);

            bool list_is_empty;

            if (scheduling_policy == FIFO)
                list_is_empty = is_list_empty (*requests);
            else
            {
                list_is_empty = is_list_empty (requests[0]) && is_list_empty (requests[1]); 
            }


            if (!list_is_empty)
            {
                pthread_mutex_lock (&lock_statistics);
                    stat_req_dispatch_count++;
                pthread_mutex_unlock (&lock_statistics);

                gettimeofday (&end, NULL);

                long time_difference = (end.tv_sec - start.tv_sec);

                (*requests)->dispatch_time = time_difference;

                Thread thread = threads[(int) position];
                thread.http_request_executions++;
                threads[(int) position] = thread;

                int thread_position = (int) position;
                
                int client_fd;

                switch (scheduling_policy)
                {
                    case FIFO:
                        client_fd = (*requests)->client_fd;

                        doit (client_fd, position, requests);
                        Close(client_fd);

                        *requests = remove_request (client_fd, *requests);
                        break;
                    case HPSC:
                        if (!is_list_empty (requests[0]))
                        {
                            client_fd = requests[0]->client_fd;

                            doit (client_fd, position, requests);
                            Close(client_fd);

                            requests[0] = remove_request (client_fd, requests[0]);
                        }
                        else
                        {
                            client_fd = requests[1]->client_fd;

                            doit (client_fd, position, requests);
                            Close(client_fd);

                            requests[1] = remove_request (client_fd, requests[1]);
                        }
                        break;
                    case HPDC:
                        if (!is_list_empty (requests[1]))
                        {
                            client_fd = requests[1]->client_fd;

                            doit (client_fd, position, requests);
                            Close(client_fd);

                            requests[1] = remove_request (client_fd, requests[1]);
                        }
                        else
                        {
                            client_fd = requests[0]->client_fd;

                            doit (client_fd, position, requests);
                            Close(client_fd);

                            requests[0] = remove_request (client_fd, requests[0]);
                        }
                        break;
                }       
                    
            }

        pthread_mutex_unlock(&lock_list_request);
    }    
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

    pthread_mutex_init (&lock_list_request, NULL);

    pthread_mutex_lock (&lock_list_request);

        switch (scheduling_policy)
        {
            case FIFO:
                requests = (Requests *) malloc (sizeof (Requests));
                break;
            case HPSC:
                requests = (Requests *) malloc (sizeof (Requests) * 2);
                break;
            case HPDC:
                requests = (Requests *) malloc (sizeof (Requests) * 2);
                break;
        }

    pthread_mutex_unlock (&lock_list_request);

    bool threads_created = create_threads ();
    if (threads_created == false)
        exit (EXIT_FAILURE);

    fprintf (stderr, "Servidor : executando na porta <%d>\n", server_port);

    int listener_fd = Open_listenfd(server_port);

    struct sockaddr_in client_address;


    fprintf (stderr, "Servidor : iniciando o loop de conexoes\n");
    fprintf (stderr, "Servidor : estado da lista: %s\n", is_list_empty (*requests) ? "vazia" : "nao vazia");

    while (true)
    {
        pthread_mutex_lock (&lock_list_request);

            int list_size;

            if (scheduling_policy == FIFO)
                list_size = size_of_list (*requests);
            else
                list_size = size_of_list (requests[0]) + size_of_list (requests[1]);

        pthread_mutex_unlock (&lock_list_request);

        if ( list_size < max_requests_handled)
        {
            socklen_t client_address_size = sizeof(client_address);
            int client_fd = Accept(listener_fd, (SA *) &client_address, &client_address_size);
            fprintf (stderr, "Servidor: conexao recebida (%d)\n", client_fd);

            pthread_mutex_lock (&lock_list_request);

                Requests request = create_request (client_fd);

                gettimeofday (&end, NULL);

                long time_difference = (end.tv_sec - start.tv_sec);
                printf ("Arrival time: %ld\n", time_difference);
                request->arrival_time = time_difference;

                switch (scheduling_policy)
                {
                    case FIFO:
                        *requests = append_right (request->client_fd, *requests);
                        break;
                    case HPSC:
                        if (get_content_type (client_fd) == STATIC)
                            requests[0] = append_right (request->client_fd, requests[0]);
                        else
                            requests[1] = append_right (request->client_fd, requests[1]);
                        break;
                    case HPDC:
                        if (get_content_type (client_fd) == STATIC)
                            requests[1] = append_right (request->client_fd, requests[1]);
                        else
                            requests[0] = append_right (request->client_fd, requests[0]);
                        break;
                }
            pthread_mutex_unlock (&lock_list_request);

            pthread_mutex_lock (&lock_list_request);
                stat_req_arrival_count++;
            pthread_mutex_unlock (&lock_list_request);
            
        }
    }
}

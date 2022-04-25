#include "csapp.h"
#include "ListasLigadasSimples.h"

typedef enum {
    FIFO,
    HPSC,
    HPDC
} SchedulingType;

typedef struct {
    int ID;
    int Ficheiro;
    int HTTP_REQUEST_EXEC;
    int HTTP_STATIC_REQUEST;
    int HTTP_DYNAMIC_REQUEST;
} THREAD;

typedef struct timeval Time;
Time start;

THREAD threads[MAX_THREADS - 1];

pthread_mutex_t lock_list_request;

PNodo requests;

void *thread() {

}

void doit(int fd);

void read_requesthdrs(rio_t *rp);

int parse_uri(char *uri, char *filename, char *cgiargs);

void serve_static(int fd, char *filename, int filesize);

void get_filetype(char *filename, char *filetype);

void serve_dynamic(int fd, char *filename, char *cgiargs);

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);


int main(int ARGUMENTS_AMOUNT, char **arguments) {
    gettimeofday(&start, NULL);

    if (ARGUMENTS_AMOUNT != 5) {
        fprintf(stderr, "Uso: server <porta> <nr_threads> <max_pedidos> <alg_escalonamento>\n");
        exit(0);
    }

    const int PORT = atoi(arguments[1]);
    const int MAX_THREADS = atoi(arguments[2]);
    if (MAX_THREADS <= 0) {
        fprintf(stderr, "Numero de threads deve ser maior que 0!\n");
        exit(0);
    }

    const int MAX_REQUESTS_HANDLED = atoi(arguments[3]);
    if (MAX_REQUESTS_HANDLED <= 0) {
        fprintf(stderr, "Numero de pedidos possiveis deve ser maior que 0!\n");
        exit(0);
    }

    SchedulingType scheduling_algorithm;
    if (strcmp(arguments[4], "ANY") == 0)
        scheduling_algorithm = FIFO;
    else if (strcmp(arguments[4], "FIFO") == 0)
        scheduling_algorithm = FIFO;
    else if (strcmp(arguments[4], "HPSC") == 0)
        scheduling_algorithm = HPSC;
    else if (strcmp(arguments[4], "HPDC") == 0)
        scheduling_algorithm = HPDC;
    else {
        fprintf(stderr, "Algoritmo de escalonamento inválido!\nOpcoes possiveis sao: ANY, FIFO, HPSC ou HPDC.\n");
        exit(0);
    }

    fprintf(stderr, "Servidor : executando na porta <%d>\n", PORT);

    int listen_file_descriptor = Open_listenfd(PORT);

    struct sockaddr_in client_address;

    // CRIAR O APONTADOR PARA A LISTA E A LISTA TEM QUE TER (MAX_REQUEST_HANDLED) ELEMENTOS

    for (int i = 0; i < MAX_THREADS; i++) {
        THREAD th;
        pthread_create(&th.ID, NULL, (void *) thread, NULL);
        th.HTTP_DYNAMIC_REQUEST = 0;
        th.HTTP_STATIC_REQUEST = 0;
        th.HTTP_REQUEST_EXEC = 0;
        th.Ficheiro = -1;
        threads[i] = th;
    }

    pthread_mutex_init(&lock_list_request, NULL);
    pthread_mutex_lock(&lock_list_request);


    while (1) {
        if (!listaVazia(requests)) {
            unsigned int client_length = sizeof(client_address);
            int connection_descriptor = Accept(listen_file_descriptor, (SA *) &client_address,
                                               &client_length);
            requests = ins
        }

        //line:netp:tiny:accept
        doit(connection_descriptor);        //line:netp:tiny:doit
        Close(connection_descriptor);        //line:netp:tiny:close
    }

}


int numeroRequestStat = 0;

/* $end tinymain */

/*
 * doit - handle one HTTP request/response transaction
 */
/* $begin doit */
void doit(int fd) {
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
        serve_static(fd, filename, sbuf.st_size);    //line:netp:doit:servestatic
    } else {                /* Serve dynamic content */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {            //line:netp:doit:executable
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
            return;
        }
        serve_dynamic(fd, filename, cgiargs);    //line:netp:doit:servedynamic
    }
}

/* $end doit */

/*
 * read_requesthdrs - read and parse HTTP request headers
 */
/* $begin read_requesthdrs */
void read_requesthdrs(rio_t *rp) {
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    while (strcmp(buf, "\r\n")) {                //line:netp:readhdrs:checkterm
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}

/* $end read_requesthdrs */

/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */
/* $begin parse_uri */
int parse_uri(char *uri, char *filename, char *cgiargs) {
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

/* $end parse_uri */

/*
 * serve_static - copy a file back to the client
 */
/* $begin serve_static */
void serve_static(int fd, char *filename, int filesize) {
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /* Send response headers to client */
    get_filetype(filename, filetype);    //line:netp:servestatic:getfiletype
    sprintf(buf, "HTTP/1.0 200 OK\r\n");    //line:netp:servestatic:beginserve
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sRequestStat: %d\r\n", buf, numeroRequestStat++);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));    //line:netp:servestatic:endserve

    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);    //line:netp:servestatic:open
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);    //line:netp:servestatic:mmap
    Close(srcfd);        //line:netp:servestatic:close
    Rio_writen(fd, srcp, filesize);    //line:netp:servestatic:write
    Munmap(srcp, filesize);    //line:netp:servestatic:munmap
}

/*
 * get_filetype - derive file type from file name
 * Deverá adicionar mais tipos
 */
void get_filetype(char *filename, char *filetype) {
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}

/* $end serve_static */

/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
/* $begin serve_dynamic */
void serve_dynamic(int fd, char *filename, char *cgiargs) {
    char buf[MAXLINE], *emptylist[] = {NULL};

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


    /* Generate the HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");    //line:netp:servestatic:beginserve
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sRequestStat: %d\r\n", buf, numeroRequestStat++);
    sprintf(buf, "%sContent-length: %d\r\n", buf, contentLength);
    sprintf(buf, "%sContent-type: text/html\r\n\r\n", buf);
    Rio_writen(fd, buf, strlen(buf));    //line:netp:servestatic:endserve

    Rio_writen(fd, content, contentLength);

}

/* $end serve_dynamic */

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
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
    sprintf(buf, "%sRequestStat: %d\r\n", buf, numeroRequestStat++);
    Rio_writen(fd, buf, strlen(buf));

    sprintf(buf, "Content-length: %d\r\n\r\n", (int) strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));


    Rio_writen(fd, body, strlen(body));
}

/* $end clienterror */
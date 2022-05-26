#define DEBUG 1

#include "example.h"
#include <assert.h>
#include <stdbool.h>
#include "csapp.h"

#define MAX_THREADS 20

// Threads
pthread_mutex_t mutex;

// Semaphores
sem_t sem_fila;

// ID Array
int *ids;


// Create function cliente
// client [host] [portnum] [threads] [schedalg] [filename1] [filename2]

char request[MAX_REQUEST_LEN];
char request_template[] = "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n";
struct protoent *protoent;


char *hostname = "localhost";
unsigned short server_port = 8080;  //default port
char *file;
char *fileindex = "/";
char *filedynamic = "/cgi-bin/adder?150&100";
char *filestatic = "/godzilla.jpg";

void *cliente(void *arg) {
    char buffer[BUFSIZ];
    enum CONSTEXPR {
        MAX_REQUEST_LEN = 1024
    };


    in_addr_t in_addr;
    int request_len;
    int socket_file_descriptor;
    ssize_t nbytes;
    struct hostent *hostent;
    struct sockaddr_in sockaddr_in;

    // Lock mutex
    pthread_mutex_lock(&mutex);

    /* Critic ZONE*/
    if (arg[1] == NULL) {
        hostname = arg[1];
        if (arg[2] == NULL) {
            server_port = strtoul(arg[2], NULL, 10);
            if (arg[3] == NULL) {
                file = arg[3];
                else
                file = fileindex;  //ou escolher outra filedynamic filestatic

                //construção do pedido de http
                request_len = snprintf(request, MAX_REQUEST_LEN, request_template, file, hostname);
                if (request_len >= MAX_REQUEST_LEN) {
                    fprintf(stderr, "request length large: %d\n", request_len);
                    exit(EXIT_FAILURE);
                }

                /* Build the socket. */
                protoent = getprotobyname("tcp");
                if (protoent == NULL) {
                    perror("getprotobyname");
                    exit(EXIT_FAILURE);
                }

                //Open the socket
                socket_file_descriptor = Socket(AF_INET, SOCK_STREAM, protoent->p_proto);

                /* Build the address. */
                // 1 get the hostname address
                hostent = Gethostbyname(hostname);

                in_addr = inet_addr(inet_ntoa(*(struct in_addr *) *(hostent->h_addr_list)));
                if (in_addr == (in_addr_t) - 1) {
                    fprintf(stderr, "error: inet_addr(\"%s\")\n", *(hostent->h_addr_list));
                    exit(EXIT_FAILURE);
                }
                sockaddr_in.sin_addr.s_addr = in_addr;
                sockaddr_in.sin_family = AF_INET;
                sockaddr_in.sin_port = htons(server_port);

                /* Ligar ao servidor */
                Connect(socket_file_descriptor, (struct sockaddr *) &sockaddr_in, sizeof(sockaddr_in));

                /* Send HTTP request. */
                Rio_writen(socket_file_descriptor, request, request_len);

                /* Read the response. */
                if (DEBUG) fprintf(stderr, "debug: before first read\n");

                rio_t rio;
                char buf[MAXLINE];



                /* Leituras das linhas da resposta. Os cabecalhos — Headers */
                const int numeroDeHeaders = 5;
                Rio_readinitb(&rio, socket_file_descriptor);
                for (int k = 0; k < numeroDeHeaders; k++) {
                    Rio_readlineb(&rio, buf, MAXLINE);

                    //Envio das estatisticas para o canal de standard error
                    if (strstr(buf, "Stat") != NULL)
                        fprintf(stderr, "STATISTIC : %s", buf);
                }

                //Ler o resto da resposta — o corpo de resposta.
                //Vamos ler em blocos caso que seja uma resposta grande.
                while ((nbytes = Rio_readn(socket_file_descriptor, buffer, BUFSIZ)) > 0) {
                    if (DEBUG) fprintf(stderr, "debug: after a block read\n");
                    //commentar a lina seguinte se não quiser ver o output
                    Rio_writen(STDOUT_FILENO, buffer, nbytes);
                }

                if (DEBUG) fprintf(stderr, "debug: after last read\n");

                Close(socket_file_descriptor);

            }
        }
    }
    /* END OF ZONE*/
    // Unlock mutex
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char **argv) {
    int N = atoi(argv[3]);

    // Check if N is less than MAX_THREADS
    if (N > MAX_THREADS) {
        // If N is greater than MAX_THREADS, print error message
        printf("N must be less than %d\n", MAX_THREADS);
        N = MAX_THREADS;
    }

    // Declare a thread array with N elements malloc
    pthread_t *threads = malloc(N * sizeof(pthread_t));

    // initialize mutex
    pthread_mutex_init(&mutex, NULL);
    sem_init(&sem_fila, 0, 0);

    // Declare an id array with N elements malloc
    *ids = malloc(N * sizeof(int));
    // for loop to give values to ids
    for (int i = 0; i < N; i++) {
        ids[i] = i;
    }

    // create N semaphores
    for (int i = 0; i < N; i++) {
        sem_init(&sem_fila, 0, id[i]);
    }
    // initialize loops
    for (int i = 0; i < N; i++) {
        // sem_wait semaphore id
        sem_wait(&sem_fila);
        // Create thread that each send and receive one request
        pthread_create(&threads[i], NULL, cliente,
        void *argv);
        // sem_post semaphore id + 1
        sem_post(&sem_fila + 1);
    }
    // for cicle to wait all threads
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    // destroy mutex
    pthread_mutex_destroy(&mutex);
    sem_destroy(&sem_fila);

    exit(EXIT_SUCCESS);
}
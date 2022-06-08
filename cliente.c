#define DEBUG 1

#include "example.h"
#include <assert.h>
#include <stdbool.h>
#include "csapp.h"
#include "libtomcrypt/tomcrypt_hash.h"

typedef enum
{
    FIFO, // First In First Out
	CONCUR, // Concurrent Groups
} SchedulingPolicy;

char* host_name;
int server_port;
int max_threads;
SchedulingPolicy scheduling_policy;
int total_files = 0;
char* files[2];
int file_index = 0;

pthread_t * threads;
int * ids;
sem_t * semaphores;
pthread_barrier_t barrier;
pthread_mutex_t file_comutation_mutex;
pthread_mutex_t thread_lock;

bool check_hash_result(char* hash, int difficulty, char* msg)
{
	unsigned char tmp[20];
	hash_state md;
	char *powStr=(char*) malloc( strlen(msg) + 10 + 1);
	sprintf(powStr, "%s%u", msg, difficulty);

	sha1_init(&md);
	sha1_process(&md, (unsigned char *)powStr, (unsigned long)strlen(powStr));
	sha1_done(&md, tmp);

	int checks = 0;

	for (int i=0; i< difficulty; i++) {
		if (tmp[i] == 0) {
			checks++;
		}
	}

	if (checks == difficulty) {
		if(strcmp(hash, powStr) == 0)
			return true;
	}
	
	return false;

}

int retrieve_nonce (char* nonce_ptr)
{
	char *p;
	int nonce;
	p = strchr(nonce_ptr, ':');
	p++;
	nonce = atoi(p);
	return nonce;
}

char * retrieve_hash (char * hash_pointer)
{
	char *p;
	p = strchr(hash_pointer, ':');
	p++;
	return p;
}

void * cliente (void * ids)
{
	while (true)
	{
		
		char buffer[BUFSIZ];
		enum CONSTEXPR {
			MAX_REQUEST_LEN = 1024
		};

		char request[MAX_REQUEST_LEN];
		char request_template[] = "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n";
		struct protoent *protoent;

		in_addr_t in_addr;
		int request_len;
		int socket_file_descriptor;
		ssize_t nbytes;
		struct hostent *hostent;
		struct sockaddr_in sockaddr_in;		
		
		
		
		int id = *((int *) ids);
		switch (scheduling_policy)
		{
			case FIFO:
			
				printf ("Thread %d: FIFO\n", id);
				sem_wait(&semaphores[id]);

					/* Construir pedido HTTP */
					if (total_files == 1)
					{
						request_len = snprintf(request, MAX_REQUEST_LEN, request_template, files[0], host_name);
						if (request_len >= MAX_REQUEST_LEN) {
							fprintf(stderr, "Comprimento do pedido grande: %d\n", request_len);
							exit(EXIT_FAILURE);
						}
					}	
					else
					{
						pthread_mutex_lock(&file_comutation_mutex);

							if (file_index == 0)
								file_index = 1;
							else
								file_index = 0;
							
							request_len = snprintf(request, MAX_REQUEST_LEN, request_template, files[file_index], host_name);
							if (request_len >= MAX_REQUEST_LEN) {
								fprintf(stderr, "Comprimento do pedido grande: %d\n", request_len);
								exit(EXIT_FAILURE);
							}

						pthread_mutex_unlock(&file_comutation_mutex);
					}
					
					
					/* Construir Socket */
					protoent = getprotobyname("tcp");
					if (protoent == NULL) {
						perror("getprotobyname");
						exit(EXIT_FAILURE);
					}
					
					/* Abrir Socket */
					socket_file_descriptor = Socket(AF_INET, SOCK_STREAM, protoent->p_proto);
					
					/* Construir Endereco */
					hostent = Gethostbyname(host_name);

					in_addr = inet_addr(inet_ntoa(*(struct in_addr *) *(hostent->h_addr_list)));
					if (in_addr == (in_addr_t) - 1) {
						fprintf(stderr, "error: inet_addr(\"%s\")\n", *(hostent->h_addr_list));
						exit(EXIT_FAILURE);
					}
					sockaddr_in.sin_addr.s_addr = in_addr;
					sockaddr_in.sin_family = AF_INET;
					sockaddr_in.sin_port = htons(server_port);

					
					/* Ligar ao Servidor */
					Connect(socket_file_descriptor, (struct sockaddr *) &sockaddr_in, sizeof(sockaddr_in));

					
					/* Enviar pedido HTTP */
					Rio_writen(socket_file_descriptor, request, request_len);

					/* Leitura do pedido HTTP */
					while ((nbytes = Rio_readn(socket_file_descriptor, buffer, BUFSIZ)) > 0) {
					if (DEBUG) fprintf(stderr, "debug: apos leitura de bloco\n");
						if(strstr(buffer, "Proof of Work") != NULL)
						{
							char* nonce_ptr = strstr(buffer, "Nonce: ");
							char* hash_ptr = strstr(buffer, "Hash: ");
							int nonce = retrieve_nonce(nonce_ptr);
							printf ("[%d]\n", nonce);
							char* hash = retrieve_hash(hash_ptr);
							hash[strlen(hash) - 3] = '\0';
							printf ("[%s]\n", hash);
							// COMPARAR RESULTADO E MOSTRAR
							/*
							if(check_hash_result(hash, nonce, strtok(files[0], "&")))
								printf("\nSUCESSO");
							else
								printf("\nFALHA");
								*/
						}
						printf("%s\n", buffer);
						

							//Rio_writen(STDOUT_FILENO, buffer, nbytes);
					}

				if (id + 1 == max_threads)
					sem_post(&semaphores[0]);
				else
					sem_post(&semaphores[id + 1]);
				break;
			case CONCUR:
				pthread_mutex_lock (&thread_lock);
				/* Construir pedido HTTP */
				if (total_files == 1)
				{
					request_len = snprintf(request, MAX_REQUEST_LEN, request_template, files[0], host_name);
					if (request_len >= MAX_REQUEST_LEN) {
						fprintf(stderr, "Comprimento do pedido grande: %d\n", request_len);
						exit(EXIT_FAILURE);
					}
				}	
				else
				{
					pthread_mutex_lock(&file_comutation_mutex);

						if (file_index == 0)
							file_index = 1;
						else
							file_index = 0;
						
						request_len = snprintf(request, MAX_REQUEST_LEN, request_template, files[file_index], host_name);
						if (request_len >= MAX_REQUEST_LEN) {
							fprintf(stderr, "Comprimento do pedido grande: %d\n", request_len);
							exit(EXIT_FAILURE);
						}

					pthread_mutex_unlock(&file_comutation_mutex);
				}
				
				
				/* Construir Socket */
				protoent = getprotobyname("tcp");
				if (protoent == NULL) {
					perror("getprotobyname");
					exit(EXIT_FAILURE);
				}
				
				/* Abrir Socket */
				socket_file_descriptor = Socket(AF_INET, SOCK_STREAM, protoent->p_proto);
				
				/* Construir Endereco */
				hostent = Gethostbyname(host_name);

				in_addr = inet_addr(inet_ntoa(*(struct in_addr *) *(hostent->h_addr_list)));
				if (in_addr == (in_addr_t) - 1) {
					fprintf(stderr, "error: inet_addr(\"%s\")\n", *(hostent->h_addr_list));
					exit(EXIT_FAILURE);
				}
				sockaddr_in.sin_addr.s_addr = in_addr;
				sockaddr_in.sin_family = AF_INET;
				sockaddr_in.sin_port = htons(server_port);

				
				/* Ligar ao Servidor */
				Connect(socket_file_descriptor, (struct sockaddr *) &sockaddr_in, sizeof(sockaddr_in));
				
				/* Enviar pedido HTTP */
				Rio_writen(socket_file_descriptor, request, request_len);

				/* Leitura do pedido HTTP */
				while ((nbytes = Rio_readn(socket_file_descriptor, buffer, BUFSIZ)) > 0) {
				if (DEBUG) fprintf(stderr, "debug: apos leitura de bloco\n");

					Rio_writen(STDOUT_FILENO, buffer, nbytes);
				}
				pthread_mutex_unlock (&thread_lock);
				pthread_barrier_wait(&barrier);
				break;
		}

		Close(socket_file_descriptor);
	}

	return NULL;
}

bool parse_arguments (int ARGUMENTS_AMOUNT, char ** arguments)
{
	if (ARGUMENTS_AMOUNT < 6)
	{
		fprintf(stderr, "Uso: %s <hospedeiro> <porta> <nr_threads> <alg_escalonamento> <ficheiro1> (<ficheiro2>)\n", arguments[0]);
		return true;
	}

	host_name = arguments[1];
	server_port = atoi(arguments[2]);

	if (server_port <= 0)
	{
		fprintf(stderr, "Erro: porta invalida\n");
		return true;
	}
	
	max_threads = atoi(arguments[3]);
	if (max_threads <= 0)
	{
		fprintf(stderr, "Erro: Numero de threads deve ser maior que 0.\n");
		return true;
	}

	if (strcmp(arguments[4], "FIFO") == 0)
	{
		scheduling_policy = FIFO;
	}
	else if (strcmp(arguments[4], "CONCUR") == 0)
	{
		scheduling_policy = CONCUR;
	}
	else
	{
		fprintf(stderr, "Erro: Algoritmo de escalonamento invalido.\n");
		fprintf(stderr, "Opcoes: FIFO ou CONCUR\n");
		return true;
	}

	files[0] = malloc (strlen(arguments[5]) + 1);
	strcpy(files[0], arguments[5]);
	total_files++;

	if (ARGUMENTS_AMOUNT == 7)
	{
		files[1] = malloc (strlen(arguments[6]) + 1);
		strcpy(files[1], arguments[6]);
		total_files++;
	}

	return false;
}

bool create_semaphores ()
{
	semaphores = malloc (max_threads * sizeof(sem_t));
	if (semaphores == NULL)
	{
		fprintf(stderr, "Erro: Nao foi possivel alocar memoria para os semaforos.\n");
		return false;
	}

	sem_init(&semaphores[0], 0, 1);

	for (int i = 1; i < max_threads; i++)
	{
		sem_init(&semaphores[i], 0, 0);
	}

	return true;
}

bool create_threads ()
{
	threads = malloc (max_threads * sizeof (pthread_t));
	if (threads == NULL)
	{
		fprintf(stderr, "Erro: Nao foi possivel alocar memoria para as threads.\n");
		return false;
	}

	ids = malloc (max_threads * sizeof (int));
	if (ids == NULL)
	{
		fprintf(stderr, "Erro: Nao foi possivel alocar memoria para os ids das threads.\n");
		return false;
	}

	for (int index = 0; index < max_threads; index++)
	{
		ids[index] = index;
		if (pthread_create(&threads[index], NULL, &cliente, &ids[index]) != 0)
		{
			fprintf(stderr, "Erro: Nao foi possivel criar a thread %d.\n", index);
			return false;
		}
	}
	return true;
}


// client [host] [portnum] [threads] [schedalg] [filename1] [filename2]
int main (int ARGUMENTS_AMOUNT, char ** arguments)
{

	if (parse_arguments(ARGUMENTS_AMOUNT, arguments))
		exit(EXIT_FAILURE);
	
	pthread_mutex_init(&file_comutation_mutex, NULL);
	pthread_mutex_init(&thread_lock, NULL);

	switch (scheduling_policy)
	{
		case FIFO:
			if(!create_semaphores())
				exit(EXIT_FAILURE);
			break;
		case CONCUR:
			pthread_barrier_init(&barrier, NULL, max_threads);
			break;
	}

	if (!create_threads())
		exit(EXIT_FAILURE);
	
	while (true);

	exit(EXIT_SUCCESS);
}
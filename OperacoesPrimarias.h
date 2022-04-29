#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "Aleatorio.h"
#include "data_types.h"

bool compare_request (int client_fd1, int client_fd2)
{
    return client_fd1 == client_fd2;
}
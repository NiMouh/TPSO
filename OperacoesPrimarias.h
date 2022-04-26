#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "Aleatorio.h"
#include "data_types.h"

bool compare_request (int client_fd1, RequestType type1, int client_fd2, RequestType type2)
{
    if (client_fd1 == client_fd2 && type1 == type2)
        return true;
    else
        return false;
}
/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include <stdio.h>
#include "csapp.h"
#include "libtomcrypt/tomcrypt_hash.h"

int main(void) {
    char *buf, *p;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1=0, n2=0;

    int difficulty;

    /* Extrai os 2 argumentos */
    if ((buf = getenv("QUERY_STRING")) != NULL) {
        p = strchr(buf, '&');
        *p = '\0';              //anular o & dentro do string p -  para aproveitar no strcpy a seguir
        strcpy(arg1, buf);
        strcpy(arg2, p+1);
        difficulty = atoi(arg2);
    }
    
    unsigned char tmp[20];
    hash_state md;

    unsigned long k=0;
    //allocate space for a long integer (10 digits) + 1 for any terminating null character necessary
    char *powStr=(char*) malloc( strlen(arg1) + 10 + 1);

    sprintf(content, "Proof of Work: ");
 
    while (1) {
    
        //TENHO a certeza que alocei bem memoria para sprintf
        #pragma GCC diagnostic ignored "-Wformat-overflow"
        sprintf(powStr, "%s%lu", arg1, k);

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
            sprintf(content, "%sNonce: %lu :\n", content, k);
            sprintf(content, "%sHash: ", content);
            for (int i=0;i<20;i++)
                sprintf(content, "%s%x ", content, tmp[i]);
            sprintf(content, "%s:\n", content);
            break;
        }
        if (k%5000000==0)
            sprintf(content, "%ssearching %lu\n", content, k);
        k++;
    }

    printf("%s", content);
    exit(0);
}
/* $end adder */

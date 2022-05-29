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

    /* Extrai os 2 argumentos */
    if ((buf = getenv("QUERY_STRING")) != NULL) {
        p = strchr(buf, '&');
        *p = '\0';              //anular o & dentro do string p -  para aproveitar no strcpy a seguir
        strcpy(arg1, buf);
        strcpy(arg2, p+1);
        n1 = atoi(arg1);
        n2 = atoi(arg2);
    }

    unsigned char tmp[20];
    hash_state md;

    unsigned long k=0;
    //char *powMsg="abcsfsrgfresgdrsgdsgdtgdtghdthdt";
    //allocate space for a long integer (10 digits) + 1 for any terminating null character necessary
    char *powStr=(char*) malloc( strlen(arg1) + 10 + 1);

    sprintf(content, "Welcome to cripto: ");
 
    while (1) {
    
        //TENHO a certeza que alocei bem memoria para sprintf
        #pragma GCC diagnostic ignored "-Wformat-overflow"
        sprintf(powStr, "%s%lu", arg1,k);

        sha1_init(&md);
        sha1_process(&md, (unsigned char *)powStr, (unsigned long)strlen(powStr));
        sha1_done(&md, tmp);

        if ( 0==tmp[0] && 0==tmp[1] && 0==tmp[2])
        {
            printf("POW done : k = %lu\n",k);
            sprintf(content, "%sThe answer is: ", content);
            for (int i=0;i<20;i++)
                sprintf(content, "%s%x ",buf, tmp[i]);
            printf("\n");
            break;
        }
        if (k%5000000==0)
            printf("searching %lu\n",k);
        k++;
    }

    sprintf(content, "%sThanks for visiting!\r\n", content);
    fflush(stdout);
    printf("%s", content);
    fflush(stdout);
    exit(0);
}
/* $end adder */

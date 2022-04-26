#include "OperacoesPrimarias.h"

typedef enum{
    STATIC,DYNAMIC;
} REQUEST_TYPE;
struct Nodo {
    int CONECTION_DESC;
    REQUEST_TYPE TYPE;
    struct Nodo *Prox;
};

typedef struct Nodo *PNodo;


PNodo criarLista() {
    PNodo L;
    L = NULL;
    return L;
}

PNodo criarNodo(int X) {
    PNodo P;
    P = (PNodo) malloc(sizeof(struct Nodo));
    if (P == NULL)
        return NULL;
    P->CONECTION_DESC = X;
    P->Prox = NULL;
    return P;
}

void libertarNodo(PNodo P) {
    free(P);
    P = NULL;
}

int listaVazia(PNodo L) {
    if (L == NULL)
        return 1;
    else
        return 0;
}


void mostrarListaInicio(PNodo L) {
    PNodo P = L;
    while (P != NULL) {
        mostrarElemento(P->CONECTION_DESC);
        P = P->Prox;
    }
    printf("\n\n");
}

void mostrarListaInicioRec(PNodo L) {
    if (L != NULL) {
        mostrarElemento(L->CONECTION_DESC);
        mostrarListaInicioRec(L->Prox);
    }
}

void mostrarListaFimRec(PNodo L) {
    if (L != NULL) {
        mostrarListaFimRec(L->Prox);
        mostrarElemento(L->CONECTION_DESC);
    }
}

int tamanhoLista(PNodo L) {
    int tam = 0;
    while (L != NULL) {
        tam++;
        L = L->Prox;
    }
    return tam;
}

int tamanhoListaRec(PNodo L) {
    if (L == NULL)
        return 0;
    else
        return 1 + tamanhoListaRec(L->Prox);
}

int pesquisarLista(int X, PNodo L) {
    while (L != NULL && compararElementos(L->CONECTION_DESC, X) != 0)
        L = L->Prox;
    if (L == NULL)
        return 0;
    else
        return 1;
}

int pesquisarListaRec(int X, PNodo L) {
    if (L == NULL)
        return 0;
    if (compararElementos(L->CONECTION_DESC, X) == 0)
        return 1;
    else
        return pesquisarListaRec(X, L->Prox);
}

PNodo procurarAntecessor(int X, PNodo L) {
    PNodo Ant = NULL;
    while (L != NULL && compararElementos(L->CONECTION_DESC, X) != 0) {
        Ant = L;
        L = L->Prox;
    }
    return Ant;
}

PNodo procurarAntecessorRec(int X, PNodo L) {
    if (compararElementos(L->CONECTION_DESC, X) == 0)
        return NULL;   //  s� acontece se X estiver no in�cio de L
    if (compararElementos(L->Prox->CONECTION_DESC, X) == 0)
        return L;
    return procurarAntecessorRec(X, L->Prox);
}

PNodo inserirListaInicio(int X, PNodo L) {
    PNodo P;
    P = criarNodo(X);
    if (P == NULL)
        return L;
    P->Prox = L;
    L = P;
    return L;
}

PNodo inserirListaFim(int X, PNodo L) {
    PNodo P, PAux;
    P = criarNodo(X);
    if (P == NULL)
        return L;
    if (L == NULL)
        return P;
    PAux = L;
    while (PAux->Prox != NULL)    // marcar o elemento do fim de L
        PAux = PAux->Prox;
    PAux->Prox = P;
    return L;
}

PNodo inserirListaFimRec(int X, PNodo L) {
    PNodo P;
    if (L == NULL) {      // s� acontece se a lista inicial for vazia
        P = criarNodo(X);
        if (P != NULL)
            L = P;
        return L;
    }
    if (L->Prox == NULL) {      // fim da lista L
        P = criarNodo(X);
        if (P != NULL)
            L->Prox = P;
    } else
        inserirListaFimRec(X, L->Prox);    // o que devolve n�o interessa
    return L;    // devolve o in�cio da lista
}

// assume-se que a lista est� ordenada por ordem crescente
PNodo procurarAntecessorOrdem(int X, PNodo L) {
    PNodo Ant = NULL;
    while (L != NULL && compararElementos(L->CONECTION_DESC, X) < 0) {
        Ant = L;
        L = L->Prox;
    }
    return Ant;
}

PNodo inserirListaOrdem(int X, PNodo L) {
    PNodo P, PAnt;
    P = criarNodo(X);
    if (P == NULL)
        return L;
    if (L == NULL)
        return P;    // a lista L inicia-se em P
    PAnt = procurarAntecessorOrdem(X, L);
    if (PAnt == NULL) {   // inserir no in�cio de L
        P->Prox = L;
        return P;
    }
    P->Prox = PAnt->Prox;
    PAnt->Prox = P;
    return L;
}

// remover X da lista L, em que X est� na lista
PNodo removerLista(int X, PNodo L) {
    PNodo P, PAnt;
    PAnt = procurarAntecessor(X, L);
    if (PAnt == NULL) {   // remover elemento do in�cio de L
        P = L;
        L = L->Prox;
    } else {
        P = PAnt->Prox;
        PAnt->Prox = P->Prox; // ou (PAnt->Prox)->Prox
    }
    libertarNodo(P);
    return L;
}

// remover X da lista L, em que X est� na lista
PNodo removerListaRec(int X, PNodo L, PNodo LAux) {
    PNodo P;
    if (compararElementos(L->CONECTION_DESC, X) == 0) {   // X est� no in�cio da Lista L
        P = L;
        L = L->Prox;
        libertarNodo(P);
        return L;
    }
    if (compararElementos(LAux->Prox->CONECTION_DESC, X) == 0) {
        // X est� na lista L, mas n�o no inicio
        P = LAux->Prox;
        LAux->Prox = P->Prox;    // ou LAux->Prox->Prox;
        libertarNodo(P);
        return L;
    }
    return removerListaRec(X, L, LAux->Prox);
}



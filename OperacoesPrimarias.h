#include <stdio.h>
#include <stdlib.h>
#include "Aleatorio.h"

#define N 3
// PRECISAR TER ID DA THREAD E FILE SCRIPTED DA CONEXÃO
typedef struct {
    int NIF;
    int data[N];
    int NFatura;
    float Pagamento;
} INFO;


void mostrarElemento(INFO X) {
    printf("%d - ", X.NIF);
    for (int i = 0; i < N; i++) {
        printf("%d - ", X.data[i]);
    }
    printf("%d - ", X.NFatura);
    printf("%f\n", X.Pagamento);
}

INFO criarElemento() {
    INFO X;
    X.NIF = gerarNumeroInteiro(1780, 1790);
    for (int i = 0; i < N; i++) {
        if (i == 0) { X.data[i] = gerarNumeroInteiro(1, 30); }
        else if (i == 1) { X.data[i] = gerarNumeroInteiro(1, 12); }
        else if (i == 2) { X.data[i] = gerarNumeroInteiro(2000, 3000); }
    }
    X.NFatura = gerarNumeroInteiro(1000, 2000);
    X.Pagamento = gerarNumeroReal(10.0, 1000.0);
    return X;
}

// compara��o de 2 elementos do tipo INFO, segundo o campo NIF (chave)
//   devolve -1 se X < Y, 0 se X = Y, 1 se X > Y
int compararElementos(INFO X, INFO Y) {
    if (X.NIF > Y.NIF)
        return 1;
    if (X.NIF < Y.NIF)
        return -1;
    return 0;
}





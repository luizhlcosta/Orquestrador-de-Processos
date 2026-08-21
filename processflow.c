#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_TOKENS 64
#define MAX_LINE 1024

int tokenizar(char *linha, char *tokens[]) {
    int contador = 0;

    char *token = strtok(linha, " \t\n");

    while (token != NULL && contador < MAX_TOKENS - 1) {
        tokens[contador] = token;
        contador++;

        token = strtok(NULL, " \t\n");
    }

    tokens[contador] = NULL;

    return contador;
}

int linha_em_branco(char *linha) {
    for (int i = 0; linha[i] != '\0'; i++) {
        if (linha[i] != ' ' && linha[i] != '\t') {
            return 0;
        }
    }

    return 1;
}

int main() {
    char entrada[MAX_LINE];
    char *tokens[MAX_TOKENS];

    while (1) {
        printf("processflow> ");

        if (fgets(entrada, sizeof(entrada), stdin) == NULL) {
            break;
        }

        entrada[strcspn(entrada, "\n")] = '\0';

        if (linha_em_branco(entrada)) {
            continue;
        }

        int qtd_tokens = tokenizar(entrada, tokens);

        if (strcmp(tokens[0], "exit") == 0) {
            break;
        }

        for (int i = 0; i < qtd_tokens; i++) {
            printf("token[%d] = %s\n", i, tokens[i]);
        }
    }

    return 0;
}
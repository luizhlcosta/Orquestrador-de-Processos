#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_TOKENS 64
#define MAX_LINE 1024

#define MAX_NOME 64
#define MAX_ARGS 64
#define MAX_TAREFAS 64

typedef struct {
    char nome[MAX_NOME];
    char *argv[MAX_ARGS];
} Tarefa;

Tarefa tarefas[MAX_TAREFAS];
int total_tarefas = 0;

void cadastrar_tarefa(char *tokens[], int qtd_tokens) {
    if (qtd_tokens < 3) {
        printf("uso: task <nome> <programa> [args...]\n");
        return;
    } else if (total_tarefas >= MAX_TAREFAS) {
        printf("limite maximo de tarefas atingido\n");
        return;
    } else {
        strncpy(tarefas[total_tarefas].nome, tokens[1], MAX_NOME - 1);
        tarefas[total_tarefas].nome[MAX_NOME - 1] = '\0';

        int i = 0;

        while ((2 + i) < qtd_tokens && i < (MAX_ARGS - 1)) {
            tarefas[total_tarefas].argv[i] = strdup(tokens[2 + i]);
            i++;
        }

        tarefas[total_tarefas].argv[i] = NULL;

        printf("tarefa '%s' cadastrada\n", tarefas[total_tarefas].nome);

        total_tarefas++;
    }
}

int buscar_tarefa(char *nome) {
    for (int i = 0; i < total_tarefas; i++) {
        if (strcmp(tarefas[i].nome, nome) == 0) {
            return i;
        }
    }

    return -1;
}

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

void executar_tarefa(int indice) {
    pid_t pid = fork();

    if (pid == 0) {
        execvp(
            tarefas[indice].argv[0],
            tarefas[indice].argv
        );

        perror("erro ao executar");
        exit(1);

    } else if (pid > 0) {
        int status;

        waitpid(pid, &status, 0);

    } else {
        perror("erro no fork");
    }
}

void executar_sequential(char *nomes_tarefas[], int qtd) {
    for (int i = 0; i < qtd; i++) {
        int indice = buscar_tarefa(nomes_tarefas[i]);
        if (indice == -1) {
            printf("tarefa nao encontrada: %s\n", nomes_tarefas[i]);
            continue;
        }
        executar_tarefa(indice);
    }
}

void executar_paralelo(char *nomes_tarefas[], int qtd) {
    pid_t pids[qtd];
    int qtd_pids = 0;

    for (int i = 0; i < qtd; i++) {
        int indice = buscar_tarefa(nomes_tarefas[i]);
        if (indice == -1) {
            printf("tarefa nao encontrada: %s\n", nomes_tarefas[i]);
            continue;
        }
        pid_t pid = fork();

        if (pid == 0) {
            execvp(tarefas[indice].argv[0], tarefas[indice].argv);
            perror("erro ao executar");
            exit(1);
        } else if (pid > 0) {
            pids[qtd_pids] = pid;
            qtd_pids++;
        } else {
            perror("erro no fork");
        }
    }

    for (int i = 0; i < qtd_pids; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }
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

        if (qtd_tokens == 0) {
            continue;
        }

        if (strcmp(tokens[0], "task") == 0) {

            cadastrar_tarefa(tokens, qtd_tokens);

        } else if (strcmp(tokens[0], "run") == 0) {

            if (qtd_tokens < 3) {
                printf("uso: run <sequential|parallel> <tarefas...>\n");
                continue;
            }

            int qtd_nomes = qtd_tokens - 2;

            if (strcmp(tokens[1], "sequential") == 0) {
                executar_sequential(&tokens[2], qtd_nomes);
            } else if (strcmp(tokens[1], "parallel") == 0) {
                executar_paralelo(&tokens[2], qtd_nomes);
            } else {
                printf("modo invalido: use sequential ou parallel\n");
            }

        } else if (strcmp(tokens[0], "exit") == 0) {

            break;

        } else {

            for (int i = 0; i < qtd_tokens; i++) {
                printf("token[%d] = %s\n", i, tokens[i]);
            }
        }
    }

    return 0;
}
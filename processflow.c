#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_TOKENS 64
#define MAX_LINE 1024

#define MAX_NOME 64
#define MAX_ARGS 64
#define MAX_TAREFAS 64

#define MAX_JOBS 64

typedef struct {
    char nome[MAX_NOME];
    char *argv[MAX_ARGS];
    char *input_file;
    char *output_file;
    int append;
} Tarefa;

Tarefa tarefas[MAX_TAREFAS];
int total_tarefas = 0;

typedef struct { 
    int job_id;
    pid_t pid;
    char nome_tarefa[MAX_NOME];
    int status;
} Job;

Job jobs[MAX_JOBS];
int total_jobs = 0;


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

        tarefas[total_tarefas].input_file = NULL;
        tarefas[total_tarefas].output_file = NULL;
        tarefas[total_tarefas].append = 0;

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

void cmd_input(char *tokens[], int qtd_tokens) {
    if (qtd_tokens < 3) {
        printf("uso: input <tarefa> <arquivo>\n");
        return;
    }
    int indice = buscar_tarefa(tokens[1]);
    if (indice == -1) {
        printf("tarefa nao encontrada: %s\n", tokens[1]);
        return;
    }
    tarefas[indice].input_file = strdup(tokens[2]);
}

void cmd_output(char *tokens[], int qtd_tokens) {
    if (qtd_tokens < 3) {
        printf("uso: output <tarefa> <arquivo>\n");
        return;
    }
    int indice = buscar_tarefa(tokens[1]);
    if (indice == -1) {
        printf("tarefa nao encontrada: %s\n", tokens[1]);
        return;
    }
    tarefas[indice].output_file = strdup(tokens[2]);
    tarefas[indice].append = 0;
}

void cmd_append(char *tokens[], int qtd_tokens) {
    if (qtd_tokens < 3) {
        printf("uso: append <tarefa> <arquivo>\n");
        return;
    }
    int indice = buscar_tarefa(tokens[1]);
    if (indice == -1) {
        printf("tarefa nao encontrada: %s\n", tokens[1]);
        return;
    }
    tarefas[indice].output_file = strdup(tokens[2]);
    tarefas[indice].append = 1;
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
        if (tarefas[indice].input_file != NULL) {
            int fd = open(tarefas[indice].input_file, O_RDONLY);
            if (fd < 0) {
                perror("erro ao abrir arquivo de entrada");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (tarefas[indice].output_file != NULL) {
            int flags;
            if (tarefas[indice].append == 1) {
                flags = O_WRONLY | O_CREAT | O_APPEND;
            } else {
                flags = O_WRONLY | O_CREAT | O_TRUNC;
            }

            int fd = open(tarefas[indice].output_file, flags, 0644);
            if (fd < 0) {
                perror("erro ao abrir arquivo de saida");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

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

void executar_pipe(char *nomes_tarefas[], int qtd) {
    int pipes[qtd - 1][2];

    for (int i = 0; i < qtd - 1; i++) {
        pipe(pipes[i]);
    }
    pid_t pids[qtd];

    for (int i = 0; i < qtd; i++) {
        int indice = buscar_tarefa(nomes_tarefas[i]);
        if (indice == -1) {
            printf("tarefa nao encontrada: %s\n", nomes_tarefas[i]);
            continue;
        }
        pid_t pid = fork();

        if (pid == 0) {
            if (i == 0) {
                dup2(pipes[i][1], STDOUT_FILENO);
            } else if (i == qtd - 1) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            } else {
                dup2(pipes[i-1][0], STDIN_FILENO);
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            for (int j = 0; j < qtd - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(tarefas[indice].argv[0], tarefas[indice].argv);
            perror("erro ao executar");
            exit(1);
        } else if (pid > 0) {
            pids[i] = pid;
        } else {
            perror("erro no fork");
        }
    }

    for (int i = 0; i < qtd - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < qtd; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }
}

void cmd_workdir(char *tokens[], int qtd_tokens) {
    if (qtd_tokens < 2) {
        printf("uso: workdir <diretorio>");
        return;
    }

    if (chdir(tokens[1]) != 0) {
        perror("erro ao mudar diretorio");
        return;
    }

    printf("diretorio de trabalho alterado para: %s\n", tokens[1]);
}

void cmd_start(char *tokens[], int qtd_tokens) {

    if (qtd_tokens < 2) {
        printf("uso: start <tarefa>\n");
        return;
    }

    int indice = buscar_tarefa(tokens[1]);

    if (indice == -1) {
        printf("tarefa nao encontrada: %s\n", tokens[1]);
        return;
    }

    pid_t pid = fork();

    if (pid == 0) {
        
        if (tarefas[indice].input_file != NULL) {
            int fd = open(tarefas[indice].input_file, O_RDONLY);
            if (fd < 0) {
                perror("erro ao abrir arquivo de entrada");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (tarefas[indice].output_file != NULL) {
            int flags;
            if (tarefas[indice].append == 1) {
                flags = O_WRONLY | O_CREAT | O_APPEND;
            } else {
                flags = O_WRONLY | O_CREAT | O_TRUNC;
            }
            int fd = open(tarefas[indice].output_file, flags, 0644);
            if (fd < 0) {
                perror("erro ao abrir arquivo de saida");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        execvp(tarefas[indice].argv[0], tarefas[indice].argv);
        perror("erro ao executar");
        exit(1);

    } else if (pid > 0) {

        if (total_jobs >= MAX_JOBS) {
            printf("limite maximo de jobs atingido\n");
            return;
        }

        jobs[total_jobs].job_id = total_jobs + 1;
        jobs[total_jobs].pid = pid;
        strncpy(jobs[total_jobs].nome_tarefa, tokens[1], MAX_NOME - 1);
        jobs[total_jobs].nome_tarefa[MAX_NOME - 1] = '\0';
        jobs[total_jobs].status = 0; // RUNNING

        printf("[%d] %d\n", jobs[total_jobs].job_id, pid);

        total_jobs++;

    } else {
        perror("erro no fork");
    }
}

void cmd_jobs() {
    for (int i = 0; i < total_jobs; i++) {
        if (jobs[i].status == 0) {
            int status;
            int resultado = waitpid(jobs[i].pid, &status, WNOHANG);
            if (resultado > 0) {
                jobs[i].status = 1; // DONE
            }
        }

        const char *texto_status = (jobs[i].status == 0) ? "Running" : "Done";
        printf("[%d] %d %s %s\n", jobs[i].job_id, jobs[i].pid, jobs[i].nome_tarefa, texto_status);
    }
}

void cmd_wait(char *tokens[], int qtd_tokens) {
    if (qtd_tokens < 2) {
        printf("uso: wait <jobId>\n");
        return;
    }

    int job_id = atoi(tokens[1]);

    for (int i = 0; i < total_jobs; i++) {
        if (jobs[i].job_id == job_id) {
            if (jobs[i].status == 0) {
                int status;
                waitpid(jobs[i].pid, &status, 0);
                jobs[i].status = 1;
            }
            printf("job %d finalizado\n", job_id);
            return;
        }
    }

    printf("job nao encontrado: %d\n", job_id);
}


int main(int argc, char *argv[]) {
    char entrada[MAX_LINE];
    char *tokens[MAX_TOKENS];

    FILE *entrada_stream;

    if (argc == 1) {
        entrada_stream = stdin;
    } else if (argc == 2) {
        entrada_stream = fopen(argv[1], "r");
        if (entrada_stream == NULL) {
            printf("erro: nao foi possivel abrir o arquivo workflow: %s\n", argv[1]);
            return 1;
        }
    } else {
        printf("uso: %s [workflowFile]\n", argv[0]);
        return 1;
    }

    while (1) {
        
        if (entrada_stream == stdin) {
            printf("processflow> ");
        } 
        
        if (fgets(entrada, sizeof(entrada), entrada_stream) == NULL) {
            break;
        }

        entrada[strcspn(entrada, "\n")] = '\0';

        if (entrada_stream != stdin) {
            printf("%s\n", entrada);
        }

        if (linha_em_branco(entrada)) {
            continue;
        }

        int qtd_tokens = tokenizar(entrada, tokens);

        if (qtd_tokens == 0) {
            continue;
        }

        if (strcmp(tokens[0], "task") == 0) {

            cadastrar_tarefa(tokens, qtd_tokens);

        } else if (strcmp(tokens[0], "input") == 0) {

            cmd_input(tokens, qtd_tokens);

        } else if (strcmp(tokens[0], "output") == 0) {

            cmd_output(tokens, qtd_tokens);

        } else if (strcmp(tokens[0], "append") == 0) {

            cmd_append(tokens, qtd_tokens);

        } else if (strcmp(tokens[0], "workdir") == 0) {

            cmd_workdir(tokens, qtd_tokens);

        } else if (strcmp(tokens[0], "start") == 0) {

            cmd_start(tokens, qtd_tokens);

        } else if (strcmp(tokens[0], "jobs") == 0) {

            cmd_jobs();

        } else if (strcmp(tokens[0], "wait") == 0) {

            cmd_wait(tokens, qtd_tokens);

        } else if (strcmp(tokens[0], "run") == 0) {

            if (qtd_tokens < 2) {
                printf("uso: run <sequential|parallel|pipe> <tarefas...>\n");
                continue;
            }

            int qtd_nomes = qtd_tokens - 2;

            if (strcmp(tokens[1], "sequential") == 0) {
                executar_sequential(&tokens[2], qtd_nomes);
            } else if (strcmp(tokens[1], "parallel") == 0) {
                executar_paralelo(&tokens[2], qtd_nomes);
            } else if (strcmp(tokens[1], "pipe") == 0) {
                executar_pipe(&tokens[2], qtd_nomes);
            } else if (qtd_tokens == 2) {
                // run <nome> direto, sem modo
                int indice = buscar_tarefa(tokens[1]);
                if (indice == -1) {
                    printf("tarefa nao encontrada: %s\n", tokens[1]);
                } else {
                    executar_tarefa(indice);
                }
            } else {
                printf("uso: run <sequential|parallel|pipe> <tarefas...>\n");
            }

        } else if (strcmp(tokens[0], "exit") == 0) {

            break;

        } else {

            for (int i = 0; i < qtd_tokens; i++) {
                printf("token[%d] = %s\n", i, tokens[i]);
            }
        }
    }

    if (entrada_stream != stdin) {
        fclose(entrada_stream);
    }

    return 0;
}
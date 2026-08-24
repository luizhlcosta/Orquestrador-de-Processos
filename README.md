# ProcessFlow

## Descrição
ProcessFlow é um orquestrador de processos no estilo shell, desenvolvido em C,
que permite cadastrar e executar tarefas (programas do sistema) por meio de
processos filhos, usando fork(), exec(), wait()/waitpid(), pipe() e dup2().

O programa suporta dois modos de operação:
- **Interativo**: apresenta o prompt `processflow>` para o usuário digitar comandos.
- **Workflow**: recebe um arquivo `.pf` como argumento, contendo uma lista de
  comandos a serem executados em sequência, sem exibir o prompt.

Funcionalidades suportadas: cadastro e execução de tarefas, execução sequencial
e paralela, pipes entre tarefas, redirecionamento de entrada/saída (input,
output, append), alteração de diretório de trabalho (workdir), execução em
background (start/jobs/wait).

## Status de implementação

- [x] Execução simples (task/run) + exit
- [x] Execução sequencial (run sequential)
- [x] Execução paralela (run parallel)
- [x] Pipe entre tarefas (run pipe)
- [x] Redirecionamento de entrada (input)
- [x] Redirecionamento de saída (output)
- [x] Redirecionamento de saída em modo append (append)
- [x] Diretório de trabalho (workdir)
- [x] Execução em background (start)
- [x] Listagem de jobs (jobs)
- [x] Espera de job específico (wait)
- [x] Modo workflow (.pf) com impressão de linha antes de processar
- [x] Tratamento de erros (argumentos inválidos, tarefa/programa inexistente, etc.)

## Ambiente de desenvolvimento
Este projeto foi desenvolvido e testado em WSL (Windows Subsystem for Linux),
distribuição Ubuntu, rodando sobre Windows.

## Como compilar
No diretório do projeto, execute:

    make

Isso gera o executável `processflow` na raiz do projeto.

## Como executar
Modo interativo (sem argumentos):

    ./processflow

Modo workflow (com arquivo .pf):

    ./processflow nome_do_arquivo.pf 

## Como limpar arquivos compilados

    make clean

Remove o executável e arquivos objeto gerados pela compilação.
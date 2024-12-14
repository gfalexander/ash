#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// Tamanho inicial do buffer de leitura
#define ASH_RL_BUFSIZE 1024
// Lê uma linha de comando
char *ash_read_line(void) {
    int bufsize = ASH_RL_BUFSIZE;
    // Posição atual do buffer
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "ash: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();

        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        if (position >= bufsize) {
            bufsize += ASH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "ash: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define ASH_TOK_BUFSIZE 64
#define ASH_TOK_DELIM " \t\r\n\a"
// Divide a linha de comando em tokens
char **ash_split_line(char *line) {
    int bufsize = ASH_TOK_BUFSIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "ash: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, ASH_TOK_DELIM);

    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += ASH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "ash: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, ASH_TOK_DELIM);
    }

    tokens[position] = NULL;
    return tokens;
}

int ash_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("ash");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("ash");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int ash_cd(char **args);
int ash_help(char **args);
int ash_exit(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &ash_cd,
    &ash_help,
    &ash_exit
};

int ash_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int ash_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "ash: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("ash");
        }
    }
    return 1;
}

int ash_help(char **args) {
    int i;
    printf("Ash - A Simple Shell\n");
    printf("The following are built in:\n");

    for (i = 0; i < ash_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    return 1;
}

int ash_exit(char **args) {
    return 0;
}

int ash_execute(char **args) {
    int i;

    if (args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < ash_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return ash_launch(args);
}

// Inicializa o loop do shell
void ash_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        
        line = ash_read_line();
        args = ash_split_line(line);
        status = ash_execute(args);
    } while (status);
}

int main(int argc, char **argv) {
    ash_loop();

    return EXIT_SUCCESS;
}
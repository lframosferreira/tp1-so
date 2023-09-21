#include <ctype.h>
#include <dirent.h>
#include <pthread.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 256
#define FILE_PATH_SIZE 256
#define STAT_COUNTER_MAX 20

// Constantes para organização do display do top
#define PID_DISPLAY_WIDTH 10
#define USER_DISPLAY_WIDTH 10
#define PROCNAME DISPLAY WIDTH 10
#define STATE_DISPLAY_WIDTH 10

volatile bool keep_running = true;

static char *token;

void get_name(char *file_path) {
  FILE *file_handler = fopen(file_path, "r");
  if (file_handler == NULL) {
    perror("Erro ao abrir arquivo: ");
    exit(1);
  }

  char buffer[BUFFER_SIZE];
  fgets(buffer, BUFFER_SIZE, file_handler);

  token = strtok(buffer, " "); // PID
  token = strtok(NULL, " ");   // PROCNAME
  fputs(token, stdout);
  token = strtok(NULL, " "); // STATE
  fclose(file_handler);
}

void display_top_header(void) { printf("|%-10s|", "PID"); }

void *print_processes(void *dir) {
  DIR *directory = (DIR *)dir;

  struct dirent *entry;

  while (keep_running) {
    int stat_counter = 0;

    printf("\nPID     | User    | PROCNAME         | Estado    |\n");
    printf("--------|---------|------------------|-----------|\n");

    while ((entry = readdir(directory))) {
      int pid;

      if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {
        pid = atoi(entry->d_name);

        struct stat stat_buffer;
        char file_path[FILE_PATH_SIZE];
        snprintf(file_path, sizeof(file_path), "/proc/%d/stat", pid);

        if (stat(file_path, &stat_buffer) == 0) {
          struct passwd *user_data = getpwuid(stat_buffer.st_uid);

          if (user_data != NULL) {
            printf("%d %s ", pid, user_data->pw_name);
            get_name(file_path);
            printf(" %c\n", token[0]);

            stat_counter++;

            if (stat_counter >= STAT_COUNTER_MAX) {
              break;
            }
          }
        }
      }
    }
    rewinddir(directory);
    sleep(1);
  }
  return NULL;
}

void *read_input(void *unused) {
  // HACK O compilador dá um warning se a função não tiver argumentos (tem que
  // passar pelo menos um void). Se passar void, não tem como fazer casting
  // (pelo menos eu não consegui) na hora de criar a thread, porque a função
  // para criar a thread espera um void *. Então a gente cria um ponteiro pra
  // void, e "usa" ele pro compilador não reclamar. Provalmente tem um jeito
  // mais elegante de resolver isso

  (void)unused;
  int process;
  int signal;
  while (scanf("%d %d", &process, &signal) != EOF) {
    printf("Processo %d\n", process);
    printf("Sinal %d\n", signal);
    // TODO(luisal): faz o processamento do sinal
  }
  keep_running = false;

  return NULL;
}

int main(void) {
  DIR *directory = opendir("/proc");

  if (directory == NULL) {
    perror("Erro ao abrir o diretório /proc: ");
    exit(1);
  }

  pthread_t *thread_handles = malloc(2 * sizeof(pthread_t));
  pthread_create(&thread_handles[0], NULL, print_processes, (void *)directory);
  pthread_create(&thread_handles[1], NULL, read_input, NULL);

  pthread_join(thread_handles[0], NULL);
  pthread_join(thread_handles[1], NULL);

  free(thread_handles);

  closedir(directory);
}

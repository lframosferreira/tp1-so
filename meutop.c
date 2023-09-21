#include <ctype.h>
#include <dirent.h>
#include <pthread.h>
#include <pwd.h>
#include <signal.h>
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
  token[strlen(token) - 1] = '\0';
  printf("%-31s", token + 1);
  token = strtok(NULL, " "); // STATE
  fclose(file_handler);
}

void *print_processes(void *dir) {
  DIR *directory = (DIR *)dir;

  struct dirent *entry;

  while (keep_running) {
    int stat_counter = 0;

    printf("\nPID    | User    | PROCNAME                      | Estado |\n");
    printf("-------|---------|-------------------------------|--------|\n");

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
            printf("%-8d", pid);
            printf("%-10s ", user_data->pw_name);
            get_name(file_path);
            printf("%-10c\n", token[0]);

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

void *read_input(void *) {
  int pid;
  int signal;
  while (scanf("%d %d", &pid, &signal) != EOF) {
    printf("Processo %d\n", pid);
    printf("Sinal %d\n", signal);
    if (kill(pid, signal) == -1) {
      fprintf(stdout, "Erro ao processar sinal\n");
      exit(EXIT_FAILURE);
    }
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

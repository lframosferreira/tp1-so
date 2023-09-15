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

volatile bool keepRunning = true;

static char *token;

void get_name(char *filePath) {
  FILE *fileHandler = fopen(filePath, "r");
  if (fileHandler == NULL) {
    perror("Erro ao abrir arquivo: ");
    exit(1);
  }

  char buffer[BUFFER_SIZE];
  fgets(buffer, BUFFER_SIZE, fileHandler);

  token = strtok(buffer, " "); // PID
  token = strtok(NULL, " ");   // PROCNAME
  fputs(token, stdout);
  token = strtok(NULL, " "); // STATE
  fclose(fileHandler);
}

void *print_processes(void *dir) {
  DIR *directory = (DIR *)dir;

  struct dirent *entry;

  while (keepRunning) {
    int statCounter = 0;

    printf("\nPID User PROCNAME Estado\n");

    while ((entry = readdir(directory))) {
      int pid;

      if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {
        pid = atoi(entry->d_name);

        struct stat statBuffer;
        char filePath[FILE_PATH_SIZE];
        snprintf(filePath, sizeof(filePath), "/proc/%d/stat", pid);

        if (stat(filePath, &statBuffer) == 0) {
          struct passwd *userData = getpwuid(statBuffer.st_uid);

          if (userData != NULL) {
            printf("%d %s ", pid, userData->pw_name);
            get_name(filePath);
            printf(" %c\n", token[0]);

            statCounter++;

            if (statCounter >= STAT_COUNTER_MAX) {
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
  // passar void) Se passar void, não tem como fazer casting (pelo menos eu não
  // consegui) na hora de criar a thread Então a gente cria um ponteiro pra
  // void, e "usa" ele pro compilador não reclamar Provalmente tem um jeito mais
  // elegante de resolver isso

  (void)unused;

  int process;
  int signal;
  while (scanf("%d %d", &process, &signal) != EOF) {
    printf("Processo %d\n", process);
    printf("Sinal %d\n", signal);
    // TODO faz o processamento do sinal
  }
  keepRunning = false;

  return NULL;
}

int main(void) {
  DIR *directory;

  directory = opendir("/proc");

  if (directory == NULL) {
    perror("Erro ao abrir o diretório /proc: ");
    exit(1);
  }

  pthread_t *threadHandles = malloc(2 * sizeof(pthread_t));
  pthread_create(&threadHandles[0], NULL, print_processes, (void *)directory);
  pthread_create(&threadHandles[1], NULL, read_input, NULL);

  pthread_join(threadHandles[0], NULL);
  pthread_join(threadHandles[1], NULL);

  free(threadHandles);

  closedir(directory);
}

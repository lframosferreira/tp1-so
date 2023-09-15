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

typedef struct proc_info {
  int pid;
  char *name;
  char state;
  char *user;
} proc_info;

void *print_processes(void *dir) {
  // TODO quebrar essa função em bloquinhos porque a complexidade ciclomática
  // dela está muito grande
  DIR *directory = (DIR *)dir;
  proc_info processes[STAT_COUNTER_MAX];

  struct dirent *entry;
  while (keepRunning) {
    int statCounter = 0;
    printf("\nPID User PROCNAME Estado\n");
    while ((entry = readdir(directory))) {
      char *name = NULL;
      int pid;
      if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {
        pid = atoi(entry->d_name);

        struct stat statBuffer;
        char filePath[FILE_PATH_SIZE];
        snprintf(filePath, sizeof(filePath), "/proc/%d/stat", pid);

        FILE *fileHandler = fopen(filePath, "r");
        if (fileHandler == NULL) {
          perror("Erro ao abrir arquivo: ");
          exit(1);
        }

        char lineBuffer[BUFFER_SIZE];
        fgets(lineBuffer, BUFFER_SIZE, fileHandler);
        char *token = strtok(lineBuffer, " "); // PID
        token = strtok(NULL, " ");
        name = token;
        token = strtok(NULL, " "); // State

        if (stat(filePath, &statBuffer) == 0) {
          struct passwd *userData = getpwuid(statBuffer.st_uid);

          if (userData != NULL) {

            proc_info *proc = &processes[statCounter];
            proc->pid = pid;
            proc->user = userData->pw_name;
            proc->name = name;
            proc->state = token[0];

            printf("%d %s %s %c\n", proc->pid, proc->user, proc->name,
                   proc->state);

            statCounter++;

            if (statCounter >= STAT_COUNTER_MAX) {
              break;
            }
          }
        }

        fclose(fileHandler);
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

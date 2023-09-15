#include <ctype.h>
#include <dirent.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 256
#define FILE_PATH_SIZE 256
#define STAT_COUNTER_MAX 20

typedef struct proc_info {
  int pid;
  char *name;
  char state;
  char *user;
} proc_info;

int main(void) {
  struct dirent *entry;
  DIR *directory;

  directory = opendir("/proc");

  if (directory == NULL) {
    perror("Erro ao abrir o diretório /proc: ");
    exit(1);
  }

  printf("\nPID User PROCNAME Estado\n");
  int statCounter = 0;
  proc_info processes[STAT_COUNTER_MAX];

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

          printf("%d  %s  %s  %c\n", proc->pid, proc->user, proc->name,
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

  closedir(directory);
}

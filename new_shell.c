#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <curses.h>
#include <signal.h>
#include <unistd.h>

int add_line(int x, char **buffer, int size);

void sigcatch(int sig){
  printf("exit\n");
  exit(0);
}

int my_mvaddstr(int y, int x, char *str){
  char *p = str;
  for(; x < 0; ++x, ++p)
    if(*p == '\0') return ERR;
  for(; *p != '\0'; ++p, ++x)
    if(mvaddch(y, x, *p) == ERR) return ERR;
  return OK;
}

int main(int argc, char *argv[]){
  int x;
  int h = 0;
  int i;
  int size = 0;
  char **buffer;
  char *shell, *eob;
  pid_t cpid;
  int pipefd[2];
  FILE *fp;

  shell = getenv("SHELL");
  if(shell == NULL){
    fprintf(stderr, "cannot get login shell!\n");
    exit(EXIT_FAILURE);
  }

  if(pipe(pipefd) == -1){
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  cpid = fork();
  if(cpid == -1){
    close(pipefd[0]);
    close(pipefd[1]);
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if(cpid == 0){
    close(pipefd[0]);
    dup2(pipefd[1], 1);
    close(pipefd[1]);

    execl(shell, shell, NULL);
    exit(EXIT_FAILURE);
  }else{
    close(pipefd[1]);
    dup2(pipefd[0], 0);
    close(pipefd[0]);

    if(SIG_ERR == signal(SIGCHLD, sigcatch)){
      perror("signal");
      exit(1);
    }
    fp = fopen("test.log", "a");
    
    initscr();
    noecho();
    leaveok(stdscr, TRUE);
    scrollok(stdscr, FALSE);
    buffer = malloc(sizeof(char *) * (LINES - 1));
    for(i = 0; i < LINES - 1; i++){
      *(buffer + i) = malloc(sizeof(char) * 4096);
    }

    while(1){
      for(size = 0; size < LINES-1; size++){
        fprintf(fp, "%d\n", size);
        eob = fgets(*(buffer+size), 4096, stdin);
        if(eob == NULL) break;
        fprintf(fp, "%s\n", *(buffer+size));
      }
      for(x = COLS - 1; x > -COLS; --x){
        if(add_line(x, buffer, size) == ERR) break;
        refresh();
        usleep(20000);
        fprintf(fp, "x=%d\n", x);
      }
      mvcur(0, COLS - 1, LINES - 1, 0);
    }
    kill(cpid, SIGKILL);
  }
  fclose(fp);

  endwin();
  for(i = 0; i < 10; i++){
    free(*(buffer+i));
  }
  free(buffer);
  printf("OK\n");
  return 0;
}

int add_line(int x, char **buffer, int size){
  int i;
  for(i = 0; i < size; i++){
    my_mvaddstr(i, x, *(buffer+i));
  }
  return OK;
}

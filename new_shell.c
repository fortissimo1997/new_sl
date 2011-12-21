#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <curses.h>
#include <signal.h>
#include <unistd.h>

int add_line(int x, int h, char *buffer);

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
  char *buffer;
  char *shell, *eob;
  pid_t cpid;
  int pipefd[2];


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
    
    if((buffer = malloc(4096 * sizeof(char))) == NULL){
      fprintf(stderr, "cannot allocate buffer!\n");
      exit(EXIT_FAILURE);
    }

    initscr();
    noecho();
    leaveok(stdscr, TRUE);
    scrollok(stdscr, FALSE);
    while(1){
      eob = fgets(buffer, 4096, stdin);
      eob = strchr(buffer, '\n');
      if(eob != NULL)
        *eob = '\0';
      for(x = COLS - 1; ; --x){
        if(add_line(x, h, buffer) == ERR) break;
        refresh();
        usleep(20000);
      }
      mvcur(0, COLS - 1, LINES - 1, 0);
      h++;
    
//    fprintf(stdout, "%s\n", buffer);
    }
  }

  endwin();
  free(buffer);
  return 0;
}

int add_line(int x, int h, char *buffer){
  int y = LINES - (COLS / 6) + h;
  my_mvaddstr(y, x, buffer);
  return OK;
}

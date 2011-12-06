#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <curses.h>
#include <signal.h>
#include <unistd.h>

int my_mvaddstr(int y, int x, char *str){
  for(; x < 0; ++x, ++str)
    if(*str == '\0') return ERR;
  for(; *str != '\0'; ++str, ++x)
    if(mvaddch(y, x, *str) == ERR) return ERR;
  return OK;
}

int main(int argc, char *argv[]){
  char buffer[4096];
  char *shell, *eob;
  FILE *fp;
  shell = getenv("SHELL");
  if(shell == NULL){
    fprintf(stderr, "cannot get login shell!\n");
    exit(-1);
  }
  if((fp = popen(shell, "r")) == NULL){
    fprintf(stderr, "cannot open shell!\n");
    exit(-1);
  }
  while(1){
    eob = fgets(buffer, 4096, fp);
    if(feof(fp)){
      break;
    }
    eob = strchr(buffer, '\n');
    if(eob != NULL)
      *eob = '\0';
    initscr();
    signal(SIGINT, SIG_IGN);
    noecho();
    leaveok(stdscr, TRUE);
    scrollok(stdscr, FALSE);
    for(int x = COLS - 1; ; --x){
      if(add_line(x) == ERR) break;
      refresh();
      usleep(20000);
    }
    mvcur(0, COLS - 1, LINES - 1, 0);
    endwin();
    
//    fprintf(stdout, "%s\n", buffer);
  }

  pclose(fp);
  return 0;
}

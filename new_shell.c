#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <curses.h>
#include <signal.h>
#include <unistd.h>

int add_line(int x, int h, char *buffer);

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
  FILE *fp;
  if((buffer = malloc(4096 * sizeof(char))) == NULL){
    fprintf(stderr, "cannot allocate buffer!\n");
    exit(-1);
  }
  shell = getenv("SHELL");
  if(shell == NULL){
    fprintf(stderr, "cannot get login shell!\n");
    exit(-1);
  }
  if((fp = popen(shell, "r")) == NULL){
    fprintf(stderr, "cannot open shell!\n");
    exit(-1);
  }
  initscr();
  noecho();
  leaveok(stdscr, TRUE);
  scrollok(stdscr, FALSE);
  while(1){
    eob = fgets(buffer, 4096, fp);
    if(feof(fp)) break;
    /*
    eob = strchr(buffer, '\n');
    if(eob != NULL)
      *eob = '\0';
      */
    for(x = COLS - 1; ; --x){
      if(add_line(x, h, buffer) == ERR) break;
      refresh();
      usleep(20000);
    }
    mvcur(0, COLS - 1, LINES - 1, 0);
    h++;
    
//    fprintf(stdout, "%s\n", buffer);
  }

  endwin();
  pclose(fp);
  free(buffer);
  return 0;
}

int add_line(int x, int h, char *buffer){
  int y = LINES - (COLS / 6) + h;
  my_mvaddstr(y, x, buffer);
  return OK;
}

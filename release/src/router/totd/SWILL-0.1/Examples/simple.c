#include "swill.h"
#include <stdio.h>
#include <unistd.h>

void foo() {
  int i;
  printf("Hi, I'm foo.\n");
  for (i = 0; i < 10; i++) {
    printf("%d\n",i);
  }
}

/* A function that prints some HTML. Ok, this sucks in C */
void print_form(FILE *f) {
  int i;
  fprintf(f,"<HTML><form action=\"http://localhost:8080/blah.html\" method=POST>\n");
  fprintf(f,"Your name : <input type=text name=name width=30></input><br>\n");
  fprintf(f,"Submit : <input type=submit></input>\n");
  fprintf(f,"</form>");
  fprintf(f,"</html>\n");
}

/* A function that gets a form variable */

void print_name(FILE *f) {
  char *name;
  if (!swill_getargs("s(name)",&name)) {
    fprintf(f,"Hey, go enter your name.\n");
    return;
  }
  fprintf(f,"Your name is %s\n", name);
}

int main() {
  printf("Hello World!\n");
  if (swill_init(8080)) {
    printf("SWILL listening on port 8080\n");
  } else {
    printf("Couldn't initialize the server.\n");
    exit(1);
  }

  swill_title("SWILL Example");

  swill_handle("stdout:foo.txt", foo, 0);
  swill_handle("form.html", print_form,0);
  swill_handle("blah.html", print_name,0);
  swill_file("README.txt","../README");
  swill_log(stdout);

  /* Serve files out of a directory */
  swill_directory("../Doc/");
  {
    int i = 0;
    while (1) {
      swill_serve();
      i++;
    }
  }
}




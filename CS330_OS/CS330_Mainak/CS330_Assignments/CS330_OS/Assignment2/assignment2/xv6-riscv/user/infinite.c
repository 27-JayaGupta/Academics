#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
//   int i;

//   if (argc != 2) {
//      fprintf(2, "syntax: sleep n\nAborting...\n");
//      exit(0);
//   }

//   i = atoi(argv[1]);
  // sleep(i)
  while(1){
    printf("inside loop\n");
    for(int j=0;j<10000000;j++){}
  }

  printf("INFINITE\n");
  
  exit(0);
}

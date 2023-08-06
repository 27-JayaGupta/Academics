#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char* argv[]) 
{   
    int x;
    x = uptime();
    if( x <= 0 ) {
        fprintf(2, "uptime: error\n");
        exit(1);
    }

    fprintf(1, "%d clock ticks.\n", x);
    exit(0);
}


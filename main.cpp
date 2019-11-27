#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

#include "rayqueue.h"
#include "render.h"
#include "schedule.h"

int main(int argc, char **argv)
{
    struct Communicator comm;
    int slave_num = comm.size - 1;
    if(comm.rank == comm.size - 1) {
        struct Master master(&comm); 
        master.run();
    } else {
        struct Slave slave(&comm);
        slave.run();
    }
    return 0; 
}
    
  

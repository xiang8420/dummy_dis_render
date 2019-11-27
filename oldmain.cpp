#include "schedule.h"
#include "comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#define QUEUE_LIMIT 400
// Phase 1: Fixed increment known to all at runtime
#define QUEUE_INCREMENT 10
#define WORK_TIME 0
#define WORK_UNCERTAINTY 10000
#define DATA_SIZE 1048608

struct SendPath {
    int dst, src;
    SendPath(int src, int dst): dst(dst), src(src){}
};

int main(int argc, char **argv)
{
    printf("start\n");
    // MPI stuff
    int mpi_rank, mpi_size;
    int ierr;
    MPI_Status  status;

    // Queue Window stuff
    int win_size = 0;
    int disp_unit = sizeof(int);
    int *win_mem;
    MPI_Win win;
  
    // Data window stuff
    int data_win_size = 0;
    int *data_win_mem;
    MPI_Win data_win;

    Renderer fake;
    fake.render();
    MPI_Init_thread(NULL, NULL, MPI_THREAD_SERIALIZED, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  
    int rept_size = mpi_size + 1;
//   Set up the shared memory
//   send a fetch order ? 
    if ( mpi_rank == 0 ) {
        struct Table table = Table(mpi_size);
        int iter = 0;
        int send_path[2];
        int *end = new int[mpi_size];
        while(1 /*!table.empty() || iter == 0*/) {
            MPI_Recv(table.tmp, rept_size, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status); 
            for(int i = 0; i < rept_size; i++){
                printf("%d ", table.tmp[i]);
            }
            if(table.fakemsg()) 
                break;
            printf("master recv\n");
            table.update();
            table.get_send_path(&send_path[0]);
            MPI_Send(send_path, 2, MPI_INT, send_path[0], 1, MPI_COMM_WORLD);
            printf("master send\n");
            iter ++;
        }
    } else {
        srand( ( mpi_rank + 2 ) * (unsigned) time (NULL) );
        int *local_data_array;
        int size = DATA_SIZE;
        int *rays = new int[size];
        int *num_rays = new int[rept_size];
        int iter = 0;
        for(int i = 0; i < rept_size; i++){
            num_rays[i] = 0;
        } 
       
        //mpi win
//        int *ray_queue; MPI_Win win;
//        MPI_Win_allocate(size * sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &ray_queue, &win);

        // set a recv thread waiting for master message 
        //1. while has rays
        //2. trace rays and cluster
        //
        //3. put rays in share memory
        //4. send message to master 
        //5. if recv thread get message  
        //      if message is empty keep work if work over->done 
        //      else get rays data from dst node
        //6. goto 1 
        while (iter < 3) {
            
            for( int i = 0; i < size; i++ ){
                rays[i] = rand(); 
                printf(" %d %d %d \n", i, rays[i], rays[i] % mpi_size);
                num_rays[rays[i] % mpi_size]++;
                
            }
            num_rays[mpi_size] = mpi_rank;
            MPI_Send(num_rays, rept_size, MPI_INT, 0, 1, MPI_COMM_WORLD);
            printf("slave send over\n"); 
            int sendpath[2];
            //if empty send 0 a empty message 
            MPI_Recv(sendpath, 2, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
            printf("slave recv over\n");
            
             
            iter++;
        }    
        num_rays[0] = -1;
        MPI_Send(num_rays, rept_size, MPI_INT, 0, 1, MPI_COMM_WORLD);
        

        MPI_Win_free(&win);
    }
   


  // Finish up
    ierr = MPI_Finalize();
    return 0; 
}
    
  

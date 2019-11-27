#include "mpi.h"
#define MSG_SIZE 3

struct Communicator {
    MPI_Status  sta[3];
    MPI_Request req[3];
    int rank, size, master;
    int width;

    Communicator() {
        MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, NULL);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        master = size - 1;
        width = DATA_WIDTH;
    }
    
    ~Communicator() {
        MPI_Finalize();
    }
    
    void Send_rays(struct RayQueue* buffer, int size, int dst) {
        printf("Send to %d rays size %d", dst, buffer->get_size());
 //                       for(int i = 0; i < 10; i ++) {
 //                           printf("%d %f *", (int)(*buffer)[i * width], (*buffer)[i * width + 1]);
 //                       } 
        printf("\n");
        int buffer_size = buffer->get_size();
        float *data = buffer->rays();
        int    send_size = buffer_size > size ? size : buffer_size;
        printf("send ray send size %d buffer_size%d size %d", send_size, buffer_size, size);    
        MPI_Send(&data[(buffer_size - send_size) * width], send_size * width, MPI_FLOAT, dst, 1, MPI_COMM_WORLD);
        buffer->size -= send_size;
    }

    void Isend_rays(struct RayQueue* buffer, int size, int dst, int tag) {
        int buffer_size = buffer->get_size();
        float *data = buffer->rays();
        int    send_size = buffer_size > size ? size : buffer_size;
        printf("send ray send size %d buffer_size%d size %d", send_size, buffer_size, size);    
        MPI_Isend(&data[(buffer_size - send_size) * width], send_size * width, MPI_FLOAT, dst, 1, MPI_COMM_WORLD, &req[tag]);
//        MPI_Wait(&req[tag],sta);
        buffer->size -= send_size;
    }
   
    void Wait(int tag){
         MPI_Wait(&req[tag],sta);
    }

    int Recv_rays(struct RayQueue* buffer, int src) {
        // return 0 end, 1 no ray, 2 get rays
        int msg[3];
        int recv_num;
        MPI_Probe(src, 1, MPI_COMM_WORLD, sta);
        MPI_Get_count(&sta[0], MPI_FLOAT, &recv_num);
        if(recv_num == 1) {
            MPI_Recv(&msg, 1, MPI_FLOAT, src, 1, MPI_COMM_WORLD, sta);        
            return 0;
        } else if (recv_num == 2) {
            MPI_Recv(&msg, 2, MPI_FLOAT, src, 1, MPI_COMM_WORLD, sta);        
            return 1;
        } else {
            float *rays = &buffer->rays()[buffer->get_size() * width];
            MPI_Recv(rays, recv_num, MPI_FLOAT, src, 1, MPI_COMM_WORLD, sta); 
            buffer->size += recv_num / width;
            printf("get rays from %d recv_num %d size %d", src, recv_num / width, buffer->get_size());
            printf("\n");
            return 2;
        }
    }

    // msg[0] where msg from; msg[1] 1 has work 0 no work
    void Send_request(int id, bool has_work, int dst, int request_size){
        int msg[3];
        msg[0] = has_work ? 1 : 0;
        msg[1] = id;
        msg[2] = request_size;
        printf("send request %d %d\n", id, msg[2]);
        MPI_Send(&msg[0], MSG_SIZE, MPI_INT, dst, 1, MPI_COMM_WORLD);
    }

    void Send_noray(int dst) {
        float a[2];
        MPI_Send(&a, 2, MPI_FLOAT, dst, 1, MPI_COMM_WORLD);
    }


    void Send_end(int dst) {
        float a = 0;
        MPI_Send(&a, 1, MPI_FLOAT, dst, 1, MPI_COMM_WORLD); 
    }

    bool Master_recv(struct RayQueue* buffer, int *msg) {
        int recv_num, src;
        MPI_Probe(MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, sta);
        MPI_Get_count(&sta[0], MPI_INT, &recv_num);
        src = sta[0].MPI_SOURCE;
        if(recv_num == 3) {
            MPI_Recv(msg, 3, MPI_INT, src, 1, MPI_COMM_WORLD, sta);
            return true;
        } else {
            printf("master recv rays %d\n", recv_num / width);
            MPI_Get_count(&sta[0], MPI_FLOAT, &recv_num);
            MPI_Recv(buffer->rays(), recv_num, MPI_FLOAT, src, 1, MPI_COMM_WORLD, sta);
            buffer->size = recv_num / width;
            return false;
        }
    }
};



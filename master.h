#include <fstream>

pthread_t thread[2];
struct Master {
    struct RayQueue **rayqueue;
    struct RayQueue *raybuffer;
    int recv_capacity, buffer_capacity, slave_num; 
    bool *slave_wait;
    
    struct Communicator *comm;

    int recv_num, send_num;

    double recv_t, wait_t, send_t, total_t, read_t, write_t, st, ed;

    pthread_mutex_t mutex;
    pthread_cond_t  isempty;
    pthread_t pids[2];

    Master(struct Communicator *comm):comm(comm) {
        slave_num = comm->size - 1;
        recv_capacity = 1024 * 1024;
        buffer_capacity = 4 * recv_capacity;
        raybuffer = new struct RayQueue(recv_capacity); 
        rayqueue  = new struct RayQueue *[slave_num];
        slave_wait = new bool[slave_num];
        for(int i = 0; i < slave_num; i++){
            rayqueue[i] = new struct RayQueue(buffer_capacity * 2);
            slave_wait[i] = false;
        }
        
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&isempty, NULL);
    
        recv_num = 0; send_num = 0;
        
        st = clock();
        recv_t = 0; wait_t = 0;
        send_t = 0; total_t = 0;
        write_t = 0; read_t = 0;
    }

    ~Master(){
        for(int i = 0; i < slave_num; i++){
            delete rayqueue[i];
        }
        delete raybuffer;
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&isempty);
    }; 

    // ray queue isempty and recv all slave end msg
    bool shutdown(){
            return false;
    }

    void run(){
        int msg[3];
        bool all_done = false;
        for(;;){
            // master recv, true: recv a request  
            //             false: recv ray data
            double t = clock();
            if(comm->Master_recv(raybuffer, &msg[0])){
                recv_t += (double)(clock() - t) / CLOCKS_PER_SEC;
                if(msg[0] == 0) {
                    // slave has no work
                    int id = msg[1];
                    slave_wait[id] = true;
                    if(rayqueue[id]->isempty()) {
                        // if slave wait and no rays in its queue
                        all_done = true;
//                        printf("%d rayqueue empty\n", id);
                        for(int i = 0; i < slave_num; i++){
                            if(!slave_wait[i] || !rayqueue[i]->isempty()) {
                                all_done = false; break;
                            }
                        }
                    }
                    if(all_done) {
                        break;
                    }
                } 
                int dst = msg[1];  
                int request_size = msg[2];

                t = clock();
                if(!rayqueue[dst]->isempty()) {
                    slave_wait[dst] = false;
                    struct RayQueue *r = rayqueue[dst]; 
                    comm->Send_rays(rayqueue[dst], 1048576, dst);
                } else {
                    comm->Send_noray(dst);
                }
                send_t += (double)(clock() - t) / CLOCKS_PER_SEC;
            } else {
                recv_t += (double)(clock() - t) / CLOCKS_PER_SEC;
                int recv_size = raybuffer->get_size();
                
                ////write to file compare
//                FILE * fp;
//                t = clock();
//                fp = fopen ("file.txt", "w+");
//                for(int i = 0; i< recv_size; i++){
//                    fprintf(fp, "%d %f", (int)(*raybuffer)[i * 2], (*raybuffer)[i * 2 + 1]);
//                }
//                fclose(fp);
//                write_t += (double)(clock() - t) / CLOCKS_PER_SEC;
//                t = clock();
//                fp = fopen("file.txt","r");
//                while(1)
//                {
//                    float c = fgetc(fp);
//                    if( feof(fp) )
//                    { 
//                         break ;
//                    }
//                }
//                fclose(fp);
//                read_t += (double)(clock() - t) / CLOCKS_PER_SEC;
                ///////////////////////////

                // arrange data
                int width = DATA_WIDTH;
                for(int i = 0; i < recv_size; i++){
                    int id = (int)(*raybuffer)[i * width];
                    rayqueue[id]->put(id, (*raybuffer)[i * width + 1]);
                    if(rayqueue[id]->isfull()){
                        printf("error queue is isfull %d %d %d\n", id, rayqueue[id]->get_size(), rayqueue[id]->get_capacity());   
                        rayqueue[id]->clear();
                    }
                }
                for(int i = 0; i < slave_num;i++){
                    printf("%d %d \n", i, rayqueue[i]->get_size());
                }
                raybuffer->clear();
            }
        }
        for(int i = 0; i < slave_num; i++){
             comm->Send_end(i);
        }
        total_t = (double)(clock() - st) / CLOCKS_PER_SEC;
        printf( "\n Master recv time%f send time %f  total %f \n write %f read %fseconds\n", recv_t, send_t, total_t, write_t, read_t );

//        pthread_attr_t attr;
//        pthread_create(&thread[0], NULL, recv, (void*)this);
//        pthread_create(&thread[1], NULL, service, (void*)this);
//
//        pthread_join(thread[0], NULL);
//        pthread_join(thread[1], NULL);
//
//        pthread_exit(NULL);
    }
};

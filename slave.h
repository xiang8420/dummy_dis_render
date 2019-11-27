struct Slave {
    struct Ray *wait_buffer;
    struct RayQueue *send_buffer;
    struct RayQueue *wait_send_buffer;
    struct Ray *primary;
    struct RayQueue *queue;
    
    int buffer_size, slave_num; 
    int mpi_rank, master;
    bool has_work, stop_recv;

    struct Communicator *comm;
    struct Renderer *render; 

    int proc_ray_num, recv_ray_num, send_ray_num;
    double recv_t, send_t;

    pthread_mutex_t mutex;
    pthread_t pid[3];

    Slave(struct Communicator *comm):comm(comm) {
        has_work = true;
        stop_recv = false;
        mpi_rank = comm->rank;
        slave_num = comm->size - 1;
        master = slave_num;
        render = new Renderer(slave_num); 
        buffer_size      = 1024 * 1024;
        wait_buffer      = new struct Ray(buffer_size);
        primary          = new struct Ray(buffer_size);
        send_buffer      = new struct RayQueue(buffer_size);
        wait_send_buffer = new struct RayQueue(buffer_size);
        queue            = new struct RayQueue(buffer_size * 4);
        pthread_mutex_init(&mutex,NULL);
        
        proc_ray_num = 0; recv_ray_num = 0; send_ray_num = 0;
        recv_t = 0; send_t = 0;
    }
    
    ~Slave(){
        pthread_mutex_destroy(&mutex);
        // delete 
    }; 

    static void* request(void* tmp) {
        struct Slave *p = (struct Slave*)tmp;
        struct RayQueue *queue = p->queue;
        
        struct Ray *wait_buffer = p->wait_buffer;
        int queue_capacity = queue->get_capacity();
        int buffer_size = p->buffer_size;
        struct Communicator *comm = p->comm;
        int width = DATA_WIDTH; 
        //start recv thread and render
        for(;;){
            usleep(300);
            pthread_mutex_lock(&p->mutex);
            if(!queue->isempty() && wait_buffer->isempty()){
                printf("fill wait buffer\n");
                int queue_size = queue->get_size();
                int st = queue_size > buffer_size ? queue_size - buffer_size : 0;
                int n  = st == 0 ? queue_size : buffer_size;
                printf("%d %d %d\n", queue_size, buffer_size, st);
               
                for(int i = 0; i < n; i++){
                    int id = width * (st + i);
                    wait_buffer->put((int)(*queue)[id], (*queue)[id + 1]);
                } 
                queue->size -= wait_buffer->get_size();
            }
            pthread_mutex_unlock(&p->mutex);

            if(queue->get_size() + buffer_size < queue_capacity && !p->stop_recv) {
                comm->Send_request(p->mpi_rank, p->has_work, p->master, queue->capacity - queue->size);
                int msg = comm->Recv_rays(queue, p->master);
                if(msg == 0){
                    printf("slave recv stop\n");
                    p->stop_recv = true;
                } else if(msg == 2) {
//                    printf("%d get rays ", p->mpi_rank);
//                    for(int i = 0; i < 10; i ++) {
//                        printf("%d %f |", (int)(*queue)[i * width], (*queue)[i * width + 1]);
//                    }
                    //wrong
                    p->recv_ray_num += buffer_size;
                    printf("\n");
                }
            } 
            if (queue->isempty() && p->stop_recv){
                return NULL;
            }
        }   
        return NULL;
    }
   
    void schedule(struct Ray* &primary) {
        int size = primary->get_size();
        int capacity = primary->get_capacity();
        int primary_size = primary->get_size();

        primary->clear();
        bool first = true;
        for(int i = 0; i < primary_size; i++) {
            int id = (int)((*primary)[i]);
            if(id == mpi_rank) {
                primary->put((int)(*primary)[i], (*primary)[i + capacity]);
            } else if (id == slave_num) {
                continue;
            } else{
                wait_send_buffer->put((int)(*primary)[i], (*primary)[i + capacity]);
                if(wait_send_buffer->isfull()) {
//                    printf("\nslave send data:");
                    send_ray_num += wait_send_buffer->get_size(); 
                    printf("send buffer_size%d \n\n", wait_send_buffer->get_size());
                    
                    if(!first) {comm->Wait(0);} else {first = false;}
                    swap(send_buffer, wait_send_buffer); 
                    
                    comm->Isend_rays(send_buffer, send_buffer->get_size(), master, 0);
//                    comm->Wait(0);
                }
            }
        }
        pthread_mutex_lock(&mutex);
        if(primary_size < 100) {
            if(!wait_buffer->isempty()){
                printf("slave swap wait buffer %d :", mpi_rank);
                swap(primary, wait_buffer);
                wait_buffer->clear();
                printf("get rays %d\n", primary->get_size());
                has_work = true;
            } else {
                if(!wait_send_buffer->isempty()){
                    if(!first) {comm->Wait(0);} else {first = false;}
                    swap(send_buffer, wait_send_buffer); 
                    send_ray_num += send_buffer->get_size();
                    printf("send buffer_size2%d \n\n", send_buffer->get_size());
                    comm->Isend_rays(send_buffer, send_buffer->get_size(), master, 0);
//                    comm->Wait(0);
                }
                has_work = false;    
            } 
        }
        pthread_mutex_unlock(&mutex);
    }

    void run() {
        pthread_attr_t attr;
        pthread_create(&thread[0], NULL, request, (void*)this); 

        int num_rays = 1024 * 1024 * 4;
        int id = 0;
        int capacity = primary->get_capacity();
        while( has_work || !stop_recv) {
            if(primary->get_size() < capacity && id < num_rays) {
                render->generate_rays(primary, id, mpi_rank, num_rays);
            }
            schedule(primary);
            if(primary->get_size() > 0)
                printf("%d primary %d id %d\n", mpi_rank, primary->get_size(), id);
            render->intersection(primary, mpi_rank, proc_ray_num);
            usleep(500);
//            render->shade(primary);
            render->compact(primary);          
        }
        printf("slave over processed ray %d recv %d send %d send buffer %d\n", proc_ray_num, recv_ray_num, send_ray_num, send_buffer->get_size());
        pthread_join(thread[0], NULL);
    }
};


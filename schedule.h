#include "comm.h"
#include <pthread.h>
#include "master.h"
#include "slave.h"

//hold rayqueue memory
struct Scheduler {
    struct Renderer;

    Scheduler(){};

    ~Scheduler(){}; 

    void run(){};

};

//struct Scheduler sched;



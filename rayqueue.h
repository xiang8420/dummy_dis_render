#define DATA_WIDTH 20
#define QUEUE_CAPACITY 1048576

struct Ray {
    float *data;
    int size, capacity;

    Ray(int capacity): capacity(capacity) {
        data = new float[capacity * DATA_WIDTH];
        size = 0;
    }

    ~Ray(){
        delete[] data;
    }
    
    // queue store is different from orign ray
    void put(int id, float info){
        data[size] = id;
        data[size + capacity] = info;
        size++;
    }

    float& operator[](int id) const {
        return data[id]; 
    }

    bool isfull() { return size == capacity; }
    
    bool isempty() {return size == 0;}
  
    float* rays() {return data; }

    int* ids() {return (int*)data;}

    float* datas() {return &data[capacity];}


    void clear() {size = 0;}

    int get_size() { return size;}

    int get_capacity() {return capacity;}
};

struct RayQueue {
    float *data;
    int size, capacity;

    RayQueue(int capacity): capacity(capacity) {
        data = new float[capacity * DATA_WIDTH];
        size = 0;
    }

    ~RayQueue(){
        delete[] data;
    }
    
    // queue store is different from orign ray
    void put(int id, float info){
        data[size * DATA_WIDTH] = id;
        data[size * DATA_WIDTH + 1] = info;
        size++;
    }

    float& operator[](int id) const {
        return data[id]; 
    }

    bool isfull() { return size == capacity; }
    
    bool isempty() {return size <= 0;}
  
    float* rays() {return data; }

    int* ids() {return (int*)data;}

    float* datas() {return &data[capacity];}


    void clear() {size = 0;}

    int get_size() { return size;}

    int get_capacity() {return capacity;}
};

inline void swap(struct RayQueue* &a, struct RayQueue* &b) {
    struct RayQueue* tmp;
    tmp = a;
    a = b;
    b = tmp;
}

inline void swap(struct Ray* &a, struct Ray* &b) {
    struct Ray* tmp;
    tmp = a;
    a = b;
    b = tmp;
}

//bool put(struct RayQueue *queue, struct Ray *ray) {
//    int queue_size = queue->get_size();
//}



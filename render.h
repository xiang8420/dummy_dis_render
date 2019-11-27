#include <stdio.h>
#include <time.h>
#include <algorithm>

struct Renderer {
    int slave_num;
    Renderer(int slave_num):slave_num(slave_num){
        srand((unsigned) time (NULL) ); 
    }
    ~Renderer(){};
    
    //generate rays, fill rays by random element
    void generate_rays(struct Ray *primary, int &id, int d, int num_rays){
        int* ray_id = primary->ids();
        float* data = primary->datas();
        int capacity = primary->get_capacity();
        int generate_size = std::min(num_rays - id, capacity - primary->get_size());

        for(int i = 0; i < generate_size; i++){
            primary->put(rand() % slave_num, d);
        }
        id += generate_size;
    }
    
    //intersection, if ray is in our region
    void intersection(struct Ray* primary, int mpi_id, int &n) {
        int size = primary->get_size();
//        printf("node %d intersection %d\n", mpi_id, size);
        for(int i = 0; i < size; i++) {
//            if(rand() % 10 < 5 || size < 5000) {
                (*primary)[i] = slave_num;
//            } else {
//                (*primary)[i] = rand() % slave_num;
//            }
            n++;
        }
    }

    void compact(struct Ray* primary) {
        int size = primary->get_size();
        int capacity = primary->get_capacity();
        primary->clear();
        float* data = &(*primary)[0];
        for(int i = 0; i < size; i++) {
            if(data[i] != slave_num){
                primary->put((int)data[i], data[i + capacity]);
            }
        }
    }

    //shade, reflection, dead, or maybe generates secondary
    void shade(struct Ray* primary) {
        float* data = &(*primary)[0];
        int size = primary->get_size();
        int capacity = primary->get_capacity();

        for(int i = 0; i < size; i++) {
            if(data[i] > 0) {
                if(i % 2 == 0) {
                    data[i + capacity] = 111.0;                        
                }
            }
        }
    }

    


    void render(float* primary, int primary_capacity, float* buffer, int buffer_capacity) {
        
//        int primary_size = 0;
//        int buffer_size = 0;
//
//    
//        int id = 0;
//        int num_rays = 4 * 1024 * 1024;
//        while(primary_size > 100 || id < num_rays){
//            if(primary_size < primary_capacity && id < num_rays) {
//                printf("generate rays\n");
//                generate_rays(primary, primary_size, id, primary_capacity, num_rays);
//            }
//            printf("primary %d id %d\n", primary_size, id);
//            transfer(primary, primary_size, primary_capacity, buffer, buffer_size, buffer_capacity);
//            intersection(primary, primary_size);
//            shade( primary, primary_size, primary_capacity);
//        }
    }
};



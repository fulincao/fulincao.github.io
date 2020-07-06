#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <semaphore.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <semaphore.h>
#include <vector>


class ShmFlow{
    public:
        
        //  l  r  id data 
        //  4  4  4  _shm_size
        ShmFlow(char* _topic, int _shm_size=666666){
            queue_size = _shm_size;
            head_size = 12;
            shm_size = _shm_size + head_size;
            topic = (char*)malloc(strlen(_topic) + 1);
            strcpy(topic, _topic);
            parare();
        }

        ~ShmFlow(){

            if( sem_post(sem) == -1) {
                perror("sem_post");
                exit(EXIT_FAILURE);
            }

            if(sem_close(sem) == -1){
                perror("sem_close");
                exit(EXIT_FAILURE);
            }
            if(munmap(buf, shm_size) == -1){
                perror("munmap");
                exit(EXIT_FAILURE);
            }


        }

        uint8_t* read(){
            
            
            if( sem_wait(sem) == -1) {
                perror("sem_wait");
                exit(EXIT_FAILURE);
            }


            uint32_t l = _uint8_2_uint32(0); //
            uint32_t r = _uint8_2_uint32(4); //
            uint32_t last_fid = _uint8_2_uint32(8);

            if( read_index == -1) {
                read_index = (l + 1)%queue_size;
            }else{
                if(last_fid <= read_fid) {
                    if( sem_post(sem) == -1) {
                        perror("sem_wait");
                        exit(EXIT_FAILURE);
                    }
                    return NULL;
                }
                uint32_t mi_fid = _uint8_2_uint32(l + 1 + head_size + 4);  

                if( mi_fid <= read_fid) {
                    uint32_t s = _uint8_2_uint32(read_index + head_size);
                    read_index = (s + read_index) % queue_size;
            
                }else read_index = (l + 1)%queue_size;
            }
            uint32_t s = _uint8_2_uint32(read_index + head_size);
            uint8_t* data = (uint8_t*)malloc(s);
         
            if( s + read_index > queue_size){
                uint32_t dt = s + read_index - queue_size;

                memcpy(data, buf + head_size + read_index, s - dt);
                memcpy(data + s - dt, buf + head_size, dt);

            }else{
                memcpy(data, buf + head_size + read_index, s);
            }
            read_fid = _uint8_2_uint32(read_index + head_size + 4);
            printf("read fid os %d\n", read_fid);
            
            if( sem_post(sem) == -1) {
                perror("sem_post");
                exit(EXIT_FAILURE);
            }
            return data;
        }

        void write(uint8_t* data, uint32_t size){
            
            if( sem_wait(sem) == -1) {
                perror("sem_wait");
                exit(EXIT_FAILURE);
            }

            uint32_t l = _uint8_2_uint32(0); //
            uint32_t r = _uint8_2_uint32(4); // 
            uint32_t fid = _uint8_2_uint32(8);

            // 构造数据
            uint8_t* tmp = (uint8_t*)malloc(size + 8);
            memcpy(tmp+8, data, size);
            size += 8;
            fid += 1;
            for( int i = 0; i < 4; i++ ) tmp[3-i] = size >> (i * 8) & 0xFF;
            for( int i = 0; i < 4; i++ ) tmp[7-i] = fid >> (i * 8) & 0xFF;

            int real_size = (( r - l ) + queue_size) % queue_size;
            int remain = queue_size - real_size - 1;

            while( remain < size) {
                uint32_t s = _uint8_2_uint32(l + 1 + head_size);                
                l = (l + s)%queue_size;
                remain += s;
            }

            if( queue_size - r - 1 < size) {
                int dt = size - queue_size + r + 1;
                memcpy(buf + r + head_size + 1, tmp, queue_size - r - 1);
                memcpy(buf + head_size, tmp + queue_size - r -1, dt);
                r = dt - 1;
            }else{
                memcpy(buf + r + head_size + 1, tmp, size);
                r = r + size;
            }

            for( int i = 0; i < 4; i++ ) buf[3-i] = l >> (i * 8) & 0xFF;
            for( int i = 0; i < 4; i++ ) buf[7-i] = r >> (i * 8) & 0xFF;    
            for( int i = 0; i < 4; i++ ) buf[11-i] = fid >> (i * 8) & 0xFF;
            printf("write fid is %d\n", fid);

            if( sem_post(sem) == -1) {
                perror("sem_post");
                exit(EXIT_FAILURE);
            }

        }

        uint32_t _uint8_2_uint32(int start = 0){
            uint32_t res = 0;
            for( uint32_t i = start; i < start + 4; i++){
                uint32_t tmp = i;
                if( tmp >= shm_size) tmp = tmp - shm_size + head_size;
                res  = (res << 8) + buf[tmp];
            }
            return res;
        }


    private:
        char* topic;
        uint32_t fid, shm_size, head_size;;
        uint32_t read_index = -1, queue_size, read_fid = -1;
        uint8_t* buf;
        sem_t* sem = NULL;

        void parare(){
            int fd = shm_open(topic, O_CREAT | O_EXCL | O_RDWR, 0644);

            if (fd == -1){
                fd = shm_open(topic, O_EXCL | O_RDWR, 0644);
                if( fd == -1) {
                    perror("shm_open");
                    exit(EXIT_FAILURE);
                } 
            }

            if( ftruncate(fd, shm_size) == -1){
                perror("ftruncate");
                exit(EXIT_FAILURE);
            }
            buf = (uint8_t*)mmap(NULL, shm_size, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_SHARED, fd, 0);
            if (buf == MAP_FAILED){
                perror("mmap");
                exit(EXIT_FAILURE);
            }
            close(fd);

            sem = sem_open(topic, O_CREAT | O_RDWR  | O_EXCL, 0644, 1);
            if( sem == SEM_FAILED){

                sem = sem_open(topic, O_RDWR  | O_EXCL, 0644, 1);

                if(sem == SEM_FAILED) {
                    perror("sem_open");
                    exit(EXIT_FAILURE);
                }
            }

        }

};


extern "C"{
    ShmFlow* shmflow = NULL;

   
    void init_flow(char* topic, int size){
        if(shmflow == NULL)  shmflow = new ShmFlow(topic, size);
        printf("shmflow init ok\n");
    }

    void write_data(char* data, int size) {
        uint8_t* t = (uint8_t*)data;
        shmflow->write(t, size);
    }

    int read_data(char* data){
        uint8_t* res = shmflow->read();
        if( res == NULL) return 0;
        uint32_t tmp = 0;
        for( uint32_t i = 0; i < 4; i++) tmp = (tmp << 8) + res[i];
        memcpy(data, res, tmp);
        return 1;
    }

}

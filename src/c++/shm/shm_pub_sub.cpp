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


class ShmFlow{
    public:
        
        ShmFlow(char* _topic, int _max_one_data_size=10000, int cache_size=10){
            max_one_data_size = _max_one_data_size;
            data_size = cache_size * (max_one_data_size + 8);
            head_size = 2*4 + cache_size*4;
            shm_size = data_size + head_size;
            topic = (char*)malloc(strlen(_topic) + 1);
            
            strcpy(topic, _topic);
            queue_size = cache_size;
            parare();
        }

        uint8_t read(){
            printf("%d\n", buf[shm_size-1]);
        }

        void write(uint8_t* data, uint32_t size){
            uint32_t l = _uint8_2_uint32(0); //
            uint32_t r = _uint8_2_uint32(4); // 


            // 构造数据
            uint8_t* tmp = (uint8_t*)malloc(size + 8);
            memcpy(tmp+8, data, size);
            for( int i = 0; i < 4; i++ ) tmp[3-i] = size >> (i * 8) & 0xFF;
            size += 8;


            if( l == r) {
                printf("cache empty\n");
                r -= 4;
                r = (r + 4)%(queue_size*4);
                r += 4;
                uint32_t rear_start = _uint8_2_uint32(r);
                uint32_t rear_size = 0;
                cpy_2_buffer(tmp, size, rear_start + rear_size);
                
                

                return;
            }
            // 获取剩余大小
            uint32_t rear_start = _uint8_2_uint32(r);
            uint32_t rear_size = _uint8_2_uint32(rear_start);

            uint32_t front_start = _uint8_2_uint32(l);
            int remain = front_start - rear_start - rear_size;


            if(remain >= size){
                cpy_2_buffer(tmp, size, rear_start + rear_size);
            }else{
                while (remain < size)
                {
                    l = l + 4;
                    if( l >= head_size ) l = 8;


                }
                


            }
            remain -= size;

        }

    private:
        char* topic;
        int max_one_data_size, shm_size;
        int read_index = 0, queue_size, data_size;
        int head_size;
        uint8_t* buf;

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
        }

        uint32_t _uint8_2_uint32(int start = 0){
            uint32_t res = 0;
            for( int i = start; i < start + 4; i++){
                res  = (res << 8) + buf[i];
            }
            return res;
        }

        void cpy_2_buffer(uint8_t* data, int size, int buf_start) {
            if( buf_start + size >= shm_size) {
                int dt = buf_start + size - shm_size - 1;
                memcpy(buf + buf_start, data, size - dt);
                memcpy(buf + head_size, data + size - dt, dt);

            }else{
                memcpy(buf+buf_start, data, size);
            }

        }

};

int main(int argc, char* argv[]){

    ShmFlow sf("test_shm");
    sf.read();
    // sf.write();
    
    return 0;
}
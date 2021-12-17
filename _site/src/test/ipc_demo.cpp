#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <semaphore.h>

#define SHM_NAME "./build/shm"
#define SEM_NAME "sem"
#define SHM_SIZE 1024


int main( int argc, char* argv[]) {


    int fd = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0644);

    if (fd == -1){
        fd = shm_open(SHM_NAME, O_EXCL | O_RDWR, 0644);
        printf("%d\n", fd);
        // perror("shm_open");
        // exit(EXIT_FAILURE);
    }
    
    int res = ftruncate(fd, SHM_SIZE);
    if( res == -1){
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    char* data = (char*)mmap(NULL, SHM_SIZE, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    close(fd);

  


    int pid = fork();

    
    if( pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }else if (pid == 0){
          sem_t* sem = sem_open(SEM_NAME,   O_RDWR  | O_EXCL, 0644, 1);
    if( sem == SEM_FAILED){
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

        usleep(3);

        printf("child_proc(%d) waiting to read ... \n", getpid());
        if(sem_wait(sem) == -1){
            perror("sem_wait");
            exit(EXIT_FAILURE);
        }

        printf("child_proc(%d) read_data: %s\n", getpid(), data);

        if(sem_post(sem) == -1){
            perror("sem_post");
            exit(EXIT_FAILURE);
        }


        if(sem_close(sem) == -1){
            perror("sem_close");
            exit(EXIT_FAILURE);
        }

        if(munmap(data, SHM_SIZE) == -1){
            perror("munmap");
            exit(EXIT_FAILURE);
        }



    }else if (pid > 0){
          sem_t* sem = sem_open(SEM_NAME, O_CREAT | O_RDWR  | O_EXCL, 0644, 1);
    if( sem == SEM_FAILED){
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

        // sem_t* sem = sem_open(SEM_NAME, O_RDWR  | O_EXCL, 0644, 1);
        res = sem_wait(sem);
        if( res == -1) {
            perror("sem_wait");
            exit(EXIT_FAILURE);
        }
        printf("parent_proc(%d) writing now ... \n", getpid());
        for(int i=0; i<3; i++){
            sprintf(data + 5*i, "www%d ", i);
            sleep(1);
        }
        printf("parent_proc(%d) writing complete\n", getpid());

        if(sem_post(sem) == -1){
            perror("sem_post");
            exit(EXIT_FAILURE);
        }

        if(sem_close(sem) == -1){
            perror("sem_close");
            exit(EXIT_FAILURE);
        }

        if(munmap(data, SHM_SIZE) == -1){
            perror("munmap");
            exit(EXIT_FAILURE);
        }

        if(sem_unlink(SEM_NAME) == -1){
            perror("sem_unlink");
            exit(EXIT_FAILURE);
        }
    }
    
    return 0;
}
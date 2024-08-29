#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>


#define NUM_OF_THREADS 4
#define NUM_OF_CHUNKS 4

int THREAD_ORDER_EXECUTION = 0;
pthread_cond_t orderCon = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

struct compressed_data {
    int num_of_char;
    char character;
};

typedef struct {
    long start;
    long end;
    char *filename;
    char *characters_Array;
    int order;
    pthread_t thread_id;
} worker_args;

void* worker_compresser(void* arg) {
    worker_args *args = (worker_args*) arg;
    
    char c;
    char prev_c;
    int counter = 0; 
    int len = args->end - args->start;
    
    struct compressed_data *data =  malloc(sizeof(struct compressed_data) * len);
    if(data == NULL) {
        perror("malloc");
        exit(1);
    }

    int usedSpace = 0;
    prev_c = args->characters_Array[0];
    for(int i = 0; i < len; i++) {
        c = args->characters_Array[i];
        if(c == prev_c) {
            counter++;
        } else {
            data[usedSpace].num_of_char = counter;
            data[usedSpace].character = prev_c;
            counter = 1;
            prev_c = c;
            usedSpace++;
        }
    }
    
    // Storing the last character
    if(counter >= 1) {
        data[usedSpace].num_of_char = counter;
        data[usedSpace].character = prev_c;
        usedSpace++;
    }
    
    // edit the writing to stdout
    pthread_mutex_lock(&lock);
    while(THREAD_ORDER_EXECUTION != args->order) 
        pthread_cond_wait(&orderCon, &lock);
    fwrite(data, sizeof(struct compressed_data), usedSpace, stdout);
    THREAD_ORDER_EXECUTION++;
    pthread_cond_broadcast(&orderCon);
    pthread_mutex_unlock(&lock);

    free(data);
    return NULL;
}

int main(int argc, char *argv[]) {
    // check for files
    if(argc <= 1) {
        printf("Usage is not correct\n");
        return 1;
    }

    for(int i = 0; i < argc-1; i++) {
        // get size of files
        struct stat st;
        if(stat(argv[i+1], &st) == -1) {
            perror("stat");
            return 1;
        }

        if(st.st_size >= 1024 * 1024 ) {
            int fd = open(argv[i+1], O_RDONLY);
            char *characters_Array = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);

            int bufferSize = st.st_size / NUM_OF_CHUNKS;
            worker_args args[NUM_OF_THREADS];

            for(int x = 0; x < NUM_OF_CHUNKS; x++) {
                int chunkStart = x * bufferSize;
                int chunkEnd = (x == NUM_OF_CHUNKS - 1) ? st.st_size - 1 : (x + 1) * bufferSize;
                int chunkSize = (chunkEnd - chunkStart + 1) / NUM_OF_THREADS;
                
                for (int j = 0; j < NUM_OF_THREADS; j++) {
                    if(j != 0)
                        args[j].start = (chunkStart + j * chunkSize) + 1;
                    else 
                        args[j].start = (chunkStart + j * chunkSize);
                    args[j].end = (j == NUM_OF_THREADS - 1) ? chunkEnd : (chunkStart + (j + 1) * chunkSize - 1);
                    args[j].characters_Array = &characters_Array[args[j].start];
                    args[j].order = j;
                }

                for(int j = 0; j < NUM_OF_THREADS; j++) 
                    pthread_create(&args[j].thread_id, NULL, worker_compresser, &args[j]);
                
                for(int j = 0; j < NUM_OF_THREADS; j++) 
                    pthread_join(args[j].thread_id, NULL);

                THREAD_ORDER_EXECUTION = 0;
            }

            munmap(characters_Array, st.st_size);

        } else {
            FILE* fd = fopen(argv[i+1], "r");

            char c;
            char prev_c = 0;
            int counter = 0; 

            while((c = fgetc(fd)) != EOF) {
                if(c == prev_c) {
                    counter++;
                } else {
                    fwrite(&counter, 4, 1, stdout);
                    fwrite(&prev_c, 1, 1, stdout);
                    prev_c = c;
                    counter = 1;
                }
            } 

            if(counter >= 1) {
                fwrite(&counter, 4, 1, stdout);
                fwrite(&prev_c, 1, 1, stdout);
            }
        }

    }
    
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&orderCon);

}

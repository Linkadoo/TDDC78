#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ppmio.h"
#include "thresfilter.h"

pthread_mutex_t mutex;
pthread_cond_t cond;
int counter, global_sum, global_sum_done;

void *work_thread(void* tParam){
	struct thread_data *myData;
	myData=(struct thread_data*) tParam;

	long me;
	int NUM_THREADS, xsize, ysize, part_start, part_length,i,global_sum;
	pixel* src;
	int *sum;
	
	NUM_THREADS = myData->started_threads;
	me = myData->threadID;
	xsize=myData->xsize;
	ysize=myData->ysize;
	src=myData->src;
	sum=myData->local_sum;
	
	calc_part(&part_start,&part_start,me,NUM_THREADS,ysize);
	threshfilter_part_1(xsize,part_start, part_length,src);
	pthread_mutex_lock(&mutex);
    counter++;
    pthread_cond_broadcast(&cond);
    while(counter < NUM_THREADS){
    	pthread_cond_wait(&cond, &mutex);
    }
    if(me == 0){
    	for(i=0; i<NUM_THREADS; i++){
    		global_sum = *(sum + i);
    	}
    	global_sum = global_sum/(xsize*ysize);
    	global_sum_done = 1;
    	
    }
    pthread_cond_broadcast(&cond);
    while(counter < NUM_THREADS){
    	pthread_cond_wait(&cond, &mutex);
    }
	pthread_mutex_unlock(&mutex);
    threshfilter_part_2(xsize,part_start,part_length, src,global_sum);

	return NULL;
}


int main (int argc, char ** argv) {
    int xsize, ysize, colmax, NUM_THREADS,i;
    pixel* src;
    struct timespec stime, etime;

    /* Take care of the arguments */

    if (argc != 4) {
	fprintf(stderr, "Usage: %s infile outfile\n", argv[0]);
	exit(1);
    }
	NUM_THREADS = atoi(argv[1]);
	int local_sum[NUM_THREADS];
    /* read file */
    if(read_ppm (argv[2], &xsize, &ysize, &colmax, (char *) src) != 0)
        exit(1);

    if (colmax > 255) {
	fprintf(stderr, "Too large maximum color-component value\n");
	exit(1);
    }
    src = malloc(MAX_PIXELS*sizeof(*src));

    printf("Has read the image, calling filter\n");
    clock_gettime(CLOCK_REALTIME, &stime);
	
	pthread_t thread_handle[NUM_THREADS];
	struct thread_data thread_data_array[NUM_THREADS];
	for(i=0; i<NUM_THREADS; i++){
		thread_data_array[i].xsize = xsize;
		thread_data_array[i].ysize = ysize;
		thread_data_array[i].src = src;
		thread_data_array[i].started_threads = NUM_THREADS;
		thread_data_array[i].local_sum = &(local_sum[i]);
	}
	for(i=0; i<NUM_THREADS; i++) {
		thread_data_array[i].threadID = i;
		pthread_create(&thread_handle[i], NULL, work_thread, &(thread_data_array[i]));
	}
	
	for(i=0; i<NUM_THREADS; i++) {
		pthread_join(thread_handle[i], NULL);
	}
	

    clock_gettime(CLOCK_REALTIME, &etime);

    printf("Filtering took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
	   1e-9*(etime.tv_nsec  - stime.tv_nsec)) ;

    /* write result */
    printf("Writing output file\n");
    
    if(write_ppm (argv[3], xsize, ysize, (char *)src) != 0)
      exit(1);


    return(0);
}

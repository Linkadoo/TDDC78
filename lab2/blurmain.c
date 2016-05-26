#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
#include "ppmio.h"
#include "blurfilter.h"
#include "gaussw.h"

pthread_mutex_t mutex;
pthread_cond_t cond;
int counter;
#define MAX_RAD 3000

void* work_thread(void* tParam){
	
	struct thread_data *myData;
	myData=(struct thread_data *) tParam;
	
	long me;
	int radius, NUM_THREADS, xsize, ysize, part_start, part_length;
	pixel* src;
	pixel* halfway;
	double w[MAX_RAD];
	
	NUM_THREADS = myData->started_threads;
	me = myData->threadID;
	radius = myData->radius;
	xsize = myData->xsize;
	ysize = myData->ysize;
	src = myData->src;
	halfway = myData->halfway;

	calc_part(&part_start, &part_length, me, NUM_THREADS, ysize);
	get_gauss_weights(radius,w);
	blurfilter_part_1(xsize, src, radius, w, halfway, part_start, part_length);
	pthread_mutex_lock(&mutex);
    counter++;
    pthread_cond_broadcast(&cond);
    while(counter < NUM_THREADS){
    	pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
    blurfilter_part_2(xsize, ysize, src, radius, w, halfway, part_start, part_length);

	return NULL;
}	


int main (int argc, char ** argv) {

	int i, NUM_THREADS, xsize, ysize, colmax, radius;
	double w[MAX_RAD];
	pixel* src;
	pixel* halfway;
	int init_id = 0;
	struct timespec stime, etime;
	
		/* Take care of the arguments */
	if (argc != 5) {
			fprintf(stderr, "Usage: %s threads radius infile outfile\n", argv[0]);
			exit(1);
	}
	NUM_THREADS = atoi(argv[1]);
	radius = atoi(argv[2]);
	
	if((radius > MAX_RAD) || (radius < 1)) {
		fprintf(stderr, "Radius (%d) must be greater than zero and less than %d\n", radius, MAX_RAD);	
		exit(1);
	}
	//Allocating space for the picture
	src = malloc(MAX_PIXELS*sizeof(*src));
	halfway = malloc(MAX_PIXELS*sizeof(*src));
	
	//Reading file
	if(read_ppm (argv[3], &xsize, &ysize, &colmax, (char *) src) != 0){		
		exit(1);
	}
	if (colmax > 255) {
		fprintf(stderr, "Too large maximum color-component value\n");
		exit(1);
	}
	printf("Has read the image, starting time...\n");
	clock_gettime(CLOCK_REALTIME, &stime);
	
	pthread_t thread_handle[NUM_THREADS];
	struct thread_data thread_data_array[NUM_THREADS];
	for(i=0; i<NUM_THREADS; i++){
		thread_data_array[i].xsize = xsize;
		thread_data_array[i].ysize = ysize;
		thread_data_array[i].radius = radius;
		thread_data_array[i].src = src;
		thread_data_array[i].halfway = halfway;
		thread_data_array[i].started_threads = NUM_THREADS;
	}
	for(i=0; i<NUM_THREADS; i++) {
		thread_data_array[i].threadID = i;
		pthread_create(&thread_handle[i], NULL, work_thread, &(thread_data_array[i]));
	}
		
	for(i=0; i<NUM_THREADS; i++) {
		pthread_join(thread_handle[i], NULL);
	}
	
	clock_gettime(CLOCK_REALTIME, &etime);

    printf("Threads eliminated, filtering took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
	   1e-9*(etime.tv_nsec  - stime.tv_nsec)) ;

    /* write result */
    printf("Writing output file...\n");
    
    if(write_ppm (argv[4], xsize, ysize, (char *)src) != 0)
      exit(1);
	
	return 0;
}

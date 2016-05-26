#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
//#include "ppmio.h"
//#include "blurfilter.h"
//#include "gaussw.h"
sem_t init_threads;

int main (int argc, char ** argv) {
	
	int i, nt, xsize, ysize, colmax;
	pixel* src;
	pthread_t thread_handle[nt];

	int init_id = 0;
	int* ID = &init_id;
	
		/* Take care of the arguments */
	if (argc != 5) {
			fprintf(stderr, "Usage: %s threads radius infile outfile\n", argv[0]);
			MPI_Finalize();
			exit(1);
	}
	nt = atoi(argv[2]);
	radius = atoi(argv[1]);
	if((radius > MAX_RAD) || (radius < 1)) {
		fprintf(stderr, "Radius (%d) must be greater than zero and less than %d\n", radius, MAX_RAD);	
		MPI_Finalize();
		exit(1);
	};
	//Allocating space for the picture
	src = malloc(MAX_PIXELS*sizeof(*src)); 
	
	//Reading file
	if(read_ppm (argv[2], &xsize, &ysize, &colmax, (char *) src) != 0){
		MPI_Finalize();			
		exit(1);
	}
	if (colmax > 255) {
		fprintf(stderr, "Too large maximum color-component value\n");
		MPI_Finalize();
		exit(1);
	}
	printf("Has read the image, broadcasting info and generating coefficients\n");
	//Starting timer
	clock_gettime(CLOCK_REALTIME, &stime);	
	
	/*
	sem_init(&init_threads,0,0);
	for(i = 0; i<nt; i++)
	{
	    pthread_create(&thread_handle[ID], NULL, ASD_thread,(void*)ID);
	    sem_wait(&init_threads);
	    (*ID)++;
	}
	
	for(t=0; t<NUM_THREADS; i++) {
		pthread_join(thread_handle[i], NULL);
	}*/
}


/*

	int i;


   int radius;
    int xsize, ysize, colmax;
    pixel src[MAX_PIXELS];
    struct timespec stime, etime;
#define MAX_RAD 1000

    double w[MAX_RAD];

    /* Take care of the arguments 

    if (argc != 4) {
	fprintf(stderr, "Usage: %s radius infile outfile\n", argv[0]);
	exit(1);
    }
    radius = atoi(argv[1]);
    if((radius > MAX_RAD) || (radius < 1)) {
	fprintf(stderr, "Radius (%d) must be greater than zero and less then %d\n", radius, MAX_RAD);
	exit(1);
    }

    /* read file 
    if(read_ppm (argv[2], &xsize, &ysize, &colmax, (char *) src) != 0)
        exit(1);

    if (colmax > 255) {
	fprintf(stderr, "Too large maximum color-component value\n");
	exit(1);
    }

    printf("Has read the image, generating coefficients\n");

    /* filter 
    get_gauss_weights(radius, w);

    printf("Calling filter\n");

    clock_gettime(CLOCK_REALTIME, &stime);

    blurfilter(xsize, ysize, src, radius, w);

    clock_gettime(CLOCK_REALTIME, &etime);

    printf("Filtering took: %g secs\n", (etime.tv_sec  - stime.tv_sec) +
	   1e-9*(etime.tv_nsec  - stime.tv_nsec)) ;

    /* write result 
    printf("Writing output file\n");
    
    if(write_ppm (argv[3], xsize, ysize, (char *)src) != 0)
      exit(1);


    return(0);
}*/

/*
  File: thresfilter.h

  Declaration of pixel structure and thresfilter function.
    
 */
#ifndef _THRESFILTER_H_
#define _THRESFILTER_H_
/* NOTE: This structure must not be padded! */
typedef struct _pixel {
    unsigned char r,g,b;
} pixel;

struct thread_data{
	long threadID; 
	int xsize, ysize, started_threads;
	int* local_sum;
	char *src;
};

void calc_part(int* part_start, int* part_length, int me, int NUM_THREADS, int ysize);
//void thresfilter(const int xsize, const int ysize, pixel* src);

int threshfilter_part_1(int xsize, int part_start, int part_length, pixel* src);
void threshfilter_part_2(int xsize, int part_start, int part_length, pixel* src, int sum);
#endif

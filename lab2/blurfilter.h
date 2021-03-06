/*
  File: blurfilter.h

  Declaration of pixel structure and blurfilter function.
    
 */

#ifndef _BLURFILTER_H_
#define _BLURFILTER_H_

struct thread_data{
	long threadID; 
	int radius, xsize, ysize, started_threads;
	char *halfway;
	char *src;
};

typedef struct _pixel {
    unsigned char r,g,b;
} pixel;

void blurfilter_part_1(const int xsize, pixel* src, const int radius, const double *w, pixel* dst, const int startline, const int ylength);

void blurfilter_part_2(const int xsize, const int ysize, pixel* src, const int radius, const double *w, pixel* dst, const int startline, const int ylength);

void calc_part(int *part_start, int *part_length, int me,
				 int NUM_THREADS, int ysize);
int write_txt (const char* fname, const int radius, const double time, const int np);
#endif

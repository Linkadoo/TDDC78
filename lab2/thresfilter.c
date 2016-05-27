#include "thresfilter.h"

void calc_part(int* part_start, int* part_length, int me,
				int NUM_THREADS, int ysize){
	int i, rest, lines;
	lines = ysize / NUM_THREADS;
	rest = ysize % NUM_THREADS;
	if(rest > me){
		*part_start = me*(lines+1);
		*part_length = lines+1;
	}else if(rest == me){
		*part_start = me*(lines+1);
		*part_length = lines;
	}else{
		*part_start = me*lines+rest;
		*part_length = lines; 
	}
}

int threshfilter_part_1(int xsize, int part_start, int part_length, pixel* src){
#define uint unsigned int 

	uint sum, i, nump;
	nump = xsize * part_length;
	for(i=xsize*part_start, sum = 0; i < (nump + xsize*part_start); i++) {
		sum += (uint)src[i].r + (uint)src[i].g + (uint)src[i].b;
	}
    return sum;
}

void threshfilter_part_2(int xsize, int part_start, int part_length, pixel* src, float *sum,int me){
#define uint unsigned int 
	
	int i;
	uint psum,nump;
	nump = xsize * part_length;
	for(i = xsize*part_start; i < (xsize*part_start + nump); i++) {
    	psum = (uint)src[i].r + (uint)src[i].g + (uint)src[i].b;
		if(me == 1){
		//printf("i: %d, global_sum: %d, local_sum: %d\n", i,*sum, psum);
		}
    	if(*sum > psum) {
      		src[i].r = src[i].g = src[i].b = 0;
    	}else{
      		src[i].r = src[i].g = src[i].b = 255;
		}

	}
}

// x_danfa 1337supercomputer

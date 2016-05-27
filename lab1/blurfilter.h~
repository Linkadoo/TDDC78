/*
  File: blurfilter.h

  Declaration of pixel structure and blurfilter function.
    
 */

#ifndef _BLURFILTER_H_
#define _BLURFILTER_H_

typedef struct _pixel {
    unsigned char r,g,b;
} pixel;

void calclines(int buff[], int problemstart[], int problemlength[],
				int partstart[], int partlength[], const int me,
				const int np, const int ysize,int radius);

void distrwork(MPI_Comm com, pixel* local, pixel* src, const int xsize,
				int problemstart[], int problemlength[], int me, int np,
				MPI_Datatype _MPI_Pixel, MPI_Status status);
				
void blurfilter(const int xsize, const int ysize, pixel* src, const int radius,
				const double *w, const int ydiff, const int ylength, 
 				const int datalength);
int write_txt (const char* fname, const double time, const int np);
#endif

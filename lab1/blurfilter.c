/*
  File: blurfilter.c

  Implementation of blurXfilter function.
    
 */
#include <stdio.h>
#include <mpi.h>
#include "blurfilter.h"
#include "ppmio.h"

pixel* pix(pixel* image, const int xx, const int yy,
			const int xsize, const int datalength)
{
  register int off = xsize*yy + xx;

#ifdef DBG
  if(off >= datalength*xsize) {
    fprintf(stderr, "\n Terribly wrong: %d %d %d\n",xx,yy,xsize);
  }
#endif
  return (image + off);
}

//Calculates what work should be sent to which process.
void calclines(int buff[], int problemstart[], int problemlength[],
				 int partstart[], int partlength[], const int me, 
				 const int np, const int ysize, int radius){
	int i,rest,lines,tmp;
	if(me==0){
		//calculates how the lines in the picture are distributed
		for(i = 0; i < np; i++){
			problemlength[i] = 0;
			problemstart[i] = 0;
			partstart[i] = 0;
			partlength[i] = 0;
		}
		for(i = 0; i < np*4; i++){
			buff[i] = 0;
		}
		lines = ysize / np;
		rest = ysize % np;
		tmp = 0;
		//For all processes...
		for(i = 0; i<np;i++){
			//...identify where the i:th process's data start
			partstart[i] = tmp;
			buff[i+2*np] = tmp;
			//If a rest exist...
			if(rest != 0){
				//...add one line in addition to lines to i:th process.
				problemlength[i] = lines+1;
				buff[i+np]= lines+1;
				//Update the tmp-counter and decrease the rest...
				tmp = tmp + lines + 1;
				rest = rest - 1;
				//...and update the length of the i:th process's data part.
				partlength[i] = lines + 1;
				buff[i+3*np] = lines + 1;
			}
			// If a buffer doesn't exist: Do the same thing except adding
			// an extra line and decrease the rest.
			else{
				problemlength[i] = lines;
				buff[i+np] = lines;
				
				tmp = tmp + lines;
				
				partlength[i] = lines;
				buff[i+3*np] = lines;
			}
		}
		tmp = 0;
		//For all processes...
		for(i=0;i<np;i++){
			//...if the bluring radius is larger than the distance between the 
			// start of the picture and the upper part of the data for the i:th
			// process...
			if(partstart[i] < radius){
				//...set the problem start to the start of the picture... 
				problemstart[i] = 0;
				buff[i] = 0;
				//...and update the length of the problem.
				problemlength[i] = problemlength[i];// + partstart[i];
			}
			// Else set the start of the problem to the start of the 
			// i:th process's data start and update the length of the problem.
			else{
				problemstart[i] = partstart[i] - radius;
				buff[i] = partstart[i] - radius;
				problemlength[i] = problemlength[i] + radius;
				buff[i+np] = problemlength[i];  
			}
			//Do the same for the bottom part of the data parts.
			if(partstart[i] + partlength[i] > (ysize-radius)){
				problemlength[i] = problemlength[i] + ysize 
									- problemstart[i] - problemlength[i];
				buff[i+np] = problemlength[i] + ysize 
									- problemstart[i] - problemlength[i];
			}else{
				problemlength[i] = problemlength[i] + radius;
				buff[i+np] = problemlength[i];
			}
		}
	}
}

void blurfilter(const int xsize, const int ysize, pixel* src, const int radius, const double *w, const int ydiff, const int ylength, const int datalength){
	int x,y,x2,y2, wi;
	double r,g,b,n, wc;
	pixel* dst;    
    dst = malloc(datalength*xsize*sizeof(*dst));
	
	for (y=0; y<datalength; y++) {
		for (x=0; x<xsize; x++) {
			r = w[0] * (pix(src, x, y, xsize, datalength)->r);
			g = w[0] * pix(src, x, y, xsize, datalength)->g;
			b = w[0] * pix(src, x, y, xsize, datalength)->b;
			n = w[0];
			
			for ( wi=1; wi <= radius; wi++) {
				wc = w[wi];
				x2 = x - wi;
				if(x2 >= 0) {
		  			r += wc * pix(src, x2, y, xsize, datalength)->r;
		  			g += wc * pix(src, x2, y, xsize, datalength)->g;
		  			b += wc * pix(src, x2, y, xsize, datalength)->b;
		  			n += wc;
				}
				x2 = x + wi;
				if(x2 < xsize) {
		  			r += wc * pix(src, x2, y, xsize, datalength)->r;
		  			g += wc * pix(src, x2, y, xsize, datalength)->g;
		  			b += wc * pix(src, x2, y, xsize, datalength)->b;
		  			n += wc;
				}
	  		}
	  		pix(dst,x,y, xsize, datalength)->r = r/n;
	  		pix(dst,x,y, xsize, datalength)->g = g/n;
	  		pix(dst,x,y, xsize, datalength)->b = b/n;
    	}
  	}
  	for (y=ydiff; y<ylength+ydiff; y++) {
    	for (x=0; x<xsize; x++) {
      		r = w[0] * pix(dst, x, y, xsize, datalength)->r;
      		g = w[0] * pix(dst, x, y, xsize, datalength)->g;
      		b = w[0] * pix(dst, x, y, xsize, datalength)->b;
      		n = w[0];
      		for ( wi=1; wi <= radius; wi++) {
				wc = w[wi];
				y2 = y - wi;
				if(y2 >= 0) {
		  			r += wc * pix(dst, x, y2, xsize, datalength)->r;
		  			g += wc * pix(dst, x, y2, xsize, datalength)->g;
		  			b += wc * pix(dst, x, y2, xsize, datalength)->b;
		  			n += wc;
				}
				y2 = y + wi;
				if(y2 < datalength) {
					r += wc * pix(dst, x, y2, xsize, datalength)->r;
		  			g += wc * pix(dst, x, y2, xsize, datalength)->g;
		  			b += wc * pix(dst, x, y2, xsize, datalength)->b;
		  			n += wc;
				}
      		}
      		pix(src,x,y, xsize, datalength)->r = r/n;
      		pix(src,x,y, xsize, datalength)->g = g/n;
      		pix(src,x,y, xsize, datalength)->b = b/n;
		}
	}
}

int write_txt (const char* fname, const int radius, const double time, const int np) {

  FILE * fp;
  int errno = 0;

  if (fname == NULL) fname = "\0";
  fp = fopen (fname, "a");
  if (fp == NULL) {
    fprintf (stderr, "write_txt failed to open %s: %s\n", fname,strerror (errno));
    return 1;
  }
  
  fprintf(fp, "%d %d %f \n", np, radius, time);
  if (fclose (fp) == EOF) {
    perror ("Close failed");
    return 3;
  }
  return 0;
}


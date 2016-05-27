#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ppmio.h"
#include "thresfilter.h"
#include <mpi.h>
#include <math.h>
#include <stdlib.h>

void calculate_local_problem_size(const int x,const int y,const int np, int problemstart[], int problemlength[]);

int write_txt (const char* fname, const double time, const int np);

int main (int argc, char ** argv) {
    int xsize, ysize, imagecounter, colmax;
    pixel* src;
    pixel* local_src;
    unsigned int local_sum;
    unsigned int global_sum;
	int np, me, buff[2];
    MPI_Init( &argc, &argv );
    MPI_Comm com = MPI_COMM_WORLD;
    MPI_Comm_size( com, &np );
    MPI_Comm_rank( com, &me );
	MPI_Status status;
	int problemstart[np];
	int problemlength[np];

    int sendcount[np];
    int displs[np];
    int i;
    double starttime, endtime; 
	char* imagename;
	char* filename;
	char* outfile; 
	
    //Create mpi structure
    pixel item;

	MPI_Datatype pixel_mpi;
	int block_lengths [] = {1 , 1, 1};
	MPI_Datatype block_types [] = {MPI_UNSIGNED_CHAR,MPI_UNSIGNED_CHAR,MPI_UNSIGNED_CHAR};
	MPI_Aint start, displ[3];

	MPI_Address(&item, &start);
	MPI_Address(&item.r, &displ[0]);
	MPI_Address(&item.g, &displ[1]);
	MPI_Address(&item.b, &displ[2]);

	displ[0] -= start;
	displ[1] -= start;
	displ[2] -= start;
	MPI_Type_struct(3, block_lengths, displ, block_types, &pixel_mpi);

	MPI_Type_commit( &pixel_mpi);
	//STOP Create mpi structure
	
    /* Take care of the arguments */
	for(imagecounter = 0; imagecounter<4; imagecounter++)
	{
		switch(imagecounter){
			case 0: 
			imagename = "im1.ppm";
			filename = "im1thres.txt";
			outfile = "test1.ppm";
			break;
			case 1: 
			imagename = "im2.ppm";
			filename = "im2thres.txt";
			outfile = "test2.ppm";
			break;
			case 2: 
			imagename = "im3.ppm";
			filename = "im3thres.txt";
			outfile = "test3.ppm";
			break;
			case 3: 
			imagename = "im4.ppm";
			filename = "im4thres.txt";
			outfile = "test4.ppm";
			break;
		}
		if (me == 0) { // read image at process 0:
			src = malloc(MAX_PIXELS*sizeof(*src));

			/* read file */
			if(read_ppm (imagename, &xsize, &ysize, &colmax, (char *) src) != 0)
				exit(1);

			if (colmax > 255) {
				fprintf(stderr, "Too large maximum color-component value\n");
				exit(1);
			}

			printf("Has read the image, calling filter\n");
		
			//read problem size into buf
			buff[0]=xsize;
			buff[1]=ysize;
		}
	
		// Single-Broadcast of size from P0 to P1...P(np-1):
		MPI_Bcast( buff, 2, MPI_INT, 0, com );
		// Extract problem size from buff; allocate space:
	
		if(me != 0){
			xsize=buff[0];
			ysize=buff[1];
		}
	
		//Calculates lproblem, size and lines
		calculate_local_problem_size(xsize, ysize, np,problemstart,problemlength);

		local_src = malloc(problemlength[me]*xsize*sizeof(*local_src));
		
		for(i=0; i<np; i++){
			displs[i] = problemstart[i]*xsize;
			sendcount[i] = problemlength[i]*xsize;
		}
		
		MPI_Scatterv(src, sendcount, displs, pixel_mpi, local_src, problemlength[me]*xsize, pixel_mpi , 0, com);
	
		starttime = MPI_Wtime(); 

		local_sum = localthreshold(xsize, problemlength[me], local_src);
		
		MPI_Allreduce(&local_sum, &global_sum, 1, MPI_UNSIGNED, MPI_SUM, com);
		global_sum /= np;
		
		thresfilter(xsize, problemlength[me], local_src, global_sum);
		
		int recvcounts[np];
		int displs[np];
		for (i=0; i<np; i++)
		{
			recvcounts[i] = problemlength[i]*xsize;
			displs[i] = problemstart[i]*xsize;
		}
		/*if(me==0){
		for (i=0; i<np; i++)
		{
			printf("recvcounts[%d]: [%d]\n",i,recvcounts[i]);
			printf("displs[%d]: [%d]\n",i,displs[i]);
		}	}*/
		//Get data from other processes
		MPI_Gatherv(local_src, problemlength[me]*xsize, pixel_mpi, src, recvcounts, displs, pixel_mpi, 0, com);
	   endtime = MPI_Wtime(); 
	   
	   
		if(me == 0){
			printf("Filtering took: %f secs\n", (endtime-starttime)) ;

			/* write result */
			printf("Writing output file\n");
			if(write_ppm (outfile, xsize, ysize, (char *)src) != 0)
      			exit(1);
			if(write_txt (filename, (endtime-starttime), np ) != 0)
		  		exit(1);
		  	
		}
		
	}
	MPI_Finalize();
	return 0;
}


void calculate_local_problem_size(const int x,const int y,const int np, int problemstart[], int problemlength[]){
	//int lproblem[][2]){
	int linesize, rest;
	int lysize[np];
	linesize = floor(y/np);
	rest = y%np;
	int i;
	int k;
	for (i=0; i<np; i++){
		if(i<rest){
			lysize[i]=linesize+1;
		}else{
			lysize[i]=linesize;
		}
	}
	int tmp;
	for (i=0; i<np; i++){
		tmp=0;
		for (k=0; k<i; k++){
			tmp += lysize[k];
		}
		
		problemstart[i]=tmp;
		problemlength[i]=lysize[i];
	}
}

int write_txt (const char* fname, const double time, const int np) {

  FILE * fp;
  int errno = 0;

  if (fname == NULL) fname = "\0";
  	fp = fopen (fname, "a");
  if (fp == NULL) {
    fprintf (stderr, "write_txt failed to open %s: %s\n", fname,strerror (errno));
    return 1;
  }
  
  fprintf(fp, "%d %f \n", np, time);
  if (fclose (fp) == EOF) {
    perror ("Close failed");
    return 3;
  }
  return 0;
}


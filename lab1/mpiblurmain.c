#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <math.h>
#include "ppmio.h"
#include "blurfilter.h"
#include "gaussw.h"


int main (int argc, char ** argv) {
	
	// ----------- Various non-MPI variables ---------------
	int xsize, ysize, me, np, radius, colmax, imagecounter;
	double starttime, endtime; 
	char *imagename,*filename,*outfile;
	pixel item;
	pixel *local,*src;
	struct timespec stime, etime;
	#define MAX_RAD 3000
	double w[MAX_RAD];
	
	
	//------------ Initiation and declatarion of MPI stuff  -------------
	MPI_Init(&argc,&argv);
	MPI_Status status;
	MPI_Comm com = MPI_COMM_WORLD;
	MPI_Comm_rank(com,&me);
	MPI_Comm_size(com,&np);
	int buff[np*4],problemstart[np],problemlength[np],
		partstart[np],partlength[np];
	

	MPI_Datatype _MPI_Pixel;
	int block_lengths [] = {1 , 1, 1};
	MPI_Datatype block_types [] = {MPI_UNSIGNED_CHAR,MPI_UNSIGNED_CHAR,
									MPI_UNSIGNED_CHAR};
	MPI_Aint start, displ[3];

	MPI_Address(&item, &start);
	MPI_Address(&item.r, &displ[0]);
	MPI_Address(&item.g, &displ[1]);
	MPI_Address(&item.b, &displ[2]);

	displ[0] -= start;
	displ[1] -= start;
	displ[2] -= start;
	MPI_Type_struct(3, block_lengths, displ, block_types, &_MPI_Pixel);
	MPI_Type_commit( &_MPI_Pixel);
	//----------- End of MPI init --------------
	
	/* Take care of the arguments */
	if(argc != 2){
		if(me==0) fprintf(stderr, "Usage: %s write_images(0 or 1)\n", argv[0]);
		MPI_Finalize();
		exit(1);
	}
	//radius = atoi(argv[1]);	
	/*if((radius > MAX_RAD) || (radius < 1)){
		if(me == 0){
			fprintf(stderr, "Radius (%d) must be greater than zero and less than %d\n", radius, MAX_RAD);	
		}
		MPI_Finalize();
		exit(1);
	}*/
	
	//Allocating space for the picture
	src = malloc(MAX_PIXELS*sizeof(*src)); 
	//Getting gauss weights
	get_gauss_weights(radius, w);
	//For each picture
	for(imagecounter = 0; imagecounter<4; imagecounter++){
	     switch(imagecounter){
		    case 0: 
		    imagename = "im1.ppm";
			filename = "im1blur.txt";
			outfile = "test1.ppm";
			break;
			case 1: 
			imagename = "im2.ppm";
			filename = "im2blur.txt";
			outfile = "test2.ppm";
			break;
			case 2: 
			imagename = "im3.ppm";
			filename = "im3blur.txt";
			outfile = "test3.ppm";
			break;
			case 3: 
			imagename = "im4.ppm";
			filename = "im4blur.txt";
			outfile = "test4.ppm";
			break;
		}
		int ii;
		for(ii=0;ii<10;ii++){ 
			radius = pow(2,ii);
			if(me == 0){
				//Reading file
				if(read_ppm (imagename, &xsize, &ysize, &colmax, (char *) src) != 0){
					MPI_Finalize();			
					exit(1);
				}
				if (colmax > 255) {
					fprintf(stderr, "Too large maximum color-component value\n");
					MPI_Finalize();
					exit(1);
				}
				printf("Has read the image.\n");
				printf("Starting timer and broadcasting info\n");
			
				//Starting timer
				starttime = MPI_Wtime(); 
		
				//Broadcasting info		
				buff[0] = radius;		
				buff[1] = xsize;
				buff[2] = ysize;
				buff[3] = colmax;
			}
			MPI_Bcast(buff,4,MPI_INT,0,com);
			if(me != 0){
				radius = buff[0];
				xsize = buff[1];
				ysize = buff[2];
				colmax = buff[3];
			}
			if(me == 0){
				calclines(buff,problemstart,problemlength,partstart,partlength,
							me,np,ysize,radius);
			}
			MPI_Bcast(buff,4*np,MPI_INT,0,com);
			if(me != 0){
				int i;
				for(i=0; i < np; i++){
					problemstart[i] = buff[i];
					problemlength[i] = buff[i+np];
					partstart[i] = buff[i+np*2];
					partlength[i] = buff[i+np*3];
				}
			}
		
			//Allocate memory
			local = malloc(3*sizeof(char)*xsize*problemlength[me]);
			//Distribute work
			if(me == 0){
				int i;
				for(i=1;i<np;i++){ 	
					MPI_Send(&(src[problemstart[i]*xsize]), xsize*problemlength[i], 
								_MPI_Pixel, i, 10, com);
	
				}
				memcpy(local, src, problemlength[0]*xsize*3);
			}else{	
				MPI_Recv(local, xsize*problemlength[me], _MPI_Pixel, 0, 10, com, &status);
				//printf( "From P%d, error %d\n", status.MPI_SOURCE, status.MPI_ERROR );
			}
			int ydiff = partstart[me] - problemstart[me];
			blurfilter(xsize, ysize, local, radius, w, ydiff,
						partlength[me], problemlength[me]);
			MPI_Gather(&local[ydiff*xsize], partlength[me]*xsize, _MPI_Pixel, src,
						partlength[me]*xsize, _MPI_Pixel, 0, com);	
			free(local);
			if(me == 0){
				endtime = MPI_Wtime(); 
				fprintf(stderr,"Filtering took: %g secs\n",(endtime-starttime));
		
				/* write result */
				fprintf(stderr,"Writing output file\n");
				if(argv[1] == 1){
					if(write_ppm (outfile, xsize, ysize, (char *)src) != 0){	
						exit(1);      
					}
				}
				if(write_txt (filename, (endtime-starttime), np ) != 0)
			  		exit(1);
			  	}
			}
		}
	}
	MPI_Finalize(); 
}

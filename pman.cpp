#include <mpi.h>
#include "mandelbrot.h"
#include <stdlib.h>
#include <iostream>
#include <unistd.h> 
#include "display.h"
#include <X11/Xlib.h>

void master(int window_width,int window_height, int partitions, MPI_Datatype pixeltype){
	GraphicsContainer gCont = getNewDisplay(window_width,window_height);
	MPI_Status status;
	long WHITE = WhitePixel(gCont.display, gCont.screen);
	long BLACK = BlackPixel(gCont.display, gCont.screen);
	long COLOR = 0;
	for(int i = 0; i < partitions; i++){
		MPI_Probe(MPI_ANY_SOURCE,12,MPI_COMM_WORLD,&status);
		int sectionSize;
		MPI_Get_count(&status, MPI_INT,&sectionSize);
		MandelbrotPixel pixels[sectionSize];
		MPI_Recv(pixels,sectionSize,pixeltype, MPI_ANY_SOURCE, 12, MPI_COMM_WORLD,&status);	

		std::cout << "FROM: " << pixels[0].y;
		std::cout << "  TO: " << pixels[sectionSize].y << "\n";
		
		for(int j = 0; j < sectionSize; j++){
			if(pixels[j].distance == 500)
				COLOR = BLACK;
			else
				COLOR = WHITE-(500-pixels[j].distance)*(WHITE-BLACK)/500;
			
			XSetForeground(gCont.display,gCont.gc,COLOR);
			XDrawPoint(gCont.display,gCont.window,gCont.gc,pixels[j].x,pixels[j].y);
				
			//std::cout << "x:"<< pixels[j].x << " y:" << pixels[j].y << " = " << pixels[j].distance;
			//std::cout << "\n";
		}
		XFlush(gCont.display);
	}
	//MPI_Send(NULL,0,MPI_INT,1,0,MPI_COMM_WORLD);	
	sleep(4);
	//MPI_Finalize();
}



void delegator(int height, int width, int partitions){
	
	int REQUEST_TAG = 10;
	int SECTION_TAG = 11;
	MPI_Status status;
	int cursor = 0;
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD,&world_size);
	int step = height/partitions;
	for(int i=0; i<partitions + world_size -2; i++){
		int node_id;
		//Recieving request for new section
		MPI_Recv(&node_id,1,MPI_INT,MPI_ANY_SOURCE,REQUEST_TAG,MPI_COMM_WORLD,&status);
		//Defining new section to solve
		int section[4];
		section[0] = 0; //Start X
		section[1] = cursor; //Start Y
		section[2] = width; //Stop X
		
		if(cursor >= height){
			MPI_Send(section,4,MPI_INT,node_id,SECTION_TAG,MPI_COMM_WORLD);
			continue;
		}
		if(height < cursor+step){
			section[3] = height;
		}
		else{
			section[3] = cursor+step;
			cursor = cursor+step-1;
		}
		//Sending new section to the node that requested it
		MPI_Send(section,4,MPI_INT,node_id,SECTION_TAG, MPI_COMM_WORLD);
	}
}

void slave(double x, double y, double s, MPI_Datatype pixeltype){
	double minX, minY, step;
	minX = x;
	minY = y;
	step = s;
	int REQUEST_TAG = 10;
	int SECTION_TAG = 11;
	int DRAW_TAG = 12;
	int rank;
	int done = 0;
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Status status;

//	MandelbrotPixel pixels[10];
//	for(int i = 0; i < 10; i++){
//		pixels[i].x = 0;
//		pixels[i].y = 0;
//		pixels[i].distance = 0;
//	}
	while(done == 0){
	int section[4];
	MPI_Send(&rank,1,MPI_INT,1,REQUEST_TAG,MPI_COMM_WORLD);
	MPI_Recv(&section,4,MPI_INT,MPI_ANY_SOURCE,SECTION_TAG, MPI_COMM_WORLD,&status);
	int sectionSize = (section[0]-section[2])*(section[1]-section[3]);
	MandelbrotPixel pixels[sectionSize];
	if(sectionSize == 0){
		done = 1;
		continue;
	}
	int arrayCursor = 0;
	for(int i=section[0]; i<section[2]; i++){
		for(int j=section[1]; j<section[3];j++){
			pixels[arrayCursor].x = i;
			pixels[arrayCursor].y = j;
			pixels[arrayCursor].distance = getDistance((minX + (step * i)),(minY + (step * j)));
			arrayCursor++;
		}
	}



	MPI_Send(pixels,sectionSize,pixeltype, 0,DRAW_TAG,MPI_COMM_WORLD);
	}
//	for(int i = 0; i < 10; i++){
//		getDistance(i,i,result);
//		std::cout << result[i].distance << "\n";
//	}
//sleep(2);
}

int main(){
	
	MPI_Datatype pixeltype, dataType[2];
	MPI_Aint offset[2], dataTypeSize;
	int blockCount[2];	

	MPI_Init(NULL,NULL);
	
	//Adding two double variables to the datatype
	offset[0] = 0;
	dataType[0] = MPI_DOUBLE;
	blockCount[0] = 2;
	
	//Adding one int variable to the datatype
	MPI_Type_extent(MPI_DOUBLE, &dataTypeSize);
	offset[1] = 2 * dataTypeSize;
	dataType[1] = MPI_INT;
	blockCount[1] = 1;
	
	//Defining variable
	MPI_Type_struct(2, blockCount, offset, dataType, &pixeltype);
	MPI_Type_commit(&pixeltype);
	
	//Size of window
	int window_height,window_width,partitions;
	//Size of fraction
	double midX,midY,step;
	//Setting initial zoom of fraction and standard window size
	window_width = 1824;
	window_height = 984;
	midX = -0.088;
	midY = 0.654;
	step = 0.00001;
	partitions = 50;
		
	int world_size,rank;
	MPI_Comm_size(MPI_COMM_WORLD,&world_size);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	
	
	if(world_size < 3){
		if(rank == 0)
			std::cout << "World size must be at least 3\n";
//		return 1;
	}
	
	if(rank == 0){
		//Node is master
		master(window_width, window_height,partitions,pixeltype);
	}
	else if(rank == 1){
		//Node is delegator
		delegator(window_height,window_width,partitions);
	}
	else{
		//Node is slave
		slave(midX-((window_width/2)*step), midY-((window_height/2)*step),step, pixeltype);
	}
				
//	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
}

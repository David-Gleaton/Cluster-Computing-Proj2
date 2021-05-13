#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

#define WHITE    "15 15 15 "
#define RED      "15 00 00 "
#define ORANGE   "15 05 00 "
#define YELLOW   "15 10 00 "
#define LTGREEN  "00 13 00 "
#define GREEN    "05 10 00 "
#define LTBLUE   "00 05 10 "
#define BLUE     "00 00 10 "
#define DARKTEAL "00 05 05 "
#define BROWN    "03 03 00 "
#define BLACK    "00 00 00 "


int MAX_SIZE = 1000;


void PrintGrid(float grid[][MAX_SIZE], int xsource, int ysource);


int main(int argc, char *argv[]){
	
/* David Gleaton
 * HeatMap/Assignment2
 * All Edges need to be fixed at 20 C
 * Fireplace
 * 	40% of the room width and placed at the 'top' of the room
 *	The temp of the fireplace is fixed at 300 C
 * Using bitmap format, size of grid is 1000 x 1000
 *
 * PARALLELIZED CODE
 *
 * Idea/Pseudo Code
 * 	
 * 	loop for how many iter{
 *		Slave
 *			Take old section, calculated new
 *			Send new calculated to master
 * 			Maybe send one column to the next process
 * 		Master
 * 			Calculate its own section
 * 			Take Slave heatmaps and figure out borders
 * 			Save old section for new iter
 *	}
 * Notes Personal
 * 	Cols/Rows that need to be fixed
 * 		Cols = 0, set to 20
 * 		Cols = MAX_SIZE set to 20
 * 		Rows = 0, set to 20 EXCEPT Fireplace
 * 		Rows = MAX_SIZE set to 20
 */



        int iter = atoi(argv[1]);
	int FireLen = MAX_SIZE * .40;
	int lowerFire = MAX_SIZE/2 - FireLen/2;
	int upperFire = MAX_SIZE/2 + FireLen/2;
	
	//Declare pid = rank and numprocess = size
	int pid, numprocess;

	//Establish MPI
        MPI_Status status;

        MPI_Init(&argc, &argv);


        MPI_Comm_rank(MPI_COMM_WORLD, &pid);
        MPI_Comm_size(MPI_COMM_WORLD, &numprocess);


	int procRange = MAX_SIZE/numprocess;

	//Clock to see how fast
	float starttime;
	if(pid == 0){
		starttime = MPI_Wtime();
	}

	float template[MAX_SIZE][MAX_SIZE];

	//Build out first iter
		for(int i = 0; i < MAX_SIZE; i++)
			for(int j = 0; j < MAX_SIZE; j++){
			if(i == 0 && j >= lowerFire && j < upperFire )
				template[i][j] = 300;
			else if(i == 0)
				template[i][j] = 20;	
			else if(i == MAX_SIZE - 1)
				template[i][j] = 20;
			else if(j == 0)
				template[i][j] = 20;
			else if(j == MAX_SIZE -1)
				template[i][j] = 20;
			else{
				template[i][j] = 0;
			}
			}

		int index = pid*procRange;
		int endindex = index + procRange -1;
		float newheat[MAX_SIZE][procRange];
		float oldheat[MAX_SIZE][procRange];
		float gotcolsbelow[MAX_SIZE], gotcolsabove[MAX_SIZE];
		float send[MAX_SIZE], gotcha[MAX_SIZE];


			//Populate small arrays with template array
			for(int j = 0, k = index; j< procRange; j++, k++)
				for(int i = 0; i < MAX_SIZE; i++){
					oldheat[i][j] = template[i][k];
					newheat[i][j] = template[i][k];				
				}




		//Iterations
		for(int it = 0; it < iter; it++){
			//Exchange of Ghost points
			//oth process
			float sending, getting;

			if(pid == 0){
				for(int i = 0; i< MAX_SIZE; i++)
					send[i] = oldheat[i][endindex];

				gotcolsbelow[0] = -1;
	


				MPI_Send(send, MAX_SIZE, MPI_FLOAT, pid+1,0, MPI_COMM_WORLD);
				MPI_Recv(gotcolsabove, MAX_SIZE, MPI_FLOAT, pid+1, 0, MPI_COMM_WORLD, &status);

			gotcolsbelow[0] = -1;

			//Middle processes
			}else if(pid < numprocess -1){

			



				for(int i = 0; i< MAX_SIZE; i++)
					send[i] = oldheat[i][index];

				MPI_Send(send, MAX_SIZE, MPI_FLOAT, pid-1,0, MPI_COMM_WORLD);
				MPI_Recv(gotcolsbelow, MAX_SIZE, MPI_FLOAT, pid-1, 0, MPI_COMM_WORLD, &status);


				for(int i = 0; i< MAX_SIZE; i++)
					send[i] = oldheat[i][endindex];

				MPI_Send(send, MAX_SIZE, MPI_FLOAT, pid+1,0, MPI_COMM_WORLD);
				MPI_Recv(gotcolsabove, MAX_SIZE, MPI_FLOAT, pid+1, 0, MPI_COMM_WORLD, &status);


			//Last process
			}else if(pid == numprocess -1){
				for(int i = 0; i< MAX_SIZE; i++)
					send[i] = oldheat[i][index];

				MPI_Send(send, MAX_SIZE, MPI_FLOAT, pid-1,0, MPI_COMM_WORLD);
				MPI_Recv(gotcolsbelow, MAX_SIZE, MPI_FLOAT, pid-1, 0, MPI_COMM_WORLD, &status);



				gotcolsabove[0] = -1;





			}
			
			
//-----------------Beginning of Copy new to old


	for(int j = 0; j < procRange; j++){
		for(int i = 0; i < MAX_SIZE; i++){	
			oldheat[i][j] = newheat[i][j];

		}

	}
//------------------help -------- END COPY NEW TO OLD
			






//----------------BEGIN CALCLATE NEW
//Nth process calculation

	if(gotcolsabove[0] < 0){
		//Since we are dividing one vertical sections of the gird
		//Only limiter on j, <= since we want it to calculate the last column as well

		for(int j = 0; j < procRange; j++)
			for(int i = 0; i < MAX_SIZE; i++){
				if(i == 0 && newheat[i][j] == 300 )
					newheat[i][j] = 300;
				else if(i == 0)
					newheat[i][j] = 20;	
				else if(i == MAX_SIZE - 1)
					newheat[i][j] = 20;
				else if(j == procRange -1)
					newheat[i][j] = 20;



					//Use gotcolsbelow
					else if(j == 0){
						newheat[i][j] = 0.25*(oldheat[i-1][j]+oldheat[i+1][j]+gotcolsbelow[i]+oldheat[i][j+1]);
					//Use normal calculation
					}else{
						newheat[i][j] = 0.25*(oldheat[i-1][j]+oldheat[i+1][j]+oldheat[i][j-1]+oldheat[i][j+1]);
					}







			}
			





	}

	//0th process calculation
	else if(gotcolsbelow[0] < 0){
		//Since we are dividing one vertical sections of the gird
		//Only limiter on j, <= since we want it to calculate the last column as well

		for(int i = 1; i < MAX_SIZE; i++)
			for(int j = 1; j < procRange; j++){

				if(i == 0 && newheat[i][j] == 300)
					newheat[i][j] = 300;
				else if(i == 0)
					newheat[i][j] = 20;	
				else if(i == MAX_SIZE - 1)
					newheat[i][j] = 20;
				else if(j == 0)
					newheat[i][j] = 20;

				//Use gotcolsabove
				else if(j == procRange-1){
						newheat[i][j] = 0.25*(oldheat[i-1][j]+oldheat[i+1][j]+oldheat[i][j-1]+gotcolsabove[i]);
				//Use normal calculation
				}else{
						newheat[i][j] = 0.25*(oldheat[i-1][j]+oldheat[i+1][j]+oldheat[i][j-1]+oldheat[i][j+1]);
				

			
					}







			}
		}
	//Middle Process calculation
	//TODO enfore local rules
	else{

		for(int j = 0; j < procRange; j++)
			for(int i = 0; i < MAX_SIZE; i++){

				if(i == 0 && newheat[i][j] == 300 )
					newheat[i][j] = 300;
				else if(i == 0)
					newheat[i][j] = 20;	
				else if(i == MAX_SIZE - 1)
					newheat[i][j] = 20;

					//Use gotcolsbelow
				else if(j == 0){
						newheat[i][j] = 0.25*(oldheat[i-1][j]+oldheat[i+1][j]+gotcolsbelow[i]+oldheat[i][j+1]);


					}
					//Use gotcolsabove
					else if(j == procRange -1){
						newheat[i][j] = 0.25*(oldheat[i-1][j]+oldheat[i+1][j]+oldheat[i][j-1]+gotcolsabove[i]);


					//Use normal calculation
					}else{
						newheat[i][j] = 0.25*(oldheat[i-1][j]+oldheat[i+1][j]+oldheat[i][j-1]+oldheat[i][j+1]);

					}







			}
			

	}



//------------------END CALCULATENEW---------------
		}
		MPI_Barrier(MPI_COMM_WORLD);



		if(pid !=0 ){

			float go;
			for(int j = 0; j < procRange; j++){
				for(int i = 0; i< MAX_SIZE; i++){

					go = newheat[i][j];

					MPI_Send(&go, 1, MPI_FLOAT, 0,0, MPI_COMM_WORLD);
				}

	
			}
	
		}else{





			float got;

			float temp[MAX_SIZE][MAX_SIZE];
			for(int i = 0; i < MAX_SIZE; i++)
				for(int j =0; j < MAX_SIZE; j++ )
					temp[i][j] = 0;
			for(int i = 0; i< MAX_SIZE; i++){
				for(int j = 0; j < procRange; j++)
					temp[i][j] = newheat[i][j];	
			}

			for(int a = 1; a < numprocess; a++){

				for(int j = 0; j < procRange; j++){
					for(int b = a*procRange +j, c = 0; c < MAX_SIZE ; c++){

					MPI_Recv(&got, 1, MPI_FLOAT, a, 0, MPI_COMM_WORLD, &status);		
							
					temp[c][b] = got;

					}
				 }
					
			    }


			PrintGrid(temp, 0, 0);
		}








	MPI_Finalize();


	return 0;
}



void PrintGrid(float grid[][MAX_SIZE], int xsource, int ysource){
	FILE * fp;

	int numcolors = 10;
	int color;

	//The Image will be MAX_SIZE by MAX_SIZE
	int linelen = MAX_SIZE;
	int numlines = MAX_SIZE;
	int i, j;

	fp = fopen("c.pnm", "w");

	fprintf(fp, "P3\n%d %d\n15\n", linelen, numlines);

	for(i = 0; i < numlines; i++)
		for(j = 0; j < linelen; j++){
			if(grid[i][j] > 250)
				fprintf(fp, "%s ", RED);
			else if(grid[i][j] <= 250 && grid[i][j] >180)
				fprintf(fp, "%s ", ORANGE);
			else if(grid[i][j] <= 180  && grid[i][j] > 120 )
				fprintf(fp, "%s ", YELLOW);
			else if(grid[i][j] <=120 && grid[i][j] > 80  )	
				fprintf(fp, "%s ", LTGREEN);
			else if(grid[i][j] <=80 && grid[i][j] > 60 )
				fprintf(fp, "%s ", GREEN);
			else if(grid[i][j] <=60 && grid[i][j] >50  )
				fprintf(fp, "%s ", LTBLUE);
			else if(grid[i][j] <=50 && grid[i][j] >40  )
				fprintf(fp, "%s ", BLUE);
			else if(grid[i][j] <=40 && grid[i][j] >30  )
				fprintf(fp, "%s ", DARKTEAL);
			else if(grid[i][j] <=30 && grid[i][j] > 20  )
				fprintf(fp, "%s ", BROWN);
			else
				fprintf(fp, "%s ", BLACK);
		}
}






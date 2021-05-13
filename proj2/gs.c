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

void CopyNewToOld(float new[][MAX_SIZE], float old[][MAX_SIZE]);
void CalculateNew(float new[][MAX_SIZE], float old[][MAX_SIZE], int start, int end, int lowerFire, int upperFire, int gotcolabove[], int gotcolbelow[]);
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


	float newheat[MAX_SIZE][MAX_SIZE];
	float oldheat[MAX_SIZE][MAX_SIZE];

//	int iter = 1;
        int iter = atoi(argv[1]);
	int FireLen = MAX_SIZE * .40;
	int lowerFire = MAX_SIZE/2 - FireLen/2;
	int upperFire = MAX_SIZE/2 + FireLen/2;
/*
	printf("FireLen = %d\n", FireLen);
	printf("lowerFire = %d\n", lowerFire);
	printf("upperFire = %d\n", upperFire);
*/

	//Build out first iter
	for(int i = 0; i < MAX_SIZE; i++)
		for(int j = 0; j < MAX_SIZE; j++){
			if(i == 0 && j >= lowerFire && j < upperFire )
				newheat[i][j] = 300;
			else if(i == 0)
				newheat[i][j] = 20;	
			else if(i == MAX_SIZE - 1)
				newheat[i][j] = 20;
			else if(j == 0)
				newheat[i][j] = 20;
			else if(j == MAX_SIZE -1)
				newheat[i][j] = 20;
			else{
				newheat[i][j] = 0;
			}
		}

	
	//Declare pid = rank and numprocess = size
	int pid, numprocess;

	//Establish MPI
        MPI_Status status;

        MPI_Init(&argc, &argv);


        MPI_Comm_rank(MPI_COMM_WORLD, &pid);
        MPI_Comm_size(MPI_COMM_WORLD, &numprocess);

//	printf("We have %d Processes\n", numprocess);
	int procRange = MAX_SIZE/numprocess;
//	printf("This is our division of labor = %d\n", procRange);

/*
 * Thoughts
 * 	So, each process will send and recieve one entire column for calculations, except the 0th and Nth process
 * 	No need for master-slave processes, all are equal here
 * 		Actually, may need a master to make sure all processes have finished before heading out to next iter
 * 	Each process will take care of its nth range, based on its pid/rank
 * 	At the beginning of each calculation, a process sends a column to the process below, one to the process above, and 
 * 	recieves one in turn. Then, they calculate.
 *
 *
 *
 *
 */
	//Clock to see how fast
	float starttime;
	if(pid == 0){
		starttime = MPI_Wtime();
	}
	//For loop for multiple iterations
	for(int i = 0; i < iter; i++){
		int index = pid*procRange;
		//Master only sends column up
		if(pid == 0){
			CopyNewToOld(newheat, oldheat);
			//gotcol stores recieved points from pid up one
			float gotcolabove[MAX_SIZE], temp[1];
			temp[0] = -1;
			//TODO work on send
			//Send col to pid above
			float send[MAX_SIZE];
			for(int i = 0; i < MAX_SIZE; i++){
				send[i] = oldheat[i][index+procRange-1];
			}
			MPI_Isend(&send, MAX_SIZE, MPI_FLOAT, pid+1, 0, MPI_COMM_WORLD, &request);


			//Get col from pid above
			for(int i = 0; i < MAX_SIZE; i++){
				MPI_Recv(&gotcolabove, MAX_SIZE, MPI_FLOAT, pid+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
			//TODO Cal
			//Once all columns are traded, then calculate
			//Have the process start at their sector
			int start = pid * procRange;
			//Have the process end at their sector
			int end = (pid * procRange) + procRange - 1;

			//Calculate new heatgrid
			CalculateNew(newheat, oldheat, start, end, lowerFire, upperFire, gotcolabove, temp);




			//Get all messages from other pids to move on
			int buf;
			for(int i = 1; i < numprocess; i++){
				MPI_Recv(&buf, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
//printf("I am %d and I am doing section %d to %d\n", pid, pid*procRange, (pid * procRange) +procRange-1);
//printf("I am %d and I am sending %d up\n", pid, index+procRange-1);


		}


		//Last process sends only column down
		else if(pid == numprocess-1){
			int gotcolbelow[MAX_SIZE], temp[1];
			temp[0] = -1;
			int send, got;
			//Get column from pid below
			for(int i = 0; i< MAX_SIZE; i++){
				MPI_Recv(&got, 1, MPI_INT, pid-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				gotcolbelow[i] = got;
			}

			//Send col to pid below
			for(int i = 0; i < MAX_SIZE;i++){
				send = oldheat[i][index];
				MPI_Send(&send, 1, MPI_INT, pid-1, 0, MPI_COMM_WORLD);
			}	
			//TODO Cal
			//Have the process start at their sector
			int start = pid * procRange;
			//Have the process end at their sector
			int end = (pid * procRange) + procRange - 1;

			//Calculate new heatgrid
			CalculateNew(newheat, oldheat, start, end, lowerFire, upperFire, temp, gotcolbelow);



			//Send to Master to indicated I am done
			int done = 1;
			MPI_Send(&done, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
//printf("I am %d and I am doing section %d to %d\n", pid, pid*procRange, (pid * procRange) +procRange-1);
//printf("I am %d and I am sending %d down\n", pid, index);




		}
		else{
			int gotcolabove[MAX_SIZE], gotcolbelow[MAX_SIZE];
			int send, got;
			//Get column from pid below
			for(int i = 0; i< MAX_SIZE; i++){
				MPI_Recv(&got, 1, MPI_INT, pid-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	
				gotcolbelow[i] = got;
			}

			//Send col to pid above
			for(int i = 0; i< MAX_SIZE; i++){
				send = oldheat[i][index+procRange-1];
				MPI_Send(&send, 1, MPI_INT, pid+1, 0, MPI_COMM_WORLD);
			}
//printf("I am %d and I am sending %d up\n", pid, index+procRange-1);



			//Get column from pid above
			for(int i = 0; i< MAX_SIZE; i++){
				MPI_Recv(&got, 1, MPI_INT, pid+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				gotcolabove[i] = got;				
			}

			//Send col to pid below
			for(int i = 0; i < MAX_SIZE; i++){
				send = oldheat[i][index];
				MPI_Send(&send, 1, MPI_INT, pid-1, 0, MPI_COMM_WORLD);
			}
			//TODO Cal
//printf("I am %d and I am sending %d down\n", pid, index);


			
//			printf("I am %d and I am doing section %d to %d\n", pid, pid*procRange, (pid * procRange) +procRange-1);
			//Have the process start at their sector
			int start = pid * procRange;
			//Have the process end at their sector
			int end = (pid * procRange) + procRange - 1;
//printf("I am %d and my start point is %d and my end point is %d\n", pid, start, end);
			//Calculate new heatgrid
			CalculateNew(newheat, oldheat, start, end, lowerFire, upperFire, gotcolabove, gotcolbelow);

			//Send to Master to indicated I am done
			int done = 1;
			MPI_Send(&done, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

		}


	}
	
//	printf("%d has left the buidling\n\n", pid);	
	float stop;
	if(pid == 0){
		PrintGrid(newheat, 0, 0);
		stop = MPI_Wtime();
		printf("This process had %d many iterations and took %f time\n", iter, stop-starttime);
	}


	MPI_Finalize();


	return 0;
}


void CopyNewToOld(float new[][MAX_SIZE], float old[][MAX_SIZE]){

	for(int i = 0; i < MAX_SIZE; i++)
		for(int j = 0; j < MAX_SIZE; j++)
			old[i][j] = new[i][j];



}


/* For CalculateNew
 * 	Need to modify so that it starts and stops at sent in numbers
 * 	Similar to last project, each process will know what segment it is modifying
 * 		at the beginning
 *	Send in start and end ints
 * 	
 *
 *
 *
 */


//TODO, need to have this set for cal size of the parral segments
//replace MAX_SIZE with split up number
//If the calling process only has a below or above column to worry about, the value
//of that column[0] will be set to a negative number
void CalculateNew(float new[][MAX_SIZE], float old[][MAX_SIZE], int start, int end, int lowerFire, int upperFire, int gotcolabove[], int gotcolbelow[]){


	//Nth process calculation
	if(gotcolabove[0] < 0){
		//Since we are dividing one vertical sections of the gird
		//Only limiter on j, <= since we want it to calculate the last column as well
		for(int i = 0;  i < MAX_SIZE - 1; i++)
			for(int j = start; j <= end || j < MAX_SIZE - 1; j++){

//-------------BLOCK-FOR-FIXED-TEMP----------
				if(i == 0 && j >= lowerFire && j < upperFire )
					new[i][j] = 300;
				else if(i == 0)
					new[i][j] = 20;	
				else if(i == MAX_SIZE - 1)
					new[i][j] = 20;
				else if(j == 0)
					new[i][j] = 20;
				else if(j == MAX_SIZE -1)
					new[i][j] = 20;
//-------------END-BLOCK-FOR-FIXED-TEMP----------
				else{
					//Use gotcolsbelow
					if(i == start){
						new[i][j] = 0.25*(old[i-1][j]+old[i+1][j]+gotcolbelow[i]+old[i][j+1]);

					}

					//Use normal calculation
					else{
						new[i][j] = 0.25*(old[i-1][j]+old[i+1][j]+old[i][j-1]+old[i][j+1]);
				
					}



//					new[i][j] = 0.25*(old[i-1][j]+old[i+1][j]+old[i][j-1]+old[i][j+1]);




				}
			}



	}
	//0th process calculation
	else if(gotcolbelow[0] < 0){
		//Since we are dividing one vertical sections of the gird
		//Only limiter on j, <= since we want it to calculate the last column as well
		for(int i = 0;  i < MAX_SIZE - 1; i++)
			for(int j = start; j <= end || j < MAX_SIZE - 1; j++){

//-------------BLOCK-FOR-FIXED-TEMP----------
				if(i == 0 && j >= lowerFire && j < upperFire )
					new[i][j] = 300;
				else if(i == 0)
					new[i][j] = 20;	
				else if(i == MAX_SIZE - 1)
					new[i][j] = 20;
				else if(j == 0)
					new[i][j] = 20;
				else if(j == MAX_SIZE -1)
					new[i][j] = 20;
//-------------END-BLOCK-FOR-FIXED-TEMP----------
				else{
					//Use gotcolsabove
					if(j == end){
						new[i][j] = 0.25*(old[i-1][j]+old[i+1][j]+old[i][j-1]+gotcolabove[i]);

					//Use normal calculation
					}else{
						new[i][j] = 0.25*(old[i-1][j]+old[i+1][j]+old[i][j-1]+old[i][j+1]);
				
					}



//					new[i][j] = 0.25*(old[i-1][j]+old[i+1][j]+old[i][j-1]+old[i][j+1]);




				}
			}

	}
	//Middle Process calculation
	//TODO edit to include gotcols
	else{
		//Since we are dividing one vertical sections of the gird
		//Only limiter on j, <= since we want it to calculate the last column as well
		for(int i = 0;  i < MAX_SIZE - 1; i++)
			for(int j = start; j <= end || j < MAX_SIZE - 1; j++){

//-------------BLOCK-FOR-FIXED-TEMP----------
				if(i == 0 && j >= lowerFire && j < upperFire )
					new[i][j] = 300;
				else if(i == 0)
					new[i][j] = 20;	
				else if(i == MAX_SIZE - 1)
					new[i][j] = 20;
				else if(j == 0)
					new[i][j] = 20;
				else if(j == MAX_SIZE -1)
					new[i][j] = 20;
//-------------END-BLOCK-FOR-FIXED-TEMP----------
				else{
					//Use gotcolsbelow
					if(i == start){
						new[i][j] = 0.25*(old[i-1][j]+old[i+1][j]+gotcolbelow[i]+old[i][j+1]);

					}
					//Use gotcolsabove
					else if(j == end){
						new[i][j] = 0.25*(old[i-1][j]+old[i+1][j]+old[i][j-1]+gotcolabove[i]);


					//Use normal calculation
					}else{
						new[i][j] = 0.25*(old[i-1][j]+old[i+1][j]+old[i][j-1]+old[i][j+1]);
				
					}



//					new[i][j] = 0.25*(old[i-1][j]+old[i+1][j]+old[i][j-1]+old[i][j+1]);




				}
			}

	}



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






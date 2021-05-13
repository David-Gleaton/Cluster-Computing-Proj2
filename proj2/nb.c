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
void CalculateNew(float new[][MAX_SIZE], float old[][MAX_SIZE], int start, int end, int lowerFire, int upperFire);
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
		
		CopyNewToOld(newheat,oldheat);
	//First iteration start
		if(i == 0){

			//Master creates first copytonew and calculates
			if(pid == 0){
//			printf("First iter, pid %d starting\n", pid);
	//			CopyNewToOld(newheat,oldheat);
				//Have the process start at their sector
				int start = pid * procRange;
				//Have the process end at their sector
				int end = (pid * procRange) + procRange - 1;

				//Calculate new heatgrid
				CalculateNew(newheat, oldheat, start, end, lowerFire, upperFire);
//				printf("First iter, pid %d ending \n", pid);
			//Slaves calculate and send to master to indicate first round down
			}else{
//			printf("First iter, pid %d starting\n", pid);
				//Have the process start at their sector
				int start = pid * procRange;
				//Have the process end at their sector
				int end = (pid * procRange) + procRange - 1;

				//Calculate new heatgrid
				CalculateNew(newheat, oldheat, start, end, lowerFire, upperFire);			
				int buf = 0;
	//			MPI_Send(&buf, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
//				printf("First iter, pid %d ending \n", pid);

			}
		//End Iteration
		}else if(i == iter -1){	

			if(pid == 0){
//				printf("Last iter, pid %d starting \n", pid);	
				int buf = 0;
				//Lock all processes once down
//				for(int i = 1; i < numprocess; i++)
	//				MPI_Recv(&buf, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				//Copy newheat to old for working
	//			CopyNewToOld(newheat, oldheat);
				//Release all pids once copy is complete.
//				for(int i = 1; i < numprocess; i++)
	//				MPI_Send(&buf, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
				//Have the process start at their sector
				int start = pid * procRange;
				//Have the process end at their sector
				int end = (pid * procRange) + procRange - 1;

				//Calculate new heatgrid
				CalculateNew(newheat, oldheat, start, end, lowerFire, upperFire);
//				printf("Last iter, pid %d ending \n", pid);



			}else{
//				printf("Last iter, pid %d starting \n", pid);
				int buf = 0;
	//			MPI_Recv(&buf, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				//Have the process start at their sector
				int start = pid * procRange;
				//Have the process end at their sector
				int end = (pid * procRange) + procRange - 1;

				//Calculate new heatgrid
				CalculateNew(newheat, oldheat, start, end, lowerFire, upperFire);
				
//				MPI_Send(&buf, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

//				printf("Last iter, pid %d ending \n", pid);

			}

			

		//Middle Iterations
		}else{
			if(pid == 0){
//				printf("Middle iter, pid %d starting \n", pid);	
				int buf = 0;
				//Lock all processes once down
//				for(int i = 1; i < numprocess; i++)
	//				MPI_Recv(&buf, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//				printf("Middle iter, pid %d, inbetween locks\n", pid);
				//Copy newheat to old for working
	//			CopyNewToOld(newheat, oldheat);
				//Release all pids once copy is complete.
//				for(int i = 1; i < numprocess; i++)
	//				MPI_Send(&buf, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
				//Have the process start at their sector
				int start = pid * procRange;
				//Have the process end at their sector
				int end = (pid * procRange) + procRange - 1;

				//Calculate new heatgrid
				CalculateNew(newheat, oldheat, start, end, lowerFire, upperFire);

//				printf("Middle iter, pid %d ending \n", pid);


			}else{
//				printf("Middle iter, pid %d starting \n", pid);
				int buf = 0;
	//			MPI_Recv(&buf, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				//Have the process start at their sector
				int start = pid * procRange;
				//Have the process end at their sector
				int end = (pid * procRange) + procRange - 1;

				//Calculate new heatgrid
				CalculateNew(newheat, oldheat, start, end, lowerFire, upperFire);
				
	//			MPI_Send(&buf, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
//				printf("Middle iter, pid %d ending \n", pid);


			}







		}


	}
	

	float stop;
	if(pid == 0){
		int buf = 0;
		//Wait for all processes to finish last round
//		for(int i = 1; i < numprocess; i++)
//			MPI_Recv(&buf, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	
		PrintGrid(newheat, 0, 0);
		stop = MPI_Wtime();
		printf("This process had %d many iterations and took %f time\n", iter, stop-starttime);
	}

//	printf("%d has left the buidling\n\n", pid);	
	MPI_Finalize();


	return 0;
}


void CopyNewToOld(float new[][MAX_SIZE], float old[][MAX_SIZE]){

	int MiddleCheck = MAX_SIZE/2;
	for(int i = 0; i < MAX_SIZE; i++)
		for(int j = 0; j < MAX_SIZE; j++){
			if(i >30 && new[i-20][MiddleCheck] <= 20){
				i = j = MAX_SIZE;
				break;
			}
			old[i][j] = new[i][j];
		}


}


//One size fits all CalculateNew
//This should only start and end within the correct sections
//Calculate the newheatmap from the old heatmap
void CalculateNew(float new[][MAX_SIZE], float old[][MAX_SIZE], int start, int end, int lowerFire, int upperFire){

//Problem: It is still slow
//
//Solution: Have it stop once it enters an area that is cold
//	Check middle, lets say the next element down for any heat >20.
//	If there isn't any, stop, and move to next iteration
//
		int MiddleCheck = MAX_SIZE/2;


		//Since we are dividing one vertical sections of the gird
		//Only limiter on j, <= since we want it to calculate the last column as well
		for(int i = 0;  i < MAX_SIZE - 1; i++)
			for(int j = start; j <= end || j < MAX_SIZE - 1; j++){
				if(i >30 && old[i-20][MiddleCheck] <=20){
					i = j = MAX_SIZE;
					break;
				}
//-------------BLOCK-FOR-FIXED-TEMP----------
				else if(i == 0 && j >= lowerFire && j < upperFire )
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
					//Use normal calculation
					new[i][j] = 0.25*(old[i-1][j]+old[i+1][j]+old[i][j-1]+old[i][j+1]);
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






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
void CalculateNew(float new[][MAX_SIZE], float old[][MAX_SIZE], int lowerFire, int upperFire);
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
 * THIS IS THE STATIC/SEQUENTIAL CODE
 *
 *
 *
 * Notes Personal
 * 	Cols/Rows that need to be fixed
 * 		Cols = 0, set to 20
 * 		Cols = MAX_SIZE set to 20
 * 		Rows = 0, set to 20 EXCEPT Fireplace
 * 		Rows = MAX_SIZE set to 20
 */
	float start = MPI_Wtime();
	float newheat[MAX_SIZE][MAX_SIZE];
	float oldheat[MAX_SIZE][MAX_SIZE];

	int iter = atoi(argv[1]);
	int FireLen = MAX_SIZE * .40;
	int lowerFire = MAX_SIZE/2 - FireLen/2;
	int upperFire = MAX_SIZE/2 + FireLen/2;


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

	//For loop for how many iterations
	for(int i = 0; i < iter; i++){
		//Copy new to old
		CopyNewToOld(newheat, oldheat);
		//Calculate New
		CalculateNew(newheat, oldheat, lowerFire, upperFire);
	}

	//Print the final iteration to a .pnm
	PrintGrid(oldheat, 0,0);
	float end = MPI_Wtime();
	printf("The process took %f\n", end-start);
	
	
	
	
	return 0;
}

//This method just copies the new grid to the old
void CopyNewToOld(float new[][MAX_SIZE], float old[][MAX_SIZE]){

	int MiddleCheck = MAX_SIZE/2;
	for(int i = 0; i < MAX_SIZE; i++)
		for(int j = 0; j < MAX_SIZE; j++){
			if(i >30 &&new[i-20][MiddleCheck] <=20){
				i = j = MAX_SIZE;
				break;
			}
			old[i][j] = new[i][j];
		}


}


//This Calculates the new heat movement, I copied over the inital grid color to keep
//the fixed points fixed
void CalculateNew(float new[][MAX_SIZE], float old[][MAX_SIZE], int lowerFire, int upperFire){

	int MiddleCheck = MAX_SIZE/2;
	for(int i = 1; i < MAX_SIZE - 1; i++)
		for(int j = 1; j < MAX_SIZE - 1; j++)
			if(i >30 &&new[i-20][MiddleCheck] <=20){
				i = j = MAX_SIZE;
				break;
			}
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
			else
				new[i][j] = 0.25*(old[i-1][j]+old[i+1][j]+old[i][j-1]+old[i][j+1]);

}
//Adjusted code from printcolors.c to make a .pnm image
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






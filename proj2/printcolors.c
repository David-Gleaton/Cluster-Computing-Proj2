/* A simple program that demonstrates printing colors to a pnm 
   bitmap file.  The format P3 is used for simplicity since it 
   uses ASCII format.  

   Author:  Amy Apon
*/

#include <stdio.h>

/* 10 colors plus white are defined.  Many more are possible. */
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

int main(int argc, char * argv[]) {

   FILE * fp;

   int numcolors = 10;
   int color;

   /* Colors are list in order of intensity */
   char * colors[10] = { RED, ORANGE, YELLOW, LTGREEN, GREEN, 
                         LTBLUE, BLUE, DARKTEAL, BROWN, BLACK };

   /* The image will be 300 pixels wide and 300 pixels tall */
   int linelen = 300;
   int numlines = 300;
   int i, j;

   /* The pnm filename is hard-coded.  */

   fp = fopen("c.pnm", "w");

   /* Print the P3 format header */
   fprintf(fp, "P3\n%d %d\n15\n", linelen, numlines);

   /* Print 300 lines of colors. ASCII makes this easy.           */
   /* Each %s (color string) is a single pixel in the final image */
   for (j=0; j<numlines; j++) {
      color = j/(numlines/numcolors);
      for (i=0; i<linelen; i++)  
         fprintf(fp, "%s ", colors[color] );
      fprintf(fp, "\n");
   }

   fclose(fp);

   /* Convert the pnm file to a format that is more easily viewed
      in a web browser. */ 
   /*   system("convert c.pnm c.png"); */
   system("convert c.pnm c.gif");   /* png not supported on comp */

   return 0;

}  
   

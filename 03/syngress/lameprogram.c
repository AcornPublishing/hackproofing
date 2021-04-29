/* lameprogram.c - Hal Flynn <mrhal@mrhal.com>	*/
/* does not perform sufficient checks for a	*/
/* file before opening it and storing data	*/
/* This program is in Chapter 3.		*/

#include <stdio.h>
#include <unistd.h>

int main()
{

	char a[] = "This is my own special junk data storage.\n";
	char junkpath[] = "/tmp/junktmp";
	FILE *fp;
	fp = fopen(junkpath, "w");

	fputs(a, fp);
	fclose(fp);
	unlink(junkpath);

	return(0);
}

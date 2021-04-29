/*	
	chapter 8 - sample 1
	This is a very simple program to explain how the stack allocates
	local static variables.
*/
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	char buffer[15]="Hello World"; 	/* a 10 byte character buffer */
	int  int1=1,int2=2; 		/* 2 4 byte integers */
	
	return 1;	/*leaves the main func*/
}

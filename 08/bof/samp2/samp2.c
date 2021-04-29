/*	
	chapter 8 - sample 2
	This is a program to explain how the stack is used in call and ret
	operations, as well as how the stack frame is organized.
	it 
*/
#include <stdlib.h>
#include <stdio.h>

int callex(char *buffer, int int1, int int2)
{
	/*This prints the inputted variables to the screen*/
	printf("%s %d %d\n",buffer,int1, int2); 
	return 1;
}


int main(int argc, char **argv)
{
	char buffer[15]="Hello World"; 	/* a 10 byte character buffer */
	int  int1=1,int2=2; 		/* 2 4 byte integers */

	callex(buffer,int1,int2); /*call our function*/
	return 1;	/*leaves the main func*/
}

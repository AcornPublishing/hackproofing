/*	
	chapter 8 - sample 3
	This is a program to show a simple uncontrolled overflow 
	of the stack.  It is inteded to overflow eip with 
	0x41414141, which is AAAA in ascii
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int bof()
{
	char buffer[8]; /* an 8 byte character buffer */
	/*copy 20 bytes of A into the buffer*/
	strcpy(buffer,"AAAAAAAAAAAAAAAAAAAA");
	/*return*/
	return 1;
}


int main(int argc, char **argv)
{
	
	bof(); /*call our function*/
	/*print a short message,
		execution will never reach this point */
	printf("Not gonna do it!\n");
	return 1;	/*leaves the main func*/
}

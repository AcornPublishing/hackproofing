/*	
	chapter 8 - sample 4
	This is a program to show a simple controlled overflow 
	of the stack.  It is supposed to be paired with a
	file we will produce using an exploit program.
	For simplicity's sake, the file is hardcoded to 
	c:\badfile
*/
#include <stdlib.h>
#include <stdio.h>

int bof()
{
	char buffer[8]={0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42}; /* an 8 byte character buffer */
	FILE *badfile;

	/*open badfile for reading*/
	badfile=fopen( "c:\\badfile", "r" );

	/*this is where we overflow.  reading 1024 bytes
		into an 8 byte buffer is a "bad thing" */
	fread( buffer, sizeof( char ), 1024, badfile );
	
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

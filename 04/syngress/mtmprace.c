/* mtmprace.c				*/
/* Hal Flynn <mrhal@mrhal.com>		*/
/* mtmprace.c creates a file in the	*/
/* temporary directory that can be	*/
/* easily guessed, and exploited	*/
/* through a symbolic link attack.	*/
/* This code appears in Chapter 4.	*/

#include <stdio.h>
#include <stdlib.h>

int main()
{
	char *example;
	char *outfile;
	char ex[] = "/tmp/exampleXXXXXX";
	example = ex;

	mktemp(example);
	outfile = fopen(example, "w");

	return (0);
}

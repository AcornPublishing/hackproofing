/* scpybufo.c				*/
/* Hal Flynn <mrhal@mrhal.com>		*/
/* December 31, 2001			*/
/* scpybufo.c demonstrates the problem	*/
/* with the strcpy() function which	*/
/* is part of the c library.  This	*/
/* program demonstrates strcpy not	*/
/* sufficiently checking input.  When	*/
/* executed with an 8 byte argument, a	*/
/* buffer overflow occurs.		*/
/* This code appears in Chapter 4 and 5	*/

#include <stdio.h>
#include <strings.h>

int main(int argc, char *argv[])
{

	overflow_function(*++argv);

	return (0);
}

void overflow_function(char *b)
{
	char c[8];

	strcpy(c, b);
	return;
}

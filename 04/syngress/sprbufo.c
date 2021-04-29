/* sprbufo.c				*/
/* Hal Flynn <mrhal@mrhal.com>		*/
/* December 31, 2001			*/
/* sprbufo.c demonstrates the problem	*/
/* with the sprintf() function which	*/
/* is part of the c library.  This	*/
/* program demonstrates sprintf not	*/
/* sufficiently checking input.  When	*/
/* executed with an argument of 8 bytes */
/* or more a buffer overflow occurs.	*/
/* This code appears in Chapter 4.	*/

#include <stdio.h>

int main(int argc, char *argv[])
{

	overflow_function(*++argv);

	return (0);
}

void overflow_function(char *b)
{
	char c[8];

	sprintf(c, "%s", b);
	return;
}

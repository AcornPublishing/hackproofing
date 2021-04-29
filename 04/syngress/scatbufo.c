/* scatbufo.c				*/
/* Hal Flynn <mrhal@mrhal.com>		*/
/* December 31, 2001			*/
/* scatbufo.c demonstrates the problem	*/
/* with the strcat() function which	*/
/* is part of the c library.  This	*/
/* program demonstrates strcat not	*/
/* sufficiently checking input.  When	*/
/* executed with an 7 byte argument, a	*/
/* buffer overflow occurs.		*/
/* This code appears in Chapter 4.	*/

#include <stdio.h>
#include <strings.h>

int main(int argc, char *argv[])
{

	overflow_function(*++argv);

	return (0);
}

void overflow_function(char *b)
{
	char c[8] = "0";

	strcat(c, b);
	return;
}

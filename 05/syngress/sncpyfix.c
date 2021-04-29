/* sncpyfix.c				*/
/* Hal Flynn <mrhal@mrhal.com>		*/
/* January 13, 2002			*/
/* sncpyfix.c demonstrates the proper	*/
/* function to use when copying		*/
/* strings.  The function provides a	*/
/* check for data length by limiting	*/
/* the amount of data copied.		*/
/* This code appears in Chapter 5.	*/

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
	size_t e = 8;

	strncpy(c, b, e);
	return;
}

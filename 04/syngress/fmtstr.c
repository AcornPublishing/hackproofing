/* fmtstr.c				*/
/* Hal Flynn <mrhal@mrhal.com>		*/
/* December 31, 2001			*/
/* fmtstr.c demonstrates a format	*/
/* string vulnerability.  By supplying	*/
/* format specifiers as arguments,	*/
/* attackers may read or write to	*/
/* memory.				*/
/* This code appears in Chapter 4.	*/

#include <stdio.h>

int main(int argc, char *argv[])
{

	printf(*++argv);

	return (0);
}

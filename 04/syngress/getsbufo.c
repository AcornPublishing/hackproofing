/* getsbufo.c				*/
/* Hal Flynn <mrhal@mrhal.com>		*/
/* December 31, 2001			*/
/* This program demonstrates how NOT	*/
/* to use the gets() function.  gets()	*/
/* does not sufficient check input	*/
/* length, and can result in serious	*/
/* problems such as buffer overflows.	*/
/* This code appears in Chapter 4.	*/

#include <stdio.h>

int main()
{
	get_input();

	return (0);
}

void get_input(void)
{
	char c[8];

	printf("Enter a string greater than seven bytes: ");
	gets(c);

	return;
}

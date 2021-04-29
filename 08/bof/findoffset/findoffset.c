/*
	Findjmp.c
	written by Ryan Permeh - ryan@eeye.com
	http://www.eeye.com

	This finds useful jump points in a dll.  Once you overflow a buffer, by 
	looking in the various registers, it is likely that you will find a 
	reference to your code.  This program will find addresses suitible to
	overwrite eip that will return to your code.  

	It should be easy to modify this to search for other good jump points, 
	or specific code patterns within a dll.

	It currently supports looking for: 
			1. jmp reg

			2. call reg

			3. push reg
			   ret	
	All three options result in the same thing, EIP being set to reg.

	It also supports the following registers:
		EAX
		EBX
		ECX
		EDX
		ESI
		EDI
		ESP
		EBP
*/

#include <Windows.h>
#include <stdio.h>

void usage();
DWORD GetRegNum(char *reg);
void findjmp(char *dll,char *reg);
/*
	This finds useful jump points in a dll.  Once you overflow a buffer, by 
	looking in the various registers, it is likely that you will find a 
	reference to your code.  This program will find addresses of suitible
	addresses of eip that will return to your code.  

*/
int main(int argc, char **argv)
{
	char dll[512], //holder for the dll to look in
		 reg[512]; // holder for the register

	if(argc<2) usage(); 
	strncpy(dll,argv[1],512);
	strncpy(reg,argv[2],512);
	findjmp(dll,reg);
}

/*
	This prints the usage information.  
*/

void usage()
{
	printf("FindJmp usage\nfindjmp DLL reg\nEx: findjmp KERNEL32.DLL esp\n");
}

/*
	findjmp is the workhorse.  it loads the requested dll, and searches for 
	the specific patterns for jmp reg, push reg ret, and call reg

  */

void findjmp(char *dll,char *reg)
{
	/* patterns for jmp ops */
	BYTE jmppat[8][2]=		{{0xFF,0xE0},{0xFF,0xE3},{0xFF,0xE1},{0xFF,0xE2},
							{0xFF,0xE6},{0xFF,0xE7},{0xFF,0xE4},{0xFF,0xE5}};
	/* patterns for call ops */
	BYTE callpat[8][2]=		{{0xFF,0xD0},{0xFF,0xD3},{0xFF,0xD1},{0xFF,0xD2},
							{0xFF,0xD6},{0xFF,0xD7},{0xFF,0xD4},{0xFF,0xD5}};
	/* patterns for pushret ops */
	
	BYTE pushretpat[8][2]=	{{0x50,0xC3},{0x53,0xC3},{0x51,0xC3},{0x52,0xC3},
							{0x56,0xC3},{0x57,0xC3},{0x54,0xC3},{0x55,0xC3}};

	/*base pointer for the loaded DLL*/
	HMODULE loadedDLL;

	/*current position within the  DLL */
	BYTE *curpos;

	/* decimal representation of passed register */
	DWORD regnum=GetRegNum(reg);

	/*accumulator for addresses*/
	DWORD numaddr=0;

	/*check if register is useable*/
	if(regnum == -1)
	{
		/*it didn't load, time to bail*/
		printf("There was a problem understanding the register.\n"\
			"Please check that it isa correct IA32 register name\n"\
			"Currently supported are:\n "\
			"EAX, EBX, ECX, EDX, ESI, EDI, ESP, EBP\n"\
			);

		exit(-1);
	}

	loadedDLL=LoadLibraryA(dll);

	/* check if DLL loaded correctly*/
	if(loadedDLL == NULL)
	{
		/*it didn't load, time to bail*/
		printf("There was a problem Loading the requested DLL.\n"\
			"Please check that it is in your path and readable\n" );
		exit(-1);
	}
	else
	{
		/*we loaded the dll correctly, time to scan it*/
		printf("Scanning %s for code useable with the %s register\n",
			dll,reg);
		/*set curpos at start of DLL*/
		curpos=(BYTE*)loadedDLL;

		__try
		{
			while(1)
			{
				/*check for jmp match*/
				if(!memcmp(curpos,jmppat[regnum],2))
				{
					/* we have a jmp match */
					printf("0x%X\tjmp %s\n",curpos,reg);
					numaddr++;
				}
				/*check for call match*/
				else if(!memcmp(curpos,callpat[regnum],2))
				{
					/* we have a call match */
					printf("0x%X\tcall %s\n",curpos,reg);
					numaddr++;
				}
				/*check for push/ret match*/
				else if(!memcmp(curpos,pushretpat[regnum],2))
				{
					/* we have a pushret match */
					printf("0x%X\tpush %s - ret\n",curpos,reg);
					numaddr++;
				}
				curpos++;
			}
		}
		__except(1)
		{
			printf("Finished Scanning %s for code useable with the %s register\n",
				dll,reg);
			printf("Found %d usable addresses\n",numaddr);
		}
	}

}


DWORD GetRegNum(char *reg)
{
	DWORD ret=-1;
	if(!stricmp(reg,"eax"))
	{
		ret=0;
	}
	else if(!stricmp(reg,"ebx"))
	{
		ret=1;
	}
	else if(!stricmp(reg,"ecx"))
	{
		ret=2;
	}
	else if(!stricmp(reg,"edx"))
	{
		ret=3;
	}
	else if(!stricmp(reg,"esi"))
	{
		ret=4;
	}
	else if(!stricmp(reg,"edi"))
	{
		ret=5;
	}
	else if(!stricmp(reg,"esp"))
	{
		ret=6;
	}
	else if(!stricmp(reg,"ebp"))
	{
		ret=7;
	}
	/*return our decimal register number*/
	return ret;
}
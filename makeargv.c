// makeargv.c 
// takes a null terminated string s, and a string of delimiters and returns 
// an array of char * pointers, leaving argvp pointing to that array.
// each of the array locations points to a null terminated "token" found in s
// defined by the delimiter array. The number of tokens found is returned by makeargv
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/*
 * Make argv array (*argvp) for tokens in s which are separated by
 * delimiters.  Return -1 on error or the number of tokens otherwise.
 */
int makeargv(char * s, char * delimiters, char *** argvp)
{
	char * t;
	char * snew;
	int numtokens;
	int i;
	
	/* snew is real start of string after skipping leading delimiters */
	snew = s + strspn(s, delimiters); //very clever
	//printf("s = %s\n", s);
	//printf("delimiters = %s\n", delimiters);
	//printf("strspn = %d\n", (int) strspn(s, delimiters));
	//printf("snew = %s\n", snew);
	
	/* create space for a copy of snew in t */	
	if ((t = calloc(strlen(snew) + 1, sizeof(char))) == NULL) //only true if no
								  //more memory
	{
		*argvp = NULL;
		numtokens = -1;
	} 
	else 
	{                     /* count the number of tokens in snew */
		strcpy(t, snew);
		
		if (strtok(t, delimiters) == NULL)
			numtokens = 0;
		else
			for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++);  // THE work is in the For Construct
			//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			//start at 1 and move to the next,
			//	if its null stop
			//	else increment and move to the next
			
		/* create an argument array to contain ptrs to tokens */
		if ((*argvp = calloc(numtokens + 1, sizeof(char *))) == NULL) 
		   //^Notice the dereferencing		//Again only true if 
							//no more memory
		{
			free(t);
			numtokens = -1;
		}
		else 
		{            /* insert pointers to tokens into the array */
			if (numtokens > 0) 
			{
				strcpy(t, snew);
				**argvp = strtok(t, delimiters); //set the first token
				//printf("1 = %s\n", **argvp);
				
				for (i = 1; i < numtokens + 1; i++)
				{
					*((*argvp) + i) = strtok(NULL, delimiters);
					//^^^^^^^^^^^^^^^^ set the other tokens
					//printf("%d = %s\n", i, *((*argvp) + i));
				}
			}
			else 
			{
				**argvp = NULL;
				free(t);
			}
		}
	}
	
	return numtokens;
}

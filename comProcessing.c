/**********************************************************************
Command processing functions.
Matthew Moore
Last modified: 4/20/13
All functions relating to parsing/running commands given in the command
prompt. This includes work with aliases and external commands.
**********************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include "mcshlist.h"
#include "memory.h"
#include "comProcessing.h"

#define BUFFER_SIZE 150 //max buffer size

extern int numArgs; //number of command arguments
extern int maxArgs; //number of arguments shell able to handle

extern Node *env, *ali; //environmental variable and alias lists

extern char **argsV;

/************************************************
parseAliases(char *)
input: character string for variable in question
return: int of success
Takes the current variable inputted and checks it against the
currently stored aliases. Should it find it, it will input the alias
value into the arguement and return 1.
Otherwise, it will do nothing and return 0.
************************************************/
int parseAliases(char *var)
{
	Node *temp;
	char compString[20];
	char tempStringData[50];
	temp = mcshFind(ali, var); //searches for alias
	if (temp != NULL)		   //if found
	{
		strcpy(tempStringData, temp->data);
		//printf("alias %s found.\n",input);
		if (numArgs > 0)
		{
			if (strcmp(argsV[numArgs - 1], "saila") == 0 || strcmp(argsV[numArgs - 1], "alias") == 0) //this is a check so it doesn't automatically
			{																						  //change the alias before saila or alias command can get to it
				return 0;
			}
		}
		if (strchr(tempStringData, '{') != NULL) //check for complex string
		{
			int flag = 0;
			while (flag == 0) //this subroutine adds all of the complex string to the argsV
			{
				sscanf(tempStringData, "%s", compString); //parses word to compString
				int len = strlen(compString);
				int len2 = strlen(tempStringData);
				if (compString[0] == '{')				   //checks for { in first position
					sscanf(compString, "{%s", compString); //this removes the '{'
				if (compString[len - 1] == '}')			   //checks for end of complex string
				{
					compString[len - 1] = '\0'; //and replaces it with term character
					flag = 1;
				}
				strcpy(argsV[numArgs], compString); //adds current word to argsV array
				numArgs++;
				memcpy(tempStringData, &tempStringData[len], len2 - len); //the grabs the rest of the main string
				tempStringData[len2 - len] = '\0';
			} //repeat until end
			return 1;
		}
		else //if not complex string
		{
			strcpy(argsV[numArgs], temp->data); //copies over alias
			numArgs++;
			return 1;
		}
	}
	return 0;
}

/************************************************
int parseCommand(char *, int)
input: character string
return: int flag for inputting another line before processing
Takes a full line of characters and parses it into
seperate commands held in the argsV array.
Also increments numArgs to keep track of it.
************************************************/
int parseCommand(char *input, int lineExtensionFlag)
{
	char *temp;
	int compStringFlag = 0;
	int qFlag = 0;
	if (lineExtensionFlag == 0) //if this is flagged, it is continuing from the line before
	{
		numArgs = 0;
		int i;
		for (i = 0; i < maxArgs; i++)
		{
			argsV[i][0] = '\0';
		}
	}

	if (strchr(input, '?') != NULL) //check for ? because of echo behavior
		qFlag = 1;

	if (lineExtensionFlag == 1 && strcmp(argsV[0], "echo") == 0) //check for continuation of echo command
		temp = strtok(input, "\n\r");
	else
		temp = strtok(input, " \n\r\t"); //bases 'word' off of spaces and newline characters

	while (temp != NULL)
	{
		if (strcmp(temp, "echo") == 0 || (lineExtensionFlag == 1 && strcmp(argsV[0], "echo") == 0)) //checking for echo command
		{
			strcpy(argsV[numArgs], temp);
			numArgs++;
			if (qFlag == 1)
			{
				temp = strtok(NULL, "?");
				strcpy(argsV[numArgs], temp);
				numArgs++;
				processInternalCommand();
				numArgs = 0;
				temp = strtok(NULL, " \n\r\t");
				qFlag = 0;
				continue;
			}
			else
			{
				temp = strtok(NULL, "\n\r");
				continue;
			}
		}

		//~ symbol processing
		if (strchr(temp, '~') != NULL)
		{
			if (strchr(temp, '~') != temp) //making sure comment is not at beginning of argument
			{
				char *commentPlace = strchr(temp, '~');				//copies everything before ~
				strncpy(argsV[numArgs], temp, commentPlace - temp); //and adds null char at location
				argsV[numArgs][commentPlace - temp] = '\0';			//of ~ character
				numArgs++;
			}
			break;
		}

		//$ symbol processing
		if (strchr(temp, '$') != NULL)
		{
			char *lineExtPlace = strchr(temp, '$');
			if (strchr(temp, '$') != temp)
			{
				strncpy(argsV[numArgs], temp, lineExtPlace - temp); //copies argument
				argsV[numArgs][lineExtPlace - temp] = '\0';
				numArgs++;
			}
			return 1; //Flag telling program to require an extra line
		}

		//? symbol processing
		if (strchr(temp, '?') != NULL)
		{
			char *questionPlace = strchr(temp, '?');
			strncpy(argsV[numArgs], temp, questionPlace - temp);
			argsV[numArgs][questionPlace - temp] = '\0';
			numArgs++;
			processInternalCommand();
			numArgs = 0;
			if (strlen(questionPlace) > 1) //checking to make sure the ? is followed
			{							   //by another command 'abc?def' and not 'abc? def'
				strcpy(argsV[numArgs], questionPlace + 1);
				numArgs++;
			}
			temp = strtok(NULL, " \n\r\t");
			continue;
		}

		if (strchr(temp, '{') != NULL) //looks for { character, specifying a complex string
		{
			compStringFlag = 1;
			strcat(argsV[numArgs], temp);   //if found, it starts adding anything
			strcat(argsV[numArgs], " ");	//after it to the current arguement var
			temp = strtok(NULL, " \n\r\t"); //and advancing the token until a } is found
			continue;
		}
		if (compStringFlag == 1)
		{
			if (strchr(temp, '}') != NULL)
			{
				strcat(argsV[numArgs], temp); //when the } is found, it adds the last
				compStringFlag = 0;			  //word, turns off the flag, and increments
				numArgs++;					  //the numArgs
				temp = strtok(NULL, " \n\r\t");
				continue;
			}
			else
			{
				strcat(argsV[numArgs], temp); //this means } hasnt been found yet
				strcat(argsV[numArgs], " ");  //so it adds the word and continues
				temp = strtok(NULL, " \n\r\t");
				continue;
			}
		}
		if (!parseAliases(temp)) //runs parseAliases call. if no alias
		{
			strcpy(argsV[numArgs], temp); //then copies that word to argsV
			numArgs++;
		}
		temp = strtok(NULL, " \n\r\t");
		if (numArgs >= maxArgs) //checking to see if more space needs to be allocated
		{						//for more arguments
			int prevMax = maxArgs;
			int i;
			maxArgs *= 2;
			argsV = (char **)mcshRealloc(argsV, maxArgs * sizeof(char *));
			for (i = prevMax; i < maxArgs; i++)
			{
				argsV[i] = (char *)mcshMalloc(BUFFER_SIZE * sizeof(char));
			}
		}
	}
	return 0;
} //end parseCommand function

/************************************************
processInternalCommand()
input: no input
return: integer
Takes argsV and compares it against all known
commands. Then deals according to which command and
which variables follow.
Returns an int of 1 if wanting to exit, 0 otherwise.
************************************************/
int processInternalCommand()
{
	///////////////////////////////exit command
	if (strcmp(argsV[0], "exit") == 0)
		return 1;

	///////////////////////////////set command
	else if (strcmp(argsV[0], "set") == 0)
	{
		int i;
		//if there are no commands following "set", print out env. variables
		if (numArgs == 1)
		{
			printf("Current Environmental Variables:\n");
			printList(env);
		}

		//2 arguments means it is wanting to print out the value of the env. variable
		else if (numArgs == 2)
		{
			Node *temp;
			temp = mcshFind(env, argsV[1]);
			if (temp != NULL)
				printNode(temp);
			printf("\n");
		}

		//3 arguments means it's trying to actually set a variable OR extend a variable
		else if (numArgs == 3)
		{
			int foundAt = 0;	   //flags and indexes, mainly used for
			int foundAtAndVar = 0; //replacing @var with varvalue
			int atIndex = -1;
			int atLength = 0;
			int j;

			//checking for @ for replacing the variable
			for (i = 0; i < strlen(argsV[2]); i++)
			{
				char temp[BUFFER_SIZE / 2] = ""; //used for substringing
				char temp2[BUFFER_SIZE / 2] = "";
				Node *tempNode;

				if (argsV[2][i] == '@') //looks for @
				{
					atIndex = i;
					foundAt = 1; //flag switched
					foundAtAndVar = 0;
					//printf("@ found at %d\n",atIndex);
					for (j = i + 1; j < (strlen(argsV[2]) + 1); j++) //finds length of variable name
					{
						if (argsV[2][j] == '\0' || argsV[2][j] == '!' || argsV[2][j] == ' ' || argsV[2][j] == '@')
						{
							atLength = (j - i) - 1;
							//printf("variable length %d\n",atLength);
							break;
						}
					}
					strncpy(temp, &argsV[2][atIndex + 1], atLength); //copies variable name to temp
					//printf("%s is the variable after @\n",temp);
					tempNode = mcshFind(env, temp);
					if (tempNode != NULL)
					{
						//printf("@%s: %s\n",tempNode->label,tempNode->data);
						strncpy(temp2, &argsV[2][0], atIndex); //and replaces the @varname with the varvalue
						strncat(temp2, tempNode->data, strlen(tempNode->data));
						strncat(temp2, &argsV[2][atIndex + atLength + 1], strlen(argsV[2]) - (atIndex + atLength + 1));
						//printf("%s is now %s\n",argsV[2],temp2);
						strcpy(argsV[2], temp2); //and copies that new string to the main argument value
						i = 0;					 //this resets the loop back to find the next @ value (should there be multiple in the string)
						foundAtAndVar = 1;
					}
					else //should the var not be found in current variables, it cancels the whole operation
					{
						printf("@%s not found, nothing replaced\n\n", temp);
						continue;
					}
				}
			}

			if (foundAt == 1 && foundAtAndVar != 1)
				return 0; //should the var not be found in current variables, it cancels the whole operation

			//after replacing any @'s there may be, goes along with var assignment
			mcshInsert(&env, argsV[1], argsV[2]);
		}
		else if (numArgs == 0)
		{
		}
		//if too many arguments for set, do nothing
		else
		{
			printf("too many arguments for set\n\n");
		}

	} //end of set command

	///////////////////////////////tes command
	else if (strcmp(argsV[0], "tes") == 0)
	{
		mcshRemove(&env, argsV[1]);
	} //end of tes command

	///////////////////////////////alias command
	else if (strcmp(argsV[0], "alias") == 0)
	{
		//if there are no commands following "alias", print out all aliases
		if (numArgs == 1)
		{
			printf("Current Aliases:\n");
			printList(ali);
		}

		//2 arguments means it is wanting to print out the value of the alias
		else if (numArgs == 2)
		{
			Node *temp;
			temp = mcshFind(ali, argsV[1]);
			if (temp != NULL)
				printNode(temp);
			printf("\n");
		}

		//3 arguments means it's trying to actually set a variable
		else if (numArgs == 3)
		{
			mcshInsert(&ali, argsV[1], argsV[2]);
		}
		else
		{
			printf("too many arguments for alias\n\n");
		}
	} //end of alias command

	///////////////////////////////saila command
	else if (strcmp(argsV[0], "saila") == 0)
	{
		mcshRemove(&ali, argsV[1]);
	} //end of salia command

	///////////////////////////////echo command
	else if (strcmp(argsV[0], "echo") == 0)
	{
		int i;
		for (i = 1; i < numArgs; i++)
		{
			while (strchr(argsV[i], '@') != NULL) //check for @env variables
			{
				char *atLoc = strchr(argsV[i], '@');
				char temp[BUFFER_SIZE / 2] = ""; //used for substringing
				char temp2[BUFFER_SIZE / 2] = "";
				int j, atLength;

				Node *tempNode;

				for (j = atLoc - argsV[i] + 1; j < strlen(atLoc) + 1; j++) //finds length of variable name
				{
					if (argsV[i][j] == '\0' || argsV[i][j] == '!' || argsV[i][j] == ' ' || argsV[i][j] == '@' || argsV[i][j] == '\t')
					{
						atLength = (j - (atLoc - argsV[i])) - 1;
						//printf("variable length %d\n",atLength);
						break;
					}
				}

				strncpy(temp, &argsV[i][(atLoc - argsV[i]) + 1], atLength); //copies variable name to temp

				tempNode = mcshFind(env, temp);
				if (tempNode != NULL)
				{
					strncpy(temp2, &argsV[i][0], atLoc - argsV[i]); //and replaces the @varname with the varvalue
					strncat(temp2, tempNode->data, strlen(tempNode->data));
					strncat(temp2, &argsV[i][(atLoc - argsV[i]) + atLength + 1], strlen(argsV[i]) - ((atLoc - argsV[i]) + atLength + 1));
					strcpy(argsV[i], temp2); //and copies that new string to the main argument value
				}
				else
					break; //resumes printing if @var isn't found
			}
			printf("%s", argsV[i]);
		}
		printf("\n");
	} //end of echo command

	///////////////////////////////where command
	else if (strcmp(argsV[0], "where") == 0)
	{
		char tempPathWhole[BUFFER_SIZE];
		char tempPathCurrent[BUFFER_SIZE];
		int found;
		char presDir[BUFFER_SIZE];
		getcwd(presDir, sizeof(presDir));

		if (numArgs == 3)
		{
			//checking if internal command
			if (strcmp(argsV[2], "exit") == 0 || strcmp(argsV[2], "set") == 0 || strcmp(argsV[2], "tes") == 0 || strcmp(argsV[2], "alias") == 0 || strcmp(argsV[2], "saila") == 0 || strcmp(argsV[2], "echo") == 0 || strcmp(argsV[2], "where") == 0)
			{
				printf("%s is an internal command.\n", argsV[2]);
			}

			//checking for proper type input
			else if (strcmp(argsV[1], "any") == 0 || strcmp(argsV[1], "read") == 0 || strcmp(argsV[1], "write") == 0 || strcmp(argsV[1], "run") == 0 || strcmp(argsV[1], "dir") == 0 || strcmp(argsV[1], "file") == 0)
			{
				if (strchr(argsV[2], '/') == 0)
				{
					found = whereTest("", argsV[2], tempPathCurrent);
					if (found) //prints if whereTest returned something
					{
						printf("%s\n", tempPathCurrent);
					}
					else //otherwise, tests for .
					{
						found = whereTest("./", argsV[2], tempPathCurrent);
						if (found)
						{
							printf("./%s\n", argsV[2]);
						}
						else //and tests for ..
						{
							found = whereTest("../", argsV[2], tempPathCurrent);
							if (found)
							{
								printf("../%s\n", argsV[2]);
							}
						}
					}

					if (!found)
					{
						printf("%s was not found.\n", argsV[2]);
					}
				}

				else //test routine for absolute/relative pathing
				{
					strcpy(tempPathWhole, argsV[2]);
					char *tempElement = tempPathWhole;
					char element[BUFFER_SIZE];
					while (strchr(tempElement, '/')) //filtering out end element
					{
						tempElement = strchr(tempElement, '/') + 1;
					}
					strcpy(element, tempElement);
					tempPathWhole[strlen(tempPathWhole) - strlen(element)] = '\0'; //erasing element from path
					found = whereTest(tempPathWhole, element, tempPathCurrent);	//and running whereTest on result

					if (found) //prints if whereTest returned something
					{
						printf("%s\n", tempPathCurrent);
					}
					else
					{
						printf("%s was not found.\n", argsV[2]);
					}
				}
			}
			else
			{
				printf("incorrect type for arg2 of where\n\n");
			}
		}
		else //too many or few arguments
		{
			printf("incorrect number of arguments for where\n\n");
		}

	} //end of where command

	///////////////////////////////external commands
	else
	{
		//small check to see if anything was actually inputted
		if (strcmp(argsV[0], "") != 0)
		{
			int found = 0;		 //flag for finding actual command
			int threadFound = 0; //flag for "+" background thread
			char tempPathCurrent[BUFFER_SIZE];
			printf("External argument:\n");

			if (argsV[0][0] == '+') //background check
			{
				threadFound = 1;
				if (strlen(argsV[0]) == 1) //the + would be all by itself if there were spaces
				{						   //after it due to parsing. If this is the case, need to
					int i;				   //move all the args forward.
					for (i = 0; i < numArgs; i++)
					{
						strcpy(argsV[i], argsV[i + 1]);
					}
					numArgs--; //decreases numArgs since taking an arg away
				}
				else //removes leading +
				{
					memmove(argsV[0], argsV[0] + 1, strlen(argsV[0]));
				}

				//now checking to see if an internal command was hiding behind the +
				if (strcmp(argsV[0], "exit") == 0 || strcmp(argsV[0], "set") == 0 || strcmp(argsV[0], "tes") == 0 || strcmp(argsV[0], "alias") == 0 || strcmp(argsV[0], "saila") == 0 || strcmp(argsV[0], "echo") == 0 || strcmp(argsV[0], "where") == 0)
				{
					printf("%s is an internal command.\n", argsV[0]);
					return 0; //goes back to main loop
				}
			}

			if (strchr(argsV[0], '/') == 0) //checking against relative/absolute path
			{
				found = whereTest("", argsV[0], tempPathCurrent);
				if (found) //prints if whereTest returned something
				{
					printf("Found at %s, running..\n", tempPathCurrent);
				}

				if (!found)
				{
					printf("%s was not found in PATH environment variable.\n", argsV[0]);
				}
			}

			else //test routine for absolute/relative pathing
			{
				char tempPathWhole[BUFFER_SIZE];
				strcpy(tempPathWhole, argsV[0]);
				char *tempElement = tempPathWhole;
				char element[BUFFER_SIZE];
				while (strchr(tempElement, '/')) //filtering out end element
				{
					tempElement = strchr(tempElement, '/') + 1;
				}
				strcpy(element, tempElement);
				tempPathWhole[strlen(tempPathWhole) - strlen(element)] = '\0'; //erasing element from path
				found = whereTest(tempPathWhole, element, tempPathCurrent);	//and running whereTest on result

				if (found) //prints if whereTest returned something
				{
					printf("Found at %s, running...\n", tempPathCurrent);
				}
				else
				{
					printf("%s was not found.\n", argsV[0]);
				}
			}

			if (found)
			{
				//malloc-ing args vector
				char **args;
				int i;
				args = (char **)mcshCalloc(numArgs * sizeof(char *));
				for (i = 0; i < numArgs; i++)
				{
					args[i] = (char *)mcshCalloc(BUFFER_SIZE * sizeof(char));
					strcpy(args[i], argsV[i]);
				}
				strcpy(args[0], tempPathCurrent);

				if (threadFound)
				{
					pthread_t thr;
					if (pthread_create(&thr, NULL, runExternalCommand, (void *)args))
					{
						printf("Could not create thread\n");
						return -1;
					}
				}
				else
				{
					runExternalCommand(args);
				}
				for (i = 0; i < numArgs; i++)
				{
					mcshFree(args[i]);
				}
				mcshFree(args);
			}
		}
	}

	return 0;
}

/************************************************
void *runExternalCommand(void *arguments)
input: char *arguments
return: no return
Forks program and runs whatever the inputted arguments
are.
************************************************/
void *runExternalCommand(void *arguments)
{
	char **args = (char **)arguments;
	//running routine
	pid_t childpid;
	childpid = fork();
	if (childpid == -1)
	{
		perror("Error forking");
		exit(1);
	}
	else if (childpid == 0) //child
	{
		execvp(args[0], args);
	}
	else //parent
	{
		wait(&childpid);
		printf("%s:%d finished\n", args[0], (int)childpid);
	}
	return NULL;
}

/************************************************
int whereTest(char *testPath, char *testElement, char *buffer)
input: char *testPath, char *testElement, char *buffer
return: int found
Checks inputted path and element, returning 1 if found and also
modifying buffer to actual path.
If inputted path is relative or absolute, will only check those
paths. If no path given, will check all paths in PATH env variable.
************************************************/
int whereTest(char *testPath, char *testElement, char *buffer)
{
	char tempPathWhole[BUFFER_SIZE];
	char tempPathCurrent[BUFFER_SIZE];
	char *temp;
	int found = 0;
	Node *tempNode;
	struct stat stats;
	char presDir[BUFFER_SIZE];

	getcwd(presDir, sizeof(presDir)); //gets working directory

	if (strchr(testPath, '/') == NULL) //checking to see if PATH is needed
	{
		tempNode = mcshFind(env, "PATH"); //finds PATH env var
		if (tempNode != NULL)
		{
			strcpy(tempPathWhole, tempNode->data);
		}
		else
		{
			printf("PATH was not found\n");
		}
	}
	else //else, using relative or absolute path
	{
		if (testPath[0] == '.') //if relative path
		{
			strcpy(tempPathWhole, presDir);
			strcat(tempPathWhole, "/");
			strcat(tempPathWhole, testPath);
			tempPathWhole[strlen(tempPathWhole) - 1] = '\0';
		}
		else //if absolute path
		{
			strcpy(tempPathWhole, testPath);
			tempPathWhole[strlen(tempPathWhole) - 1] = '\0';
		}
	}

	temp = strtok(tempPathWhole, "!\0"); //and starts chopping it up
	while (temp != NULL)
	{
		strcpy(tempPathCurrent, temp);
		strcat(tempPathCurrent, "/");
		strcat(tempPathCurrent, testElement);
		//printf("Checking: %s\n",tempPathCurrent);
		if (stat(tempPathCurrent, &stats) == 0) //running stats on file/directory
		{
			if (strcmp(argsV[0], "where") != 0)
			{
				found = 1;
			}
			else if (strcmp(argsV[1], "any") == 0)
			{
				found = 1;
			}
			else if (strcmp(argsV[1], "read") == 0 && access(tempPathCurrent, R_OK) == 0)
			{
				found = 1;
			}
			else if (strcmp(argsV[1], "write") == 0 && access(tempPathCurrent, W_OK) == 0)
			{
				found = 1;
			}
			else if (strcmp(argsV[1], "run") == 0 && access(tempPathCurrent, X_OK) == 0)
			{
				found = 1;
			}
			else if (strcmp(argsV[1], "dir") == 0 && S_ISDIR(stats.st_mode))
			{
				found = 1;
			}
			else if (strcmp(argsV[1], "file") == 0 && S_ISREG(stats.st_mode))
			{
				found = 1;
			}
		}
		if (found)
		{
			//printf("%s\n",tempPathCurrent);
			strcpy(buffer, tempPathCurrent);
			return 1;
		}

		temp = strtok(NULL, "!\0");
	}
	return 0;
}

/************************************************
void freeVariables
input: no input
return: no output
Free's all of the args that were allocated
************************************************/
void freeVariables()
{
	int i;
	for (i = 0; i < maxArgs; i++)
	{
		mcshFree(argsV[i]);
	}
	mcshFree(argsV);
}

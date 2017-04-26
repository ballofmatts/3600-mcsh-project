/************************************************
Matthew Moore
CSCE 3600.001
Program 4
4/12/2013

IDE/editor:
	Dev-C++
	VIM

Program is a shell named msch.

Supports:
1)	Command line execution.
	Execute script or internal commands through command-line arguments:
		mcsh script_file
		mcsh command

	Program actually does this. Instructions said to simply print commands,
	but program will interpret command-line arguments.
	If command is a file, will treat it as a script and run the script.

2)	Interactive execution.
	All information entered after invoking the shell is interpreted by the
	shell until the command "exit" occurs.
		mcsh>

3)	Environment variables:
	Shell supports the ability to set, display, reset, extend, and clear
	environment variables.
	a.	Set an environment variable.
		set var var-value
	b.	Display an environment variable.
		set var
	c.	Display all environment variables.
		set
	d.	Reset an environment variable.
		set var new-value
	e.	Extend an environment variable.
		set var @var!new-value
		set var new-value!@var
	f.	Clear an environment variable.
		tes var
	g.	Environment variable usage occurs by placing the symbol "@" immediately,
		no space, in front of the variable.

4)	Command aliases:
	Shell supports the ability to set, display, reset, and clear command
	aliases, direct or indirect.
	a.	Set a command alias.
		alias cmd cmd-value
	b.	Display a command alias.
		alias cmd
	c.	Display all command aliases.
		alias
	d.	Reset a command alias.
		alias cmd new-value
	e.	Clear a command alias.
		saila cmd
	Alias command currently does not do anything besides store aliases.
	This was instructed by our professor.

5)	Internal commands,
	The only internal commands are set, tes, alias, saila, and exit.

6)	External commands.
	Anything which is not an internal command is considered an external command.
	For this programming exercise, external commands are not to be executed,
	simply displayed.

7)	Memory subsystem for dynamically allocating internal storage.
	a.	mcshMalloc
		Allocates space
	b.	mcshCalloc
		Allocates space and 0's the memory
	c.	mcshRealloc
		Re-allocates space
	d.	mcshFree
		Frees this variable

8)	Algorithms for keeping data elements in alphabetical order
	a.	mcshInsert
		Inserts a new element into its appropriate data structure
	b.	mcshUpdate
		Changes the value associate with either an environment variable
		or command alias
	c.	mcshRemove
		Removes either an environment variable or command alias
	d.	mcshFind
		Finds either an environment variable or command alias
	e.	mcshHead
		Returns the lexicographically smallest entry from either the
		environment variable or the command alias structures
	f.	mcshNext
		Moves to the next item, lexicographically, in either the environment
		variable or the command alias structures

9)	Implements the comment character ~.
	At this time, everything following the ~ is considered to be comments.

10)	Implements complex strings.
	A complex string is any string between matching braces, { }.

11) The HOME directory is automatically stored as an environment variable
	before any other operations are performed.

12)	Opens and process the resource file .mcshrc which is found in the home
	directory. The resource file is allowed to contain any command/line that
	can be typed in interactive mode.

13)	Line extensions: The symbol "$" is interpreted as indicating that the
	line is continued on the next line.  Anything on the same line after
	the "$" is ignored.

14)	Command ending: a line with embedded "?" symbols will be considered to
	include multiple commands separated by those symbols.

15)	Implements the internal command, "echo."
	a.	This command will print out the remainder of the line as text.
	b.	The "$" can be used to extend the text to multiple lines.
	c.	If the "?" symbol is present after an echo command, it indicates
		the end of the text to be output is the character preceding the
		symbol.
	d.	If the "@" symbol is present, then the tag following the symbol
		is to be treated as an environment variable whose value is to be
		inserted into the output string.

16) Implements the internal command “where” (with usage “where type name”)
	which will determine the location of the file “name” with the permission
	“type”.
	a.	If “name“ does not contain any “/” characters, the program will search
		through the PATH variable list of directories.
	b.	If name does contain any “/” characters, then the program will ignore
		the PATH variable and look at the absolute or relative location to
		determine if the permission is set correctly according to “type.”
	c.	Aliases are observed.
	d.	If name refers to an internal command, it is noted as such.
	e.	The value of “type” is allowed to take any of the following values:
		any
		read
		write
		run
		dir
		file.
	f.	This command returns the first qualified path to the file which meets
		the “type” specified by the user.  If none are found, this is reported.

17.	Implements external commands via the fork(), exec(), and wait() families
	of functions.
	a.	The user may execute an external command in any directory included in
		the internal environment variable PATH.
	b.	Concatenates the detected path with the file name before issuing the
		external command.
	c.	If the command file cannot be found in any of the directories in PATH,
		the program shall issue the result “command “requested command” not
		found.”

************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "pwd.h"
#include "mcshlist.h"
#include "memory.h"
#include "comProcessing.h"

#define BUFFER_SIZE 150 //max buffer size

int numArgs = 0;  //number of command arguments
int maxArgs = 10; //number of arguments shell able to handle

Node *env, *ali; //environmental variable and alias lists

char **argsV;

/************************************************
main program
input:
	int argc: number of arguments from outside of shell
	char *argv[]: multidimension char array holding
					arguments from outside of shell
returns:
	int: 0 upon successful completion

Main loop for the shell. Does 3 things:
	-Loads .mcshrc file from HOME directory.

	-If external arguments are found, will check
	 if the first argument is a file or not.
	 If so, it will read the file as a script and
	 handle accordingly.
	 If not, it will print out the commands given
	 and exit.

	-If no external arguments are found, will
	 present a shell prompt and take in keyboard
	 commands. It will then process commands
	 until 'exit' command is given.
************************************************/

int main(int argc, char *argv[])
{
	int exitFlag = 0;
	char *input;
	char buffer[BUFFER_SIZE];
	int lineExtensionFlag = 0;
	int i;

	setbuf(stdout, NULL); //disables buffering for stdout

	struct passwd *pw = getpwuid(getuid());
	char *homedir = pw->pw_dir;
	//	char *homedir="./";
	mcshInsert(&env, "HOME", homedir);

	//init dynamic argument array
	argsV = (char **)mcshMalloc(maxArgs * sizeof(char *));
	for (i = 0; i < maxArgs; i++)
	{
		argsV[i] = (char *)mcshMalloc(BUFFER_SIZE * sizeof(char));
	}

	//.mcshrc subroutine
	{
		FILE *fp;
		char mcshrcdir[BUFFER_SIZE];
		strcpy(mcshrcdir, homedir);
		strcat(mcshrcdir, "/.mcshrc");

		if (!(fp = fopen(mcshrcdir, "r")))
		{
			printf("%s\n", mcshrcdir);
			printf("No .mcshrc file found.\n");
		}
		else
		{
			while (exitFlag != 1)
			{
				input = fgets(buffer, BUFFER_SIZE, fp);
				if (feof(fp))
				{
					break;
				}
				lineExtensionFlag = parseCommand(input, lineExtensionFlag);
				if (lineExtensionFlag == 1)
					continue;

				exitFlag = processInternalCommand();
			}
			fclose(fp);
			if (exitFlag == 1)
			{
				exitFlag = 0; //resetting for possible script use
				freeVariables();
				return 0;
			}
		}
	}

	//Looks for command line arguments.
	if (argc > 1)
	{
		FILE *fp;
		//checks to see if command line is a file
		if (!(fp = fopen(argv[1], "r"))) //if no file exists, just processes the command and exits
		{
			for (i = 1; i < argc; i++)
			{
				strcpy(argsV[i - 1], argv[i]);
				numArgs++;
			}
			processInternalCommand();
			freeVariables();
			return 0;
		}
		//if it does find a file, it will attempt to read it as a script,
		//inputting each line at a time into the shell
		else
		{
			while (exitFlag != 1)
			{
				printf("mcsh> ");
				input = fgets(buffer, BUFFER_SIZE, fp);
				if (feof(fp))
				{
					printf("\nEnd of script. Quitting..\n\n");
					freeVariables();
					return 0;
				}
				printf("%s", input);
				lineExtensionFlag = parseCommand(input, lineExtensionFlag);
				if (lineExtensionFlag == 1)
					continue;
				printf("\n");
				exitFlag = processInternalCommand();
			}
			fclose(fp);
			if (exitFlag == 1)
			{
				freeVariables();
				return 0;
			}
		}
	}

	//Main shell loop for keyboard input
	while (exitFlag != 1)
	{

		//Display prompt and get input string
		printf("mcsh> ");
		input = gets(buffer);

		//parse string into individual arguments
		lineExtensionFlag = parseCommand(input, lineExtensionFlag);
		if (lineExtensionFlag == 1)
			continue;

		//process arguments
		exitFlag = processInternalCommand();
	}
	freeVariables();
	return 0;
}

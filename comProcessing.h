#ifndef MCSH_COMMAND_PROCESSING
#define MCSH_COMMAND_PROCESSING

/**********************************************************************
Command processing functions.
Matthew Moore
Last modified: 4/20/13
All functions relating to parsing/running commands given in the command
prompt. This includes work with aliases and external commands.
**********************************************************************/


/************************************************
parseAliases(char *)
input: character string for variable in question
return: int of success
Takes the current variable inputted and checks it against the
currently stored aliases. Should it find it, it will input the alias
value into the arguement and return 1.
Otherwise, it will do nothing and return 0.
************************************************/
int parseAliases(char *var);



/************************************************
int parseCommand(char *, int)
input: character string
return: int flag for inputting another line before processing
Takes a full line of characters and parses it into
seperate commands held in the argsV array.
Also increments numArgs to keep track of it.
************************************************/
int parseCommand(char *input, int lineExtensionFlag);


/************************************************
processInternalCommand()
input: no input
return: integer
Takes argsV and compares it against all known
commands. Then deals according to which command and
which variables follow.
Returns an int of 1 if wanting to exit, 0 otherwise.
************************************************/
int processInternalCommand();


/************************************************
void *runExternalCommand(void *arguments)
input: char *arguments
return: no return
Forks program and runs whatever the inputted arguments
are.
************************************************/
void *runExternalCommand(void *arguments);

/************************************************
int whereTest(char *testPath, char *testElement, char *buffer)
input: char *testPath, char *testElement, char *buffer
return: int found
Checks inputted path and element, returning 1 if found and also
modifying buffer to actual path.
If inputted path is relative or absolute, will only check those
paths. If no path given, will check all paths in PATH env variable.
************************************************/
int whereTest(char *testPath, char *testElement, char *buffer);


/************************************************
void freeVariables
input: no input
return: no output
Free's all of the args that were allocated
************************************************/
void freeVariables();


#endif

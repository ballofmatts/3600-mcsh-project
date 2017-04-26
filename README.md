# 3600-mcsh-project
Shell built in C for school. Utilizes own subcommands, list management, and memory management

You might want to put the .mcshrc file into your home folder, or you can use you own, or none!

To compile: navigate to this directory and run 'make'.

To execute: run ./mcsh5

Program is a shell named msch.
Supports:
1. Command line execution:
   - Execute script or internal commands through command-line arguments:
     - mcsh script_file
     - mcsh command
   - Program actually does this. Instructions said to simply print commands, but program will interpret command-line arguments.
   - If command is a file, will treat it as a script and run the script.
2. Interactive execution:
   - All information entered after invoking the shell is interpreted by the shell until the command "exit" occurs.
3. Environment variables:
   - Shell supports the ability to set, display, reset, extend, and clear environment variables.
     1. Set an environment variable.
        - set var var-value
     2. Display an environment variable.
        - set var
     3. Display all environment variables.
        - set
     4. Reset an environment variable.
        - set var new-value
     5. Extend an environment variable.
        - set var @var!new-value
        - set var new-value!@var
     6. Clear an environment variable.
        - tes var
     7. Environment variable usage occurs by placing the symbol "@" immediately,	no space, in front of the variable.
4. Command aliases:
   - Shell supports the ability to set, display, reset, and clear aliases, direct or indirect.
     1. Set a command alias.
        - alias cmd cmd-value
     2. Display a command alias.
        - alias cmd
     3. Display all command aliases.
        - alias
     4. Reset a command alias.
        - alias cmd new-value
     5. Clear a command alias.
        - saila cmd
5. Internal commands:
   - The only internal commands are set, tes, alias, saila, and exit.
6. External commands:
   - Anything which is not an internal command is considered an external command.
   - External commands are then passed to the system to run, with their output displayed in the shell
7. Memory subsystem for dynamically allocating internal storage:
    1. mcshMalloc
       - Allocates space
    2. mcshCalloc
       - Allocates space and 0's the memory
    3. mcshRealloc
       - Re-allocates space
    4. mcshFree
       - Frees this variable
8. Algorithms for keeping data elements in alphabetical order:
    1. mcshInsert
       - Inserts a new element into its appropriate data structure
    2. mcshUpdate
       - Changes the value associate with either an environment variable or command alias
    3. mcshRemove
       - Removes either an environment variable or command alias
    4. mcshFind
       - Finds either an environment variable or command alias
    5. mcshHead
       - Returns the lexicographically smallest entry from either the environment variable or the command alias structures
    6. mcshNext
       - Moves to the next item, lexicographically, in either the environment variable or the command alias structures
9. Implements the comment character ~:
   - Everything following the ~ is considered to be a comment.
10.	Implements complex strings:
    - A complex string is any string between matching braces, { }.
11. The HOME directory is automatically stored as an environment variable before any other operations are performed.
12.	Opens and process the resource file .mcshrc which is found in the home directory. 
    - The resource file is allowed to contain any command/line that can be typed in interactive mode.
13.	Line extensions: 
    - The symbol "$" is interpreted as indicating that the line is continued on the next line.
    - Anything on the same line after	the "$" is ignored.
14.	Command ending:
    - A line with embedded "?" symbols will be considered to include multiple commands separated by those symbols.
15.	Implements the internal command, "echo":
    - This command will print out the remainder of the line as text.
    - The "$" can be used to extend the text to multiple lines.
    - If the "?" symbol is present after an echo command, it indicates	the end of the text to be output is the character preceding the	symbol.
    - If the "@" symbol is present, then the tag following the symbol is to be treated as an environment variable whose value is to be	inserted into the output string.
16. Implements the internal command “where” (with usage “where type name”):
    1. Determines the location of the file “name” with the permission “type”.
    2. If “name“ does not contain any “/” characters, the program will search through the PATH variable list of directories.
    3. If name does contain any “/” characters, then the program will ignore the PATH variable and look at the absolute or relative location to	determine if the permission is set correctly according to “type.”
    4. Aliases are observed.
    5. If name refers to an internal command, it is noted as such.
    6. The value of “type” is allowed to take any of the following values:
       - any
       - read
       - write
       - run
       - dir
       - file.
    7. This command returns the first qualified path to the file which meets the “type” specified by the user.  If none are found, this is reported.
17.	Implements external commands via the fork(), exec(), and wait() families of functions.
    - The user may execute an external command in any directory included in the internal environment variable PATH.
    - Concatenates the detected path with the file name before issuing the external command.
    - If the command file cannot be found in any of the directories in PATH, the program shall issue the result “command “requested command” not found.”
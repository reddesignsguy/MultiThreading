/**

 * Description: Allows you to run multiple commands at once. It is recommended to use an input file to run the program.

 * Author names: Albany Patriawan, Chirag Kaudan

 * Last modified date: October 13, 2022

 * Creation date: October 12, 2022

 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Main function of the program
int main()
{

    // Take in commands 30 characters allowed each
    int lineLen = 30;
    char line[lineLen];

    int currentLineNumber = 1; // Represent the index of the current line

    // Parse input from stdin into commands array
    while (fgets(line, lineLen, stdin) != NULL)
    {
        int pid = fork(); // Create child process

        if (pid < 0)
        { // Forking failed, exit
            exit(1);
        }
        else if (pid == 0)
        { // Child process

            // Now we must parse the current line into a command and its arguments

            // Replace new line character in line with a null character to avoid parsing issues
            for (int i = 0; i < lineLen; i++)
            {
                if (line[i] == '\n')
                {
                    line[i] = '\0'; // Replace new line character with null character
                }
            }

            int numArgs = lineLen / 2 + 1; // Max possible number of args is half the length of the input + 1
            char *argPointers[numArgs];    // Array of pointers to the arguments

            argPointers[0] = strtok(line, " "); // First argument points to text before first space
                                                // Replaces delimter, or space " ", with NULL internally
            int i = 0;

            // Parse all arguments until we reach a null character
            while (argPointers[i] != NULL)
            {
                // Make the next arg pointer point to the next argument, which is the text after the address of the last NULL placed by strtok
                argPointers[++i] = strtok(NULL, " "); // This also places NULL where the delimeter is found
            }

            int current_pid = getpid(); // child id
            int parent_pid = getppid(); // parent id

            // Set up error output file
            char errPath[15];                                             // Maximum length of file name is 15 chars
            sprintf(errPath, "%d.err", current_pid);                      // Create file name
            int err = open(errPath, O_APPEND | O_WRONLY | O_CREAT, 0777); // Create file

            dup2(err, 2); // Duplicate the error output file with stderr

            // Set up output file
            char outPath[15];                                             // Maximum length of file name is 15 chars
            sprintf(outPath, "%d.out", current_pid);                      // Create file name
            int out = open(outPath, O_APPEND | O_WRONLY | O_CREAT, 0777); // Create file

            dup2(out, 1); // Duplicate the output file with stdout

            // Start printing to .out file
            printf("Starting command %d: child %d pid of parent %d\n", currentLineNumber, current_pid, parent_pid);
            fflush(stdout);

            // Execute command
            execvp(argPointers[0], argPointers);

            // If this is printed, command has failed
            fprintf(stderr, "Failed to run command: %s\n", argPointers[0]);
            exit(2);
        }

        currentLineNumber += 1;
    }

    int child_pid; // pid of child that has finished
    int stat;      // exit code of child that has finished

    // The parent process
    while ((child_pid = wait(&stat)) > 0)
    {
        int parent_pid = getpid(); // parent pid is this pid

        char errPath[15];                                             // Maximum length of file name is 15 chars
        sprintf(errPath, "%d.err", child_pid);                        // Create file name
        int err = open(errPath, O_APPEND | O_WRONLY | O_CREAT, 0777); // Create or open file

        dup2(err, 2); // Duplicate the error output file with stderr

        char outPath[15];                                             // Maximum length of file name is 15 chars
        sprintf(outPath, "%d.out", child_pid);                        // Create file name
        int out = open(outPath, O_APPEND | O_WRONLY | O_CREAT, 0777); // Create or open file

        dup2(out, 1); // Duplicate the output file with stdout

        // If process ended by kill signal, print the kill signal
        if (WIFSIGNALED(stat))
        {
            fprintf(stderr, "Killed with signal %d", WIFSIGNALED(stat));
        }

        /// If process ended by itself, print the exit code
        else
        {
            
            fprintf(stderr, "Exited with exitcode = %d", WEXITSTATUS(stat));
        }

        printf("Finished child %d pid of parent %d", child_pid, parent_pid); // Last message to output file

        fflush(stdout);
        fflush(stderr);

        // Close FDs for this process
        close(2);
        close(1);
    };

    exit(0);
}
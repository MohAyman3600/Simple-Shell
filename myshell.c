#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for handling strings
#include <unistd.h>
#include <fcntl.h> // module for handling files

#define BUFFER_LEN 1024 // Default size for arrays looping over the input (argv, pipedArgs)

extern char **getline(); // Function reseving the input

void printDir() // Function to print current directory of the shell at the start of each shell loop
{
    char cwd[BUFFER_LEN];
    getcwd(cwd, sizeof(cwd));
    printf("\n %s >> ", cwd);
}

int interCommand(char **input) // Function to handle internal commands
{
    if (input == NULL) // when the user presses ctrl-D
    {
        printf("\n");
        exit(0);
    }
    else if (strcmp(input, "") == 0) // when the user presses Enter
    {
        return 1;
    }
    else if (strcmp(input[0], "exit") == 0) // for the exit
    {
        exit(0);
    }
    else if (strcmp(input[0], "cd") == 0) // for the change directory function
    {
        chdir(input[1]);
    }
    return 0;
}
// function to process the input and handling redirection without pipes,
// finally, put a the processed input in args array return the number of pipes. If none return 0
int processIn(char **input, char **args, char *redirFlag, int *orgFd)
{
    int pipeFlag = 0; // pipe flag to carry number of pipes
    int out = dup(1); // saving stdout adress
    int in = dup(0);  // saving stdin adress
    for (int i = 0; input[i] != NULL; ++i)
    {
        if (strcmp(input[i], "|") == 0)
        {
            pipeFlag++;
            dup2(out, 1);
            dup2(in, 0);
            continue;
        }
        else if (strcmp(input[i], ">") == 0 && strcmp(input[i + 1], "|") == 0)
        {
            continue;
        }
        else if (strcmp(input[i], "<") == 0)
        {
            *redirFlag = "read"; // flag to check for reading redirection
            *orgFd = dup(0);     // saving orginal stdin to restore later
            int file_desc = open(input[i + 1], O_RDONLY, 0777);
            dup2(file_desc, 0);
            close(file_desc);
            i = i + 1;
            continue;
        }
        else if (strcmp(input[i], ">") == 0)
        {
            *redirFlag = "write"; // flag to check for writing redirection
            *orgFd = dup(1);      // saving orginal stdout to restore later
            int file_desc = open(input[i + 1], O_CREAT | O_WRONLY | O_APPEND, 0777);
            dup2(file_desc, 1);
            close(file_desc);
            i = i + 1;
            continue;
        }
        else
        {

            args[i] = input[i];
        }
    }
    return pipeFlag;
}

// To excute commands without pipes
int excute(char **argv, char *redirfl, int orgfd)
{

    int pid = fork();
    if (pid < 0)
    {
        printf("couldn't fork");
        return 1;
    }
    else if (pid == 0)
    {

        execvp(argv[0], argv);

        perror("ERROR");
    }
    else
    {
        wait(NULL);

        // restoring the stdin or stdout depending on the flag
        if (strcmp(redirfl, "write") == 0)
        {
            dup2(orgfd, 1);
        }
        else if (strcmp(redirfl, "read") == 0)
        {
            dup2(orgfd, 0);
        }
    }
    return 0;
}

// function to excute piped commands
void ecxutePipes(char **input, int pipeFlag)
{
    int fd_new[2], fd_old[2]; // creadting file desc to handle pipes
    int index = 0;
    int file_desc;        // file desc for redirection
    int in, out;          // saving stout and stdin
    char *redir = "none"; // redirection flag
    for (int i = 0; i < pipeFlag + 1; ++i)
    {
        char **pipedArgs[BUFFER_LEN] = {NULL};
        int pipeIndex = 0;
        while (1) // loop to process input command each time
        {
            if (input[index] == NULL)
            {
                break;
            }
            else if (strcmp(input[index], "|") == 0)
            {
                break;
            }
            else if (strcmp(input[index], ">") == 0 && strcmp(input[index + 1], "|") == 0)
            {
                index++;
                break;
            }
            else if (strcmp(input[index], ">") == 0)
            {
                file_desc = open(input[index + 1], O_CREAT | O_WRONLY | O_APPEND, 0777);
                redir = "write";
                index = index + 2;
                continue;
            }
            else if (strcmp(input[index], "<") == 0)
            {
                file_desc = open(input[index + 1], O_RDONLY, 0777);
                redir = "read";
                index = index + 2;
                continue;
            }
            else
            {
                pipedArgs[pipeIndex] = input[index];
            }
            index++;
            pipeIndex++;
        }
        index++;

        pipe(fd_new); // create pipe each loop
                      //Then excute one command
        if (i == 0)
        {
            // check for redirection and handling it
            if (strcmp(redir, "write") == 0)
            {
                int out = dup(1);
                dup2(file_desc, 1);
                close(file_desc);
            }
            else if (strcmp(redir, "read") == 0)
            {
                int in = dup(0);
                dup2(file_desc, 0);
                close(file_desc);
            }

            int pid = fork();
            if (pid < 0)
            {
                printf("couldn't fork");
                break;
            }
            else if (pid == 0)
            {
                close(fd_new[0]);
                dup2(fd_new[1], 1);
                close(fd_new[1]);
                execvp(pipedArgs[0], pipedArgs);

                perror("ERROR");
            }
            else
            {
                // restoring std after redirection
                if (strcmp(redir, "write") == 0)
                {
                    dup2(out, 1);
                }
                else if (strcmp(redir, "read") == 0)
                {
                    dup2(in, 0);
                }
            }
        }
        else if (i == pipeFlag)
        {
            if (strcmp(redir, "write") == 0)
            {
                int out = dup(1);
                dup2(file_desc, 1);
                close(file_desc);
            }
            else if (strcmp(redir, "read") == 0)
            {
                int in = dup(0);
                dup2(file_desc, 0);
                close(file_desc);
            }
            int pid = fork();
            if (pid < 0)
            {
                printf("couldn't fork");
                break;
            }
            else if (pid == 0)
            {

                dup2(fd_old[0], 0);
                close(fd_old[0]);
                close(fd_old[1]);

                execvp(pipedArgs[0], pipedArgs);

                perror("ERROR");
            }
            else
            {
                // closing file desc in parent
                close(fd_old[0]);
                close(fd_old[1]);
                if (strcmp(redir, "write") == 0)
                {
                    dup2(out, 1);
                }
                else if (strcmp(redir, "read") == 0)
                {
                    dup2(in, 0);
                }
                continue;
            }
        }
        else
        {
            if (strcmp(input[index], "write") == 0)
            {
                int out = dup(1);
                dup2(file_desc, 1);
                close(file_desc);
            }
            else if (strcmp(input[index], "read") == 0)
            {
                int in = dup(0);
                dup2(file_desc, 0);
                close(file_desc);
            }

            int pid = fork();
            if (pid < 0)
            {
                printf("couldn't fork");
                break;
            }
            else if (pid == 0)
            {
                dup2(fd_old[0], 0);
                close(fd_old[0]);
                close(fd_old[1]);

                close(fd_new[0]);
                dup2(fd_new[1], 1);
                close(fd_new[1]);

                execvp(pipedArgs[0], pipedArgs);

                perror("ERROR");
            }
            else
            {
                if (strcmp(redir, "write") == 0)
                {
                    dup2(out, 1);
                }
                else if (strcmp(redir, "read") == 0)
                {
                    dup2(in, 0);
                }
                close(fd_old[0]);
                close(fd_old[1]);
            }
        }
        // use fd old as basket to carry prev excution
        fd_old[0] = fd_new[0]; 
        fd_old[1] = fd_new[1];
    }

    for (int i = 0; i < pipeFlag + 1; ++i)// wait for all childs
    {
        wait(NULL);
    }
}

int main()
{
    char **input;           // for input
    int pipfl = 0;          // pipe flag
    char *redirfl = "none"; // redirection flag
    int orgfd = 0;          // to save orginal stout and in
    while (1)
    {
        char **argv[BUFFER_LEN] = {NULL}; // flushing array for next command
        printDir();
        input = getline();
        if (interCommand(input) == 1)
            continue;
        pipfl = processIn(input, &argv, &redirfl, &orgfd);
        if (pipfl > 0)
        {

            ecxutePipes(input, pipfl);
        }
        else
        {
            if (excute(argv, redirfl, orgfd) != 0)
                break;
        }
    }
    return 0;
}

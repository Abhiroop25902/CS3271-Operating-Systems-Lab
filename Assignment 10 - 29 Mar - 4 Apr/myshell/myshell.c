/**
 * @file myshell.c
 * @author Abhiroop Mukherjee (abhiroop.m25902@gmail.com)
 * @brief shell made using fork and execvp
 * @date 2022-04-02
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <stdio.h>
#include <signal.h>  //for sigint
#include <stdbool.h> // for true and false
#include <stdlib.h>  //exit()
#include <unistd.h>  //getcwd()
#include <string.h>  //strcmp()
#include <wait.h>    //waitpid()

#define MAXLINE 256 // max line length
#define MAXARGS 128 // max number of auguments
#define PWD_SIZE 256
#define NUM_COMMAND 3 // for now support for only 2 commands + 1 NULL

char pwd[PWD_SIZE];

typedef enum CommandType
{
    SEMICOLON,
    OR,
    AND,
    NONE
} CommandType;

// hints
//  get pwd -> https://stackoverflow.com/questions/298510/how-to-get-the-current-directory-in-a-c-program

// TODO
//  pwd [-]
// exit[-]
// clear[-]
// cd [-]
// external commands [-] -> via execvp
// ; [-]
// && [-]
// || [-]
// ls after cd not working [-]
// file reading [-]

void exitShell(int signum)
{
    printf("\n");
    exit(signum);
}

// give this function a line and it will parse it to argv
void parseline(char *buf, char *argv[]) // parse command line and build argv array
{
    char *delim = NULL; // points to first space delim
    int argc = 0;

    // printf("%s", buf);
    buf[strlen(buf) - 1] = ' '; // replace trailing '\n' with space

    while (buf != NULL && (*buf == ' '))
        buf++; // skip past leading spaces

    while ((delim = strchr(buf, ' ')))
    {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;

        while (buf != NULL && (*buf == ' '))
            buf++; // skips past trailing space
    }

    argv[argc] = NULL;
}

// this will check if the command is a builtin, if yes, execute it and return true, else it till return false, which means we will have to use execvp
bool builtin_command(char *argv[])
{
    if (strcmp(argv[0], "exit") == 0)
        exit(0);
    else if (strcmp(argv[0], "pwd") == 0)
    {
        printf("%s\n", pwd);
        return true;
    }
    else if (strcmp(argv[0], "clear") == 0)
    {
        system(argv[0]); // clear screen
        return true;
    }
    else if (strcmp(argv[0], "cd") == 0)
    {
        if (chdir(argv[1]) != 0)
        {
            perror("chdir: ");
            // exit(-1); //want to just give error but not exit the shell
        }
        return true;
    }

    return false; // not a built-in command
}

// takes a command as a whole and processess it either by using inbuild method or by doing execvp
int evaluateCommand(char *cmdline)
{
    char *argv[MAXARGS]; // argument list execve()
    // char buf[MAXLINE];   // holds modified commandline
    int commandStatus = 0;

    // strcpy(buf, cmdline);
    parseline(cmdline, argv);

    if (argv[0] == NULL) // can happen if we provide "\n" only
        return commandStatus;

    if (!builtin_command(argv)) // expecting user executable
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork fail: ");
            exit(-1);
        }

        if (pid == 0) // let the child run the user program
        {
            int status = execvp(argv[0], argv);
            if (status < 0)
            {
                printf("%s: Command not found\n", argv[0]);
                exit(EXIT_FAILURE);
            }
            // exit(EXIT_SUCCESS);
        }

        if (waitpid(pid, &commandStatus, 0) < 0)
        {
            perror("waitpid error: ");
            exit(-1);
        }
    }
    // printf("status = %d\n", status);
    return commandStatus;
}

CommandType splitCmdLine(char *cmdLine, char *commands[])
{
    // printf("%s\n", cmdLine);
    char *delim;
    // char buf[MAXLINE];
    // strcpy(buf, cmdLine);

    CommandType returnVal = NONE;

    if (delim = strstr(cmdLine, ";"))
    {
        returnVal = SEMICOLON;
        *delim = '\0';
        commands[0] = cmdLine;
        commands[1] = delim + 1;
        commands[2] = NULL;
    }
    else if (delim = strstr(cmdLine, "&&"))
    {
        returnVal = AND;
        *delim = '\0';
        commands[0] = cmdLine;
        commands[1] = delim + 2;
        commands[2] = NULL;
    }
    else if (delim = strstr(cmdLine, "||"))
    {
        returnVal = OR;
        *delim = '\0';
        commands[0] = cmdLine;
        commands[1] = delim + 2;
        commands[2] = NULL;
    }
    else
    {
        commands[0] = cmdLine;
        commands[1] = NULL;
    }

    return returnVal;
}

// void clearCmdLine(char *cmdLine, int size)
// {
//     for (int i = 0; i < size; i++)
//         cmdLine[i] = '\0';
// }

// void printCommands(char *commands[])
// {
//     int idx = 0;
//
//     while (commands[idx] != NULL)
//     {
//         printf("%s\n", commands[idx++]);
//     }
// }

// this function will process the commandLine given to it
void processLine(char *cmdLine)
{
    char *commands[NUM_COMMAND];
    CommandType commandType = splitCmdLine(cmdLine, commands);

    // printf("%d", commandType);

    // switch (commandType)
    // {
    // case NONE:
    //     printf("None\n");
    //     break;
    // case SEMICOLON:
    //     printf("semicolon\n");
    //     break;
    // case AND:
    //     printf("and\n");
    //     break;
    // case OR:
    //     printf("or\n");
    //     break;

    // default:
    //     printf("erorr\n");
    //     break;
    // }

    // printCommands(commands);

    if (commandType == NONE)
    {
        evaluateCommand(commands[0]);
    }
    else if (commandType == SEMICOLON)
    {
        evaluateCommand(commands[0]);
        evaluateCommand(commands[1]);
    }
    else if (commandType == AND)
    {
        int status = evaluateCommand(commands[0]);
        if (status == 0)
            evaluateCommand(commands[1]);
    }
    else if (commandType == OR)
    {
        int status = evaluateCommand(commands[0]);
        if (status != 0)
            evaluateCommand(commands[1]);
    }
}

void interactiveMode()
{
    char cmdLine[MAXLINE];

    while (1)
    {
        // clearCmdLine(cmdLine, MAXLINE);

        printf("> ");
        fgets(cmdLine, MAXLINE, stdin); // read commandline

        if (feof(stdin))
            exit(0);

        processLine(cmdLine);
    }
}

void fileReadMode(char *filename)
{

    FILE *fin = fopen(filename, "r");
    if (fin == NULL)
    {
        perror("fopen fail: ");
        exit(EXIT_FAILURE);
    }
    char cmdLine[MAXLINE];

    while (fgets(cmdLine, MAXLINE, fin))
    {
        printf("%s", cmdLine);
        processLine(cmdLine);
    }

    fclose(fin);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, exitShell);

    if (getcwd(pwd, PWD_SIZE) == NULL)
    {
        perror("getcwd fail: ");
        exit(-1);
    }

    if (argc == 1)
        interactiveMode();
    else if (argc == 2)
        fileReadMode(argv[1]);
    else
    {
        fprintf(stderr, "Error: multiple files are not supported\n");
        exit(EXIT_FAILURE);
    }
}
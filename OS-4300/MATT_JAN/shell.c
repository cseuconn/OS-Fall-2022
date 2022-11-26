#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

int num_pipes = 0;

char*  ReadCommand()
{
	char* command = malloc(sizeof(char) * 100);
	scanf(" %[^\n]s",command);
	return command;
}


char** ParseCommand(char* command)
{
	int i = 0;
	char** listArgs = malloc(strlen(command) * sizeof(char*) + 3);
	char* arg = strtok(command," ");
	while(arg != NULL)
	{
		listArgs[i] = arg;
		arg = strtok(NULL, " ");
		++i;
	}
	listArgs[i] = NULL;
	return listArgs;
		
}

char*** SeperateProcesses(char** parsedCommand)
{
	char*** processArgs = malloc(100);
	for(int i = 0; i < 10; ++i)
		processArgs[i] = malloc(100);
	int processPos = 0;
	int i = 0;
	int j = 0;
	while(parsedCommand[i] != NULL)
	{
		if (strcmp(parsedCommand[i], "|"))
		{
			processArgs[processPos][j] = parsedCommand[i];
			j++;	
		}
		else
		{
			num_pipes++;
			processArgs[processPos][j] = NULL;
			j = 0;
			processPos++;
		}
		i++;
	}
	return processArgs;
}


void closePipes(int pipes[][2])
{
	for(int i = 0; i < num_pipes + 1; ++i)
	{
		close(pipes[i][1]);
		close(pipes[i][0]);
	}
				
}

void printArt()
{
	printf("%s\n"," _______  __   __  _______  ___      ___     ");
	printf("%s\n","|       ||  | |  ||       ||   |    |   |    ");
	printf("%s\n","|  _____||  |_|  ||    ___||   |    |   |    ");
	printf("%s\n","| |_____ |       ||   |___ |   |    |   |    ");
	printf("%s\n","|_____  ||       ||    ___||   |___ |   |___ ");
	printf("%s\n"," _____| ||   _   ||   |___ |       ||       |");
	printf("%s\n","|_______||__| |__||_______||_______||_______|");
}

// note: C provides a popen() and also a system() command which handles the forking and exec for you
// we decided to not use this and do it manually for the sake of learning 
void ExecuteCommand(char* command)
{
	char** listArgs = ParseCommand(command); // represents entire command ex. {"git","branch","|","grep","master"}
	char*** processArgs = SeperateProcesses(listArgs); //3d array, each entry is an array of each process args ex. { {"git","branch"} , {"grep","masater"} } also counts how many pipes we need
	int pipes[num_pipes][2];
	for(int i = 0; i < num_pipes + 1; ++i)
		pipe(pipes[i]);
	pid_t pids[num_pipes + 1];
	for(int i = 0; i < num_pipes + 1; ++i)
	{
		pids[i] = fork();
		if (pids[i] == 0)
		{

			if (i == 0 && num_pipes != 0)
			{
				dup2(pipes[i][1], 1);
				closePipes(pipes);
				int code = execvp(processArgs[i][0], processArgs[i]);
				if (code < 0)
				{
					fprintf(stderr, "invalid command!");
					_exit(1);
				}
			}
			else if(i < num_pipes && i != 0)
			{
				dup2(pipes[i][1],1);
				dup2(pipes[i-1][0],0);
				closePipes(pipes);
				int code = execvp(processArgs[i][0], processArgs[i]);
				if (code < 0)
				{
					fprintf(stderr, "invalid command!");
					_exit(1);
				}
				
			}
			else
			{
				if(num_pipes > 0)
					dup2(pipes[i - 1][0],0);
				closePipes(pipes);	
				int code = execvp(processArgs[i][0], processArgs[i]);
				if (code < 0)
				{
					fprintf(stderr, "invalid command!");
					_exit(1);
				}
					
			}
		}	
	}
	closePipes(pipes);	
	for(int i = 0; i < num_pipes + 1; ++i)
		waitpid(pids[i], NULL, 0);
	free(listArgs);
	for(int i = 0; i < 10; ++i)
		free(processArgs[i]);
	free(processArgs);           
}


int main()
{
	
	printf("%s\n","CSE 4300 PROJECT");
	printArt();
	char **history = malloc(100);
	int num_commands = 0;
	printf("%s\n\n\n","Created by: Matt Beauvais & Jan Feyen");
	while(1)
	{
		num_pipes = 0;
		printf("\n%s", "> ");
		char* command = ReadCommand();
		history[num_commands] = malloc(strlen(command) + 1); 
		strcpy(history[num_commands],command);
		num_commands++;
		if (!strcmp(command,"exit"))
		{
			free(command);
			break;
		}
		else if(!strcmp(command,"history"))
		{
			for(int i = 0; i < num_commands - 1; ++i) 
				printf("%s\n",history[i]);
		}
		else
			ExecuteCommand(command);
			free(command);
	}
		for(int i = 0; i < num_commands; ++i)
			free(history[i]);
		free(history);
}

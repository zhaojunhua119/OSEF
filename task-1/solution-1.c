

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<sys/wait.h>
#define MAX_ARGS 64

void execFile(char* inputFile);
void parse(char *line, char **argv);
void  execute(char **argv);

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("error, argc = %d\n", argc);
	}
	char* inputFile = argv[1];
	printf("%s\n", inputFile);
	execFile(inputFile);
	return EXIT_SUCCESS;
}

void execFile(char* inputFile) {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	char* argv[MAX_ARGS];

	fp = fopen(inputFile, "r");
	if (fp == NULL)
		exit(EXIT_FAILURE);

	while ((read = getline(&line, &len, fp)) != -1) {
		printf("executing %s", line);
		parse(line, argv);
		execute(argv);
	}

	fclose(fp);
	if (line)
		free(line);
	exit(EXIT_SUCCESS);
}

void parse(char *line, char **argv)
{
     while (*line != '\0' && *line != '\n') {
          while (*line == ' ' || *line == '\t' || *line == '\n')
               *line++ = '\0';
          *argv++ = line;
          while (*line != '\0' && *line != ' ' &&
                 *line != '\t' && *line != '\n')
               line++;
          if (*line == '\n')
        	  *line = '\0';
     }
     *argv = '\0';
}

void  execute(char **argv)
{
     pid_t  pid;
     int    status;

     if ((pid = fork()) < 0) {
          printf("*** ERROR: forking child process failed\n");
          exit(1);
     }
     else if (pid == 0) {
    	  printf("%s,%s,%s\n",argv[0],argv[1],argv[2]);

          if (execvp(*argv, argv) < 0) {
               printf("*** ERROR: exec failed\n");
               exit(1);
          }
     }
     else {
          while (wait(&status) != pid)
               ;
     }
}

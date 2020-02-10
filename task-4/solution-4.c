/* Your code goes here */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT 1024


int main(int argc, char *argv[]) {
	char command[MAX_INPUT] = {};

	while (1) {
		scanf("%s", command);
		int tokenidx = 0;
		char *pch = strtok(command, " \t");
		while (pch != NULL) {
			if (tokenidx == 0 && strcmp("exit", pch) == 0)
				exit(EXIT_SUCCESS);
			printf("%s\n", pch);
			pch = strtok(NULL, " \t");
		}
	}
	return EXIT_SUCCESS;
}

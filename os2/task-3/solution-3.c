/* Your code goes here */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BIN 1024
#define MAX_HEX 1024
#define BIN_SIZE 4

const char HEX_SET[] = {'0', '1', '2', '3', '4', '5',
		'6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

void doconvertion(const char* inputFile,const char* outputFile);
void bintohex(char* hex, const char* bin);

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("An error has occurred\n");
		exit(EXIT_FAILURE);
	}
	const char* inputFile = argv[1];
	const char* outputFile = argv[2];

	doconvertion(inputFile, outputFile);
	return 0;
}

void doconvertion(const char* inputFile,const char* outputFile) {
	FILE * fpIn = NULL;
	FILE * fpOut = NULL;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	char output[MAX_HEX + 1] = {};

	fpIn = fopen(inputFile, "r");
	fpOut = fopen(outputFile, "w");
	if (fpIn == NULL || fpOut == NULL) {
		printf("An error has occurred\n");
		exit(1);
	}

	while ((read = getline(&line, &len, fpIn)) != -1) {
		bintohex(output, line);
		fprintf(fpOut, "%s\n", output);
	}

	fclose(fpIn);
	fclose(fpOut);
	if (line)
		free(line);
	exit(EXIT_SUCCESS);
}

void bintohex(char* hex, const char* bin) {
	char *hc = hex;
	const char *bc = bin;
	int cnt = 0;
	int hexvalue = 0;
	while (*bc != '\0' && *bc != '\n') {
		if (*bc < '0' || *bc > '1') {
			printf("An error has occurred\n");
			exit(EXIT_FAILURE);
		}
		hexvalue |= (*bc - '0') << (BIN_SIZE - cnt % BIN_SIZE - 1);
		if ((cnt + 1) % BIN_SIZE == 0) {
			*(hc++) = HEX_SET[hexvalue];
			hexvalue = 0;
		}
		bc++;
		cnt++;
	}
	*hc='\0';
}

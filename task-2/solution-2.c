#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_HEX 1024
#define MAX_BIN (MAX_HEX * 4)
#define BIN_SIZE 4

const char HEX_SET[] = {'0', '1', '2', '3', '4', '5',
		'6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

const char BIN_SET[] = {'0', '1'};

void doconvertion(const char* inputFile,const char* outputFile);
void hextobin(const char* hex, char* bin);

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
	char output[MAX_BIN + 1] = {};

	fpIn = fopen(inputFile, "r");
	fpOut = fopen(outputFile, "w");
	if (fpIn == NULL || fpOut == NULL) {
		printf("An error has occurred\n");
		exit(1);
	}

	while ((read = getline(&line, &len, fpIn)) != -1) {
		hextobin(line, output);
		fprintf(fpOut, "%s\n", output);
	}

	fclose(fpIn);
	fclose(fpOut);
	if (line)
		free(line);
	exit(EXIT_SUCCESS);
}

void hextobin(const char* hex, char* bin) {
	const char *hc = hex;
	char *bc = bin;
	while (*hc != '\0' && *hc != '\n') {
		int hexvalue = -1;
		if (*hc >= '0' && *hc <= '9')
			hexvalue = *hc - '0';
		else if (*hc >= 'a' && *hc <= 'f')
			hexvalue = *hc - 'a' + 10;
		else {
			printf("An error has occurred\n");
			exit(EXIT_FAILURE);
		}
		for (int i = BIN_SIZE - 1; i >= 0; i--) {
			*(bc + i) = BIN_SET[hexvalue & 1];
			hexvalue >>= 1;
		}
		bc += BIN_SIZE;
		hc++;
	}
	*bc='\0';
}

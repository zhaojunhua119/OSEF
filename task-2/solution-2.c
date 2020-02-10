#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_HEX 1024
#define MAX_BIN (MAX_HEX * 4)
#define HEX_MAP_SIZE 255
#define HEX_MAP_BIN_SIZE 4

const char HEX_SET[] = {'0', '1', '2', '3', '4', '5',
		'6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

const char BIN_SET[] = {'0', '1'};

char HEX_MAP[HEX_MAP_SIZE][HEX_MAP_BIN_SIZE + 1];

void doconvertion(const char* inputFile,const char* outputFile);
void inithexmap();
void hextobin(const char* hex, char* bin);

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("An error has occurred\n");
		exit(EXIT_FAILURE);
	}
	inithexmap();
	const char* inputFile = argv[1];
	const char* outputFile = argv[2];

	doconvertion(inputFile, outputFile);
	return 0;
}
void inithexmap() {
	memset(HEX_MAP, 0, sizeof(HEX_MAP));
	for (int i = 0; i < sizeof(HEX_SET) / sizeof(char); i++) {
		int remain = i;
		for(int j = HEX_MAP_BIN_SIZE - 1; j >= 0; j--) {
			HEX_MAP[(int)HEX_SET[i]][j] = BIN_SET[remain & 1];
			remain >>= 1;
		}
	}
}
void doconvertion(const char* inputFile,const char* outputFile) {
	FILE * fpIn = NULL;
	FILE * fpOut = NULL;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	char output[MAX_BIN + 1];

	fpIn = fopen(inputFile, "r");
	fpOut = fopen(outputFile, "w");
	if (fpIn == NULL || fpOut == NULL) {
		printf("An error has occurred\n");
		exit(1);
	}

	while ((read = getline(&line, &len, fpIn)) != -1) {
		memset(output, 0, MAX_BIN + 1);
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
		if (HEX_MAP[(int)*hc] == NULL) {
			printf("An error has occurred\n");
			exit(EXIT_FAILURE);
		}
		memcpy(bc, HEX_MAP[(int)*hc], HEX_MAP_BIN_SIZE);
		bc += HEX_MAP_BIN_SIZE;
		hc++;
	}
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "sort.h"


void
usage(char *prog)
{
    fprintf(stderr, "usage: %s inputfile outputfile\n", prog);
    exit(1);
}

void readfile(const char *inputpath, rec_t *recs, int *numrec) {
	int fd = open(inputpath, O_RDONLY);
	if (fd < 0) {
		printf("Error: Cannot open file %s\n", inputpath);
		exit(EXIT_FAILURE);
	}
	*numrec = 0;
	while (1) {
		int rc;
		rc = read(fd, recs++, sizeof(rec_t));
		if (rc == 0) // 0 indicates EOF
			break;
		if (rc < 0) {
			printf("Error: Cannot open file %s\n", inputpath);
			exit(EXIT_FAILURE);
		}
		(*numrec)++;
	}
	close(fd);
}

int compare (const void *recsRef1, const void *recsRef2) {
	return (**(rec_t**)recsRef1).key - (**(rec_t**)recsRef2).key;
}

void writefile(const char *outputpath, rec_t **recsRef, int numrec) {
	int fd = open(outputpath, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
	if (fd < 0) {
		printf("Error: Cannot open file %s\n", outputpath);
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < numrec; i++) {
		int rc = write(fd, *(recsRef++), sizeof(rec_t));
		if (rc != sizeof(rec_t)) {
	    	printf("An error has occurred\n");
	    	exit(EXIT_FAILURE);
		}
	}
	close(fd);
}
int main(int argc, char *argv[]) {
	if (argc != 3) {
		usage(argv[0]);
	}
	const char *inputpath = argv[1];
	const char *outputpath = argv[2];

	struct stat fileStat;
    if(stat(inputpath, &fileStat) < 0) {
    	printf("Error: Cannot open file %s\n", inputpath);
    	exit(EXIT_FAILURE);
    }

    rec_t *recs = malloc(fileStat.st_size);
    if (recs == NULL) {
    	printf("An error has occurred\n");
    	exit(EXIT_FAILURE);
    }
    int numrec = 0;
    readfile(inputpath, recs, &numrec);

    //sort by reference
    rec_t **recsRef = malloc(sizeof(rec_t*) * numrec);
    for (int i = 0; i < numrec; i++)
    	recsRef[i] = &recs[i];
    qsort(recsRef, numrec, sizeof(rec_t*), compare);

    writefile(outputpath, recsRef, numrec);

    if (recsRef)
    	free(recsRef);

    if (recs)
    	free(recs);
}

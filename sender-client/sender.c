#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen("todo", "r");
	if (fp == NULL)
	   exit(EXIT_FAILURE);

	while ((read = getline(&line, &len, fp)) != -1) {
	   printf("Retrieved line of length %zu :\n", read);
	   printf("%s", line);
	}

	if (line)
	   free(line);
	exit(EXIT_SUCCESS);
}

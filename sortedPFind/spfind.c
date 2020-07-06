/*******************************************************************************
 * Name        : pfind.c
 * Author      : Luke McEvoy & Blake Marpet
 * Date        : March 8, 2020
 * Description : Sorted pfind using fork, exec, pipe, write, read.
 * Pledge      : I pledge my Honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>

bool starts_with(const char *str, const char *prefix) {

	for (int i = 0; i < strlen(prefix); i++) {
		if (str[i] != prefix[i]) {
			return false;
		}
	}
	
	return true;
}

int main(int argc, char **argv) {
	
	int pfind_to_sort[2], sort_to_parent[2];

	if (pipe(sort_to_parent) < 0) {
		fprintf(stderr, "Error: pipe sort_to_parent failed. %s\n", 
			strerror(errno));
		return EXIT_FAILURE;
	}

	pid_t pID[2];

	if ((pID[0] = fork()) == 0) {

		if (pipe(pfind_to_sort) < 0) {
			fprintf(stderr, "Error: pipe pfind_to_sort failed. %s\n",
				strerror(errno));
			return EXIT_FAILURE;
		}

		if ((pID[1] = fork()) == 0) {
			
			close(pfind_to_sort[0]);
			
			if (dup2(pfind_to_sort[1], STDOUT_FILENO) < 0) {
				fprintf(stderr, "Error: pfind_to_sort dup2 failed. %s\n",
					strerror(errno));
				return EXIT_FAILURE;
			}

			if (execv("./pfind", argv) == -1) {
				fprintf(stderr, "Error: pfind failed. %s\n", strerror(errno));
				return EXIT_FAILURE;
			}

		} else {

			close(pfind_to_sort[1]);

			if (dup2(pfind_to_sort[0], STDIN_FILENO) < 0) {
				fprintf(stderr, "Error: pfind_to_sort dup2 failed. %s\n",
					strerror(errno));
				return EXIT_FAILURE;
			}

			close(sort_to_parent[0]);

			if (dup2(sort_to_parent[1], STDOUT_FILENO) < 0) {
				fprintf(stderr, "Error: sort_to_parent dup2 failed. %s\n",
					strerror(errno));
				return EXIT_FAILURE;
			}

			if ((execlp("sort", "sort", NULL)) == -1) {
				fprintf(stderr, "Error: sort failed. %s\n",
					strerror(errno));
				return EXIT_FAILURE;
			}
		}



	} else {

		close(sort_to_parent[1]);

		if (dup2(sort_to_parent[0], STDIN_FILENO) < 0) {
			fprintf(stderr, "Error: sort_to_parent dup2 failed. %s\n",
				strerror(errno));
			return EXIT_FAILURE;
		}

		char string[1];
		int matches = 0;
		char buf[PATH_MAX];
		int bytes_read;
		int index;
		while ((bytes_read = read(STDIN_FILENO, &string, 1)) > 0) {
			write(STDOUT_FILENO, &string, bytes_read);
			if (string[0] == '\n') {
				index = 0;
				if (!starts_with(buf, "Usage:")) {
					matches++;
				}
			}
			buf[index] = string[0];
			index++;
		}

		if (matches != 0) {
			printf("Total matches: %d\n", matches);
		}

		if (bytes_read == -1) {
			perror("read()");
			exit(EXIT_FAILURE);
		}

		int status;
		if (wait(&status) < 0) {
			fprintf(stderr, "Error: Wait failed %s\n", strerror(errno));
			return EXIT_FAILURE;
		}

		if (WEXITSTATUS(status)) {
			return EXIT_SUCCESS;
		} else {
			return EXIT_FAILURE;
		}
	}
}

/*
Elliot OH code example
	char buf[4096];
	close(p[1]);
	int read_bytes = read(p[0], buf, 4096);
	buf[read_bytes] = '\0';
	//	OR	printf("Read from pipe:\n%s\n", buf)
	write(STDOUT_FILENO, "Read from pipe:\n", strlen("Read from pipe:\n"));
	write(STDOUT_FILENO, buf, strlen(buf));
	close(p[0]);
*/


/*	Debugging
	

Receieved
	input:	./spfind
	output:	Usage: ./spfind -d <directory> -p <permissions string> [-h]
			Total matches: 1
Expected
	input:	./spfind
	output: Usage: ./spfind -d <directory> -p <permissions string> [-h]



Receieved
	input:	./spfind -d
	output:	Error: Required argument -d <directory> not found.
			Total matches: 0
Expected
	input:	./spfind -d
	output: Error: Required argument -d <directory> not found.



Receieved
	input:	./spfind -p
	output:	Error: Required argument -d <directory> not found.
			Total matches: 0
Expected
	input:	./spfind -p
	output: Error: Required argument -d <directory> not found.



Receieved
	input:	./spfind -l
	output:	Error: Unknown option '-l' received.
			Total matches: 0
Expected
	input:	./spfind -l
	output: Error: Unknown option '-l' received.



Receieved
	input:	./spfind -d ~/Desktop/danger -p rw-r--r--
	output:	Error: Cannot open directory '/home/cs392/Desktop/danger'. Permission denied.
			Total matches: 0
Expected
	input:	./spfind -d ~/Desktop/danger -p rw-r--r--
	output: Error: Cannot open directory '/home/cs392/Desktop/danger'. Permission denied.



Receieved
	input:	./spfind -d ~/shared -p ---------
	output:	Total matches: 0
Expected
	input:	./spfind -d ~/shared -p ---------
	output: Total matches: 0


*/









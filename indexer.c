#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <ctype.h>
#include "freq_list.h"


char *remove_punc(char *);

/* index_file returns a pointer to linked list of nodes where each node
* contains a word and count of the number of occurrences of the node.
*/

Node *index_file(Node *head, char *fname, char **filenames) {
	char line[MAXLINE];
	char *marker, *token;
	int countlines = 0;
	FILE *fp;
	if((fp = fopen(fname, "r")) == NULL) {
		perror(fname);
		exit(1);
	}
	while((fgets(line, MAXLINE, fp)) != NULL) {
		countlines++;
		if((countlines % 1000) == 0) {
			printf("processed %d lines (%d words) from %s\n", countlines, num_words, fname);
		}
		line[strlen(line)-1] = '\0';
		marker = line;
		while((token = strsep(&marker, " \t")) != NULL) {
			token = remove_punc(token);
			if((strlen(token) <= 3) || isdigit(*token)) {
			    	continue;
			}
			if(*token != '\0') {

				head = add_word(head, filenames, token, fname);
			}
		}
	}
	return head;
}

int main(int argc, char **argv) {
	Node *head = NULL;
	char **filenames = init_filenames();
	char ch;
	char *indexfile = "index";
	char *namefile = "filenames";
	char indexpath[PATHLENGTH], namepath[PATHLENGTH];
	char dirname[PATHLENGTH] = ".";
	char path[PATHLENGTH];

	while((ch = getopt(argc, argv, "i:n:d:")) != -1) {
		switch (ch) {
			case 'i':
			indexfile = optarg;
			break;
			case 'n':
			namefile = optarg;
			break;
			case 'd':
			strncpy(dirname, optarg, PATHLENGTH);
			dirname[PATHLENGTH-1] = '\0'; 
			break;
			default:
			fprintf(stderr, "Usage: indexer [-i FILE] [-n FILE ] [-d DIRECTORY_NAME]\n");
			exit(1);
		}
	}
	DIR *dir;
	if((dir = opendir(dirname)) == NULL) {
		perror("opendir");
		exit(1);
	}
	struct dirent *dp;
	while((dp = readdir(dir)) != NULL) {
		if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0 ||
		    strcmp(dp->d_name, ".svn") == 0) {
			continue;
		}
		strncpy(path, dirname, PATHLENGTH);
		strncat(path, "/", PATHLENGTH-strlen(path) - 1);
		strncat(path, dp->d_name, PATHLENGTH-strlen(path) - 1);
		printf("Indexing: %s\n", path);
		head = index_file(head, path, filenames);
	}
		strncpy(indexpath, dirname, PATHLENGTH);
		strncat(indexpath, "/", PATHLENGTH-strlen(indexpath) - 1);
		strncat(indexpath, indexfile, PATHLENGTH-strlen(indexpath) - 1);

strncpy(namepath, dirname, PATHLENGTH);
		strncat(namepath, "/", PATHLENGTH-strlen(namepath) - 1);
		strncat(namepath, namefile, PATHLENGTH-strlen(namepath) - 1);

	write_list(namepath, indexpath, head, filenames);
	return 0;
}


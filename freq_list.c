/* The functions operate on a linked list of words.  Each element of the
* list contains a word, and an array that stores the frequency of the
* word for each file in which the word is found.  The name of the file that
* is analyzed is stored in an array of file names.  The array in a
* linked list node is a parallel array to the array of file names.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freq_list.h"
int num_words = 0;

/* Allocate and initialize a new node for the list.
*/
Node *create_node(char *word, int count, int filenum) {
	Node *newnode;
	if((newnode= malloc(sizeof(Node))) == NULL) {
		perror("create_node:");
		exit(1);
	}

	strncpy(newnode->word, word, MAXWORD);
	/*make sure it is null terminated */
	newnode->word[MAXWORD-1] = '\0';

        /* memset is a function to fill memory with a constant byte
           used to initialize memory
        */ 
	memset(newnode->freq, 0, MAXFILES * sizeof(int));
	newnode->freq[filenum] = count;
	newnode->next = NULL;
	return newnode;
}

/* Increment the frequency of "word" for the file "fname" in the list
* pointed to by "head".  If the word is in the list, uses the filenames
* array to determine which element of the freq array for that word
* should be incremented. If the word is not in the list, add it in
* alphabetical order and set the frequency of the word in the file
* fname to 1.
*/
Node *add_word(Node *head, char **filenames, char *word, char *fname) {
	Node *cur = head;
	Node *prev = head;
	int filenum = get_filenum(fname, filenames);

	//List is empty or this is to be the first word
	if(!cur || (strcmp(cur->word, word)) > 0) {
		head = create_node(word, 1, filenum);
		head->next = cur;
		num_words++;
		return head;
	}

	/* look for the word */
	while(cur != NULL) {
		if((strcmp(cur->word, word)) == 0) {
		/* found word */

			cur->freq[filenum] += 1;
			return head;
		} else if ((strcmp(cur->word, word)) > 0) {
		/* need to insert word */
				prev->next = create_node(word, 1, filenum);
				prev->next->next = cur;
				num_words++;
				return head;
			}

		prev = cur;
		cur = cur->next;
	}
		if (cur == NULL){ // word is last
			num_words++;
			prev->next = create_node(word, 1, filenum);
	}
return head;
}

/* Print the list to standard output in a readable format. 
* (Primarily useful for debugging purposes.)
*/

void display_list(Node *head, char **filenames) {
	Node *cur = head;
	int i;
	while(cur != NULL) {
		printf("%s\n", cur->word);
		for(i = 0; i < MAXFILES && filenames[i]; i++) {
				printf("    %d %s ", cur->freq[i], filenames[i]);
		}
printf("\n");
		cur = cur->next;
	}
}

/* Print the linked list of words to two files.  The array of file names
* will be written one line per file in text format to namefile.  The
* linked list will be written to the file listfile in binary format.
*/
void write_list(char *namefile, char *listfile, Node *head, char **filenames) {
	Node *cur = head;
	int i;

	/* Write out the linked list */
	FILE *list_fp;
	if((list_fp = fopen(listfile, "w")) == NULL) {
		perror("List file");
		exit(1);
	}
	while (cur != NULL) {
                 /* fwrite is a function similar to the write function we have seen in lecture
                    except that it works on FILE * instead of file descriptors;
                    it is used to write binary output (rather than characters).
                 */
		fwrite(cur, sizeof(Node), 1, list_fp);
		cur = cur->next;
	}
	if(fclose(list_fp)) {
		perror("fclose");
		exit(1);
	}

	/* Write the file names array */
	FILE *fname_fp;
	if((fname_fp = fopen(namefile, "w")) == NULL) {
		perror("Name file");
		exit(1);
	}
	for(i = 0; i < MAXFILES && filenames[i]; i++) {
		fprintf(fname_fp, "%s\n", filenames[i]);
	}
	if(fclose(fname_fp)) {
		perror("fclose");
		exit(1);
	}
}

/* Populate the linked list and filenames data structures with data
* stored in two files.  The data in namefile is used to construct the
* filenames array, and the data in listfile is used to construct a
* linked list.  Note that filenames must point to an array of the
* correct size, but that head does not point to a list node when it is
* passed in.
*/
void read_list(char *listfile, char *namefile, 
			Node **head, char **filenames) {

	/* Read in the linked list */
	FILE *list_fp;
	if((list_fp = fopen(listfile, "r")) == NULL) {
		perror("List file");
		exit(1);
	}

	Node *cur = malloc(sizeof(Node));
	Node *prev = NULL;
        /* fread is a function similar to the read function we have seen in lecture
           except that it works on FILE * instead of file descriptors;
           it is used to read binary input (rather than characters).
        */
	if((fread(cur, sizeof(Node), 1, list_fp)) == 0) {
		free(cur);
		*head = NULL;
		return;
	}
	*head = cur;
	do { 
		cur->next = NULL;
		if(cur == *head) {
			prev = cur;
		} else {
			prev->next = cur;
			prev = cur;
		}
		cur = malloc(sizeof(Node));
	} while((fread(cur, sizeof(Node), 1, list_fp)) != 0);
	free(cur);
	if((fclose(list_fp))) {
		perror("fclose");
		exit(1);
	}

	/* Read in the file names */
	FILE *fname_fp;
	if((fname_fp = fopen(namefile, "r")) == NULL) {
		perror("Name file");
		exit(1);
	}
	char line[MAXLINE];
	int i = 0;
	while((fgets(line, MAXLINE, fname_fp)) != NULL) {
		line[strlen(line)-1] = '\0';
		char *name = malloc(strlen(line) + 1);
		strncpy(name, line, (strlen(line) + 1));
		filenames[i] = name;
		i++;
	}
	if((fclose(fname_fp))) {
		perror("fclose");
		exit(1);
	}
}

/* Create an array to hold filenames and initialize it to all NULL 
*/

char **init_filenames() {
	int i;
	char **fnames = malloc(MAXFILES * sizeof(char *));
	for(i = 0; i < MAXFILES; i++) {
		fnames[i] = NULL;
	}
	return fnames;
}

/* If fname is in the filenames array, then return its index.
* Otherwise add the filename to the array and return the new index.
* Currently implemented as a linear search.
*/

int get_filenum(char *fname, char **filenames) {
	int i;
	for(i = 0; i < MAXFILES; i++) {
		if(filenames[i] == NULL) {
			filenames[i] = malloc(strlen(fname+1));
			strncpy(filenames[i], fname, strlen(fname)+1);
			return i;
		}
		if((strcmp(fname, filenames[i])) == 0) {
			return i;
		}
	}
	fprintf(stderr, "Too many files\n");
	exit(1);
}

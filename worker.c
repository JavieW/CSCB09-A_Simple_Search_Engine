#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include "freq_list.h"
#include "worker.h"

/* The function get_word should be added to this file */
FreqRecord *get_word(char *word, Node *head, char **filenames) {
	Node *cur = head;	
	FreqRecord *FreqRecords;
	if ((FreqRecords = malloc(MAXFILES*sizeof(FreqRecord))) == NULL) {
		perror("create_FreqRecords:");
		exit(1);
	}
	
	// look for the word in the list
	int i = 0;
	while(cur != NULL) {
		// found the word
		if((strcmp(cur->word, word)) == 0) {
			// filling the FreqRecords
			int j;
			for (j=0 ; j < MAXFILES ;j++) {
				char *fname = filenames[j];
				// if the finished checking the last file				
				if (fname == NULL)
					break; 		
				// if the word has at least one occurrence in the file
				if (cur->freq[j] != 0) {
					FreqRecord *NewFreqRecord;
					if((NewFreqRecord = malloc(sizeof(FreqRecord))) == NULL) {
						perror("create_NewFreqRecord:");
						exit(1);
					}
					NewFreqRecord->freq = cur->freq[j];
					strncpy(NewFreqRecord->filename, fname, PATHLENGTH);
					// make sure it is null terminated
					NewFreqRecord->filename[PATHLENGTH-1] = '\0';
					// add the NewFreqRecord to FreqRecords and increment i
					FreqRecords[i] = *NewFreqRecord;
					i++;
				}
			}
		// word not in the list since the list is ordered by alphabet	
		} else if ((strcmp(cur->word, word)) > 0)
			break;
		cur = cur->next;
	}
	
	// add the last FreqRecord which have the freq 0
	FreqRecord *LastFreqRecord;
	if((LastFreqRecord= malloc(sizeof(FreqRecord))) == NULL) {
		perror("create_LastFreqRecord:");
		exit(1);
	}
	LastFreqRecord->freq = 0;
	FreqRecords[i] = *LastFreqRecord;

	return FreqRecords;
}

/* Print to standard output the frequency records for a word.
* Used for testing.
*/
void print_freq_records(FreqRecord *frp) {
	int i = 0;
	while(frp != NULL && frp[i].freq != 0) {
		printf("%d    %s\n", frp[i].freq, frp[i].filename);
		i++;
	}
}

/* run_workerg
* - load the index found in dirname
* - read a word from the file descriptor "in"
* - find the word in the index list
* - write the frequency records to the file descriptor "out"
*/
void run_worker(char *dirname, int in, int out){

	//////////////debug print//////////////////////// 
	// printf("You are searching in: %s\n", dirname);

	// load the index fpind in direname
	Node *head = NULL;
	char **filenames = init_filenames();
	char listfile[PATHLENGTH];	
	char namefile[PATHLENGTH];
	
	strncpy(listfile, dirname, PATHLENGTH - 1);
	strncat(listfile, "/index", PATHLENGTH - strlen(listfile) - 1);
	
	strncpy(namefile, dirname, PATHLENGTH - 1);
	strncat(namefile, "/filenames", PATHLENGTH - strlen(namefile) - 1);
	
	read_list(listfile, namefile, &head, filenames);
	
	while (1) {
		// read a word a time from the file descriptor "in" until it
		int read_in;
		char buf[MAXWORD];
		strncpy(buf, "\0", MAXWORD);	// let each entry of buf be '\0'
		if ((read_in = read(in, buf, MAXWORD)) == -1) {
			perror("read from in");
			exit(1);
		} else if (read_in == 0 )
			break;

		// remove the "\n" exist in the buf
		char word[MAXWORD];
		strncpy(word, "\0", MAXWORD);  // let each entry of word be '\0'
		strncpy(word, buf, strlen(buf)-1);
		strcat(word, "\0");
		
		// find the word in the index list
		FreqRecord *FreqRecords;
		FreqRecords = get_word(word, head, filenames);

		/////////debug print//////////////////////////
		// print_freq_records(FreqRecords);

		// write the frequency records to the file descriptor "out"
		int i = 0;
		while(FreqRecords != NULL && FreqRecords[i].freq != 0) {
			if (write(out, &FreqRecords[i], sizeof(FreqRecord)) == -1) {
				perror("write in run_worker: ");
				exit(1);
			}
			i++;
		}
		// notice the master that there is no more data to send by sending an FreqRecord with freq 0
		if (write(out, &FreqRecords[i], sizeof(FreqRecord)) == -1) {
				perror("write in run_worker: ");
				exit(1);
			}
		free (FreqRecords);

		////////debug print/////////////////////////
		//printf("\n");
	}
}

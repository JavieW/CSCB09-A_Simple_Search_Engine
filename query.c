#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include "freq_list.h"
#include "worker.h"



int main(int argc, char **argv) {
	
	char ch;
	char path[PATHLENGTH];
	char *startdir = ".";

	while((ch = getopt(argc, argv, "d:")) != -1) {
		switch (ch) {
			case 'd':
			startdir = optarg;
			break;
			default:
			fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME]\n");
			exit(1);
		}
	}
	// Open the directory provided by the user (or current working directory)
	
	DIR *dirp;
	if((dirp = opendir(startdir)) == NULL) {
		perror("opendir");
		exit(1);
	} 
	
	// for each valid subdirectory, creat a worker process
	// and connect with master process with 2 pipes
	pid_t workers[MAXFILES];
    int mw_pipes[MAXFILES][2];  // master to worker
    int wm_pipes[MAXFILES][2];  // worker to master
	int i;
	struct dirent *dp;
	while((dp = readdir(dirp)) != NULL) {

		if(strcmp(dp->d_name, ".") == 0 || 
		strcmp(dp->d_name, "..") == 0 ||
		strcmp(dp->d_name, ".svn") == 0){
			continue;
		}
		strncpy(path, startdir, PATHLENGTH);
		strncat(path, "/", PATHLENGTH - strlen(path) - 1);
		strncat(path, dp->d_name, PATHLENGTH - strlen(path) - 1);

		struct stat sbuf;
		if(stat(path, &sbuf) == -1) {
			//This should only fail if we got the path wrong
			// or we don't have permissions on this entry.
			perror("stat");
			exit(1);
		} 

		// Only call run_worker only if it is a directory
		if (!S_ISDIR(sbuf.st_mode)) continue;
		
		pipe( mw_pipes[i] );
        pipe( wm_pipes[i] );
        workers[i] = fork();
		if ( workers[i] == -1 ) {				//error
			perror("fork(): ");
			exit(1);
		} else if ( workers[i] == 0 ) {  		// worker
            close( mw_pipes[i][1] );
            close( wm_pipes[i][0] );
            run_worker( path, mw_pipes[i][0], wm_pipes[i][1] );
			////////////debug print/////////////////////////
			//printf("child %i ready to exit!\n", i);
            exit(0);
        } else {								// master
            close( mw_pipes[i][0] );
            close( wm_pipes[i][1] );
        }
		i++;
	}

	// InitRecord is the FreqRecord with freq 0
	FreqRecord InitRecord[sizeof(FreqRecord)];
	InitRecord->freq = 0;

	// loop exit when stdin closed
    while (1) {
        // read a word from stdin
        int read_word;
        char word[MAXWORD];
		strncpy(word, "\0", MAXWORD); // let each entry of the word be '\0'
		// print error
        if ((read_word = read(STDIN_FILENO, word, MAXWORD)) == -1) {
            perror("read from stdin: ");
            exit(1);
		// close pipes
        } else if (read_word == 0) {
			/////////debug print//////////////
			//printf("ready to close pipes! \n");
			for (int k=0; k < i; k++) {
				close( mw_pipes[k][1] );
            	close( wm_pipes[k][0] );
				////////debug print////////////////
				//printf("close pipes for child %i\n", k);
			}
			break;
		// write the word to each worker process
		} else {
			for (int j = 0 ; j < i; j++) {
				if (write(mw_pipes[j][1], word, MAXWORD) == -1 ){
					perror("write to worker: ");
					exit(1);
				}
			}
		}

		// initialize the FreqRecords
		FreqRecord FreqRecords[MAXRECORDS*sizeof(FreqRecord)];
		for (int j=0; j < MAXRECORDS; j++)
			FreqRecords[0] = *InitRecord;
		
		/* read one FreqRecord a time from each worker */
		FreqRecord NewFreqRecord[sizeof(FreqRecord)];
		int read_freqRecord;
		for (int j = 0; j < i ; j++) {
			while (1) {
				if ((read_freqRecord = read(wm_pipes[j][0], NewFreqRecord, sizeof(FreqRecord))) == -1) {
					perror("read from worker: ");
					exit(1);
				} else if (read_freqRecord == 0) break;

				// if there is no more data from current worker, check the next worker
				if (NewFreqRecord->freq == 0)
					break;
				
				/////////////debug print////////////////////////////////////////////
				// printf("%i	%s\n", NewFreqRecord->freq, NewFreqRecord->filename);
				
				/* insert NewFreqRecord and sort FreqRecords by freq */
				// find the right place for NewFreqRecord
				int m;
				for (m=0 ; m < MAXRECORDS; m++) {
					if (FreqRecords[m].freq < NewFreqRecord->freq)
						break;
				}
				// shif the last part of the array
				if (m < MAXRECORDS) {
					int n;
					for (n=MAXRECORDS-1 ; n > m; n--) {
						FreqRecords[n] = FreqRecords[n-1];
					}
					FreqRecords[m] = *NewFreqRecord;
				}
				// make sure FreqRecords will always end with InitRecord
				FreqRecords[MAXRECORDS-1] = *InitRecord;
			}
		}
		//////////////debug print for border////////////////////////////////////////////
		//printf("///////////////////////////////////////////////////////////\n");
		print_freq_records(FreqRecords);
		//printf("///////////////////////////////////////////////////////////\n");
    }
	// wait all children terminate first
	for (int k=0 ; k < i; k++ )
		wait(NULL);
	return 0;
}

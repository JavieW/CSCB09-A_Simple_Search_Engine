#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include "freq_list.h"
#include "worker.h"

// note: when complie this file should in clude worker.c, freq_list.c and get_word_test.c
// note: should also add a function declaration in worker.h

int main(int argc, char **argv)
{
  Node *head = NULL;
  char **filenames = init_filenames();
  char arg;
  char *listfile = "testing/big/dir8/index";
  char *namefile = "testing/big/dir8/filenames";
  char *word = "yards";


  while ((arg = getopt(argc,argv,"i:n:")) > 0){
    switch(arg){
      case 'i':
        listfile = optarg;
        break;
      case 'n':
        namefile = optarg;
        break;
      default:
	fprintf(stderr, "Bad arguments for printindex\n");
	exit(1);
    }
  }


  read_list(listfile, namefile, &head, filenames);
  FreqRecord *frp;
  frp = get_word(word, head, filenames);
  print_freq_records(frp);
  return 0;
}
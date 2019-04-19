#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "freq_list.h"

int main(int argc, char **argv)
{
  Node *head = NULL;
  char **filenames = init_filenames();
  char arg;
  char *listfile = "index";
  char *namefile = "filenames";

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
    display_list(head, filenames);

  return 0;
}

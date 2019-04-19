#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char *remove_punc(char *word) {
    int i = 0;

    /* remove punctuation from the beginning of the word */
    while(ispunct(*word)) {
	word++;
    }
    i = 0;
    while(word[i] != '\0') {

	word[i] = tolower(word[i]);
	i++;
    }

    /* remove punctuation from the end of the word */
    i = strlen(word) - 1;
    while((ispunct(word[i]) || isspace(word[i]))&& i >= 0) {
	word[i] = '\0';
	i--;
    }
    return word;
}

# Makefile for programs to index and search an index.

FLAGS= -Wall -g
SRC =  freq_list.c punc.c
OBJ =  freq_list.o punc.o

all : indexer queryone printindex get_word_test query

indexer : indexer.o ${OBJ}
	gcc ${FLAGS} -o $@ indexer.o ${OBJ}

printindex : printindex.o ${OBJ}
	gcc ${FLAGS} -o $@ printindex.o ${OBJ}

queryone : queryone.o worker.o ${OBJ}
	gcc ${FLAGS} -o $@ queryone.o worker.o ${OBJ}
query : query.o worker.o ${OBJ}
	gcc ${FLAGS} -o $@ query.o worker.o  ${OBJ}
get_word_test : get_word_test.o worker.o ${OBJ}
	gcc ${FLAGS} -o $@ get_word_test.o worker.o  ${OBJ}

# Separately compile each C file
%.o : %.c freq_list.h 
	gcc ${FLAGS} -c $<

queryone.o : worker.h
worker.o : worker.h
query.o : worker.h
get_word_test.o : worker.h

clean :
	-rm *.o indexer queryone printindex

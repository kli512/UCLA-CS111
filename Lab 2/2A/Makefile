# NAME: Kevin Li
# EMAIL: li.kevin512@gmail.com
# ID: 123456789

CC=gcc
CFLAGS=-Wall -Wextra -pthread# -D_GNU_SOURCE

ADD_EXEC=lab2_add
ADD_SRC=lab2_add.c

LIST_EXEC=lab2_list
LIST_SRC=lab2_list.c

TAR=lab2a-123456789.tar.gz
ADD_SRC=lab2_add.c

ADDITIONAL_DELIVERABLES=lab2_add.csv lab2_list.csv
ADDITIONAL_DELIVERABLES+=lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png
ADDITIONAL_DELIVERABLES+=lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png

build: $(ADD_EXEC) $(LIST_EXEC)

$(ADD_EXEC):
	$(CC) $(CFLAGS) $(ADD_SRC) -o $(ADD_EXEC)

$(LIST_EXEC):
	$(CC) $(CFLAGS) $(LIST_SRC) SortedList.c -o $(LIST_EXEC)

tests: clean build
	-./lab2_add.sh 2>/dev/null
	-./lab2_list.sh 2>/dev/null

graphs: tests
	-./lab2_add.gp
	-./lab2_list.gp

dist: graphs
	tar -cvzf $(TAR) $(ADD_SRC) SortedList.h SortedList.c $(LIST_SRC) Makefile $(ADDITIONAL_DELIVERABLES) README lab2_add.gp lab2_list.gp lab2_add.sh lab2_list.sh

clean:
	rm -f $(ADD_EXEC) $(LIST_EXEC) $(TAR)
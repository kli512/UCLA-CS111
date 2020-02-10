# NAME: Kevin Li
# EMAIL: li.kevin512@gmail.com
# ID: 123456789

CC=gcc
CFLAGS=-Wall -Wextra
LIBS=-lmraa -lm
EXEC=lab4b
SRC=lab4b.c

TAR=lab4b-123456789.tar.gz

build: $(EXEC)

$(EXEC):
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LIBS)

dist:
	tar -cvzf $(TAR) $(SRC) Makefile README

clean:
	rm -f $(EXEC) $(TAR)
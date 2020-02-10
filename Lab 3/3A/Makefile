# NAME: Kevin Li
# EMAIL: li.kevin512@gmail.com
# ID: 123456789

CC=gcc
CFLAGS=-Wall -Wextra

EXEC=lab3a
SRC=lab3a.c

TAR=lab3a-123456789.tar.gz

build: $(EXEC)

$(EXEC):
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC)

dist:
	tar -cvzf $(TAR) $(SRC) Makefile README ext2_fs.h

clean:
	rm -f $(EXEC) $(TAR)
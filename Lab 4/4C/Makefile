# NAME: Kevin Li
# EMAIL: li.kevin512@gmail.com
# ID: 123456789

CC=gcc
CFLAGS=-Wall -Wextra
LIBS=-lmraa -lm
EXEC_TCP=lab4c_tcp
EXEC_TLS=lab4c_tls
SRC_TCP=lab4c_tcp.c
SRC_TLS=lab4c_tls.c

TAR=lab4c-123456789.tar.gz

build: $(EXEC_TCP) $(EXEC_TLS)

$(EXEC_TCP):
	$(CC) $(CFLAGS) $(SRC_TCP) -o $(EXEC_TCP) $(LIBS)

$(EXEC_TLS):
	$(CC) $(CFLAGS) $(SRC_TLS) -o $(EXEC_TLS) $(LIBS) -lssl -lcrypto

dist:
	tar -cvzf $(TAR) $(SRC_TLS) $(SRC_TCP) Makefile README

clean:
	rm -f $(EXEC_TCP) $(EXEC_TLS) $(TAR)
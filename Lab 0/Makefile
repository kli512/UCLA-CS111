# NAME: Kevin Li
# EMAIL: li.kevin512@gmail.com
# ID: 123456789

CC=gcc
CFLAGS=-Wall -Wextra

EXEC_FILE=lab0
SRC_FILE=lab0.c

SHELL=/bin/bash

all: $(EXEC_FILE)

debug:
	$(CC) -o $(EXEC_FILE) $(SRC_FILE) $(CFLAGS) -g

$(EXEC_FILE):
	$(CC) -o $(EXEC_FILE) $(SRC_FILE) $(CFLAGS)

dist:
	tar -czvf lab0-123456789.tar.gz lab0.c Makefile README backtrace.png breakpoint.png

clean:
	rm -f $(EXEC_FILE) lab0-123456789.tar.gz

check: clean all copy redirection segfault catch dump

copy:
	@echo
	@echo "I love Eggert and Zhaoxing" > input.txt.tmp; \
	./lab0 < input.txt.tmp > output.txt.tmp; \
	if [[ $$? -ne 0 ]]; then \
		echo "Copy test exit code INCORRECT"; \
	else \
		echo "Copy test exit code CORRECT"; \
	fi; \
	diff -s input.txt.tmp output.txt.tmp
	@rm -f input.txt.tmp output.txt.tmp

redirection:
	@echo
	@echo "I love Eggert and Zhaoxing" > input.txt.tmp; \
	./lab0 --input=input.txt.tmp --output=output.txt.tmp; \
	if [[ $$? -ne 0 ]]; then \
		echo "Redirection test exit code INCORRECT"; \
	else \
		echo "Redirection test exit code CORRECT"; \
	fi; \
	diff -s input.txt.tmp output.txt.tmp
	@rm -f input.txt.tmp output.txt.tmp

segfault:
	@echo
	@./lab0 --segfault; \
	if [[ $$? -ne 139 ]]; then \
		echo "Segfault test exit code INCORRECT"; \
	else \
		echo "Segfault test exit code CORRECT"; \
	fi;

catch:
	@echo
	@./lab0 --segfault --catch; \
	if [[ $$? -ne 4 ]]; then \
		echo "Catch test exit code INCORRECT"; \
	else \
		echo "Catch test exit code CORRECT"; \
	fi;

dump:
	@echo
	@./lab0 --segfault --catch --dump-core; \
	if [[ $$? -ne 139 ]]; then \
		echo "Dump test exit code INCORRECT"; \
	else \
		echo "Dump test exit code CORRECT"; \
	fi;

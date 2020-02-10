# NAME: Kevin Li
# EMAIL: li.kevin512@gmail.com
# ID: 123456789

CC=gcc
CFLAGS=-Wall -Wextra

EXEC_FILE=simpsh
SRC_FILE=lab1.c

SHELL=/bin/bash

all: $(EXEC_FILE)

$(EXEC_FILE):
	$(CC) -o $(EXEC_FILE) $(SRC_FILE) $(CFLAGS)

dist:
	tar -czvf lab1-123456789.tar.gz lab1.c Makefile README

clean:
	rm -f $(EXEC_FILE) lab1-123456789.tar.gz

check: clean all tr rdfail cmdfail verbose

tr:
	@echo; \
	rm -f input.txt.tmp output.txt.tmp; \
	echo "I love Eggert and Zhaoxing" > input.txt.tmp; \
	touch output.txt.tmp; \
	./simpsh --rdonly input.txt.tmp --wronly output.txt.tmp \
	--command 0 1 1 tr a-z A-Z; \
	if [[ $$? -ne 0 ]]; then \
		echo "tr test exit code INCORRECT"; \
	else \
		echo "tr test exit code CORRECT"; \
	fi; \
	if [[ `wc -l < output.txt.tmp` -ne 1 ]] || [[ `head -1 output.txt.tmp` != "I LOVE EGGERT AND ZHAOXING" ]]; then \
		echo "tr test output INCORRECT"; \
	else \
		echo "tr test output CORRECT"; \
	fi; \
	rm -f input.txt.tmp output.txt.tmp

rdfail:
	@echo; \
	rm -f input.txt.tmp output.txt.tmp err.txt.tmp; \
	touch output.txt.tmp; \
	./simpsh --rdonly input.txt.tmp --wronly output.txt.tmp \
	--command 0 1 1 tr a-z A-Z 2>err.txt.tmp; \
	if [[ $$? -ne 1 ]]; then \
		echo "rdfail test exit code INCORRECT"; \
	else \
		echo "rdfail test exit code CORRECT"; \
	fi; \
	if [[ -e err.txt.tmp ]]; then \
		echo "rdfail test error output CORRECT"; \
	else \
		echo "rdfail test error output INCORRECT"; \
	fi; \
	rm -f input.txt.tmp output.txt.tmp err.txt.tmp

cmdfail:
	@echo; \
	rm -f input.txt.tmp output.txt.tmp err.txt.tmp; \
	echo "I love Eggert and Zhaoxing" > input.txt.tmp; \
	touch output.txt.tmp; \
	./simpsh --rdonly input.txt.tmp --wronly output.txt.tmp \
	--command 0 1 tr a-z A-Z 2>err.txt.tmp; \
	if [[ $$? -ne 1 ]]; then \
		echo "cmdfail test exit code INCORRECT"; \
	else \
		echo "cmdfail test exit code CORRECT"; \
	fi; \
	if [[ -e err.txt.tmp ]]; then \
		echo "cmdfail test error output CORRECT"; \
	else \
		echo "cmdfail test error output INCORRECT"; \
	fi; \
	rm -f input.txt.tmp output.txt.tmp err.txt.tmp

verbose:
	@echo; \
	rm -f input.txt.tmp output.txt.tmp stdout.txt.tmp; \
	echo "I love Eggert and Zhaoxing" > input.txt.tmp; \
	touch output.txt.tmp; \
	./simpsh --verbose --rdonly input.txt.tmp --wronly output.txt.tmp \
	--command 0 1 1 tr a-z A-Z >stdout.txt.tmp; \
	if [[ $$? -ne 0 ]]; then \
		echo "verbose test exit code INCORRECT"; \
	else \
		echo "verbose test exit code CORRECT"; \
	fi; \
	if [[ `wc -l < stdout.txt.tmp` -ne 3 ]]; then \
		echo "verbose test output INCORRECT"; \
	else \
		echo "verbose test output CORRECT"; \
	fi; \
	rm -f input.txt.tmp output.txt.tmp stdout.txt.tmp

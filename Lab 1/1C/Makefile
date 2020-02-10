# NAME: Kevin Li
# EMAIL: li.kevin512@gmail.com
# ID: 123456789

CC=gcc
CFLAGS=-Wall -Wextra # -D_GNU_SOURCE

EXEC_FILE=simpsh
SRC_FILE=lab1.c

SHELL=/bin/bash

all: $(EXEC_FILE)

$(EXEC_FILE):
	$(CC) -o $(EXEC_FILE) $(SRC_FILE) $(CFLAGS)

dist:
	tar -czvf lab1-123456789.tar.gz lab1.c Makefile README report.pdf

clean:
	rm -f $(EXEC_FILE) lab1-123456789.tar.gz

check: clean all tr rdfail cmdfail verbose flags

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

flags:
	@echo; \
	echo "This is part 1b" > input.txt.tmp; \
	echo "Remove this!" > error.txt.tmp; \
	./simpsh --rdonly input.txt.tmp --creat --rdwr output.txt.tmp --trunc --wronly error.txt.tmp \
  --command 0 1 2 cat; \
	if [[ $$? -ne 0 ]]; then \
		echo "flags test exit code INCORRECT"; \
	else \
		echo "flags test exit code CORRECT"; \
	fi; \
	if [[ ! -s error.txt.tmp ]] && [[ -e output.txt.tmp ]]; then \
		echo "flags output CORRECT"; \
	else \
		echo "flags output INCORRECT"; \
	fi; \
	rm -f input.txt.tmp output.txt.tmp error.txt.tmp

pipes:
	@echo; \
	echo "This is part 1b" > input.txt.tmp; \
	./simpsh --rdonly input.txt.tmp --creat --wronly error.txt.tmp --pipe --creat --wronly output.txt.tmp \
  --command 0 3 1 cat --command 2 4 1 cat; \
	if [[ $$? -ne 0 ]]; then \
		echo "pipes test exit code INCORRECT"; \
	else \
		echo "pipes test exit code CORRECT"; \
	fi; \
	if [[ `diff output.txt.tmp input.txt.tmp | wc -l` -eq 0 ]]; then \
		echo "pipes output CORRECT"; \
	else \
		echo "pipes output INCORRECT"; \
	fi; \
	rm -f input.txt.tmp output.txt.tmp error.txt.tmp


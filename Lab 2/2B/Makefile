# NAME: Kevin Li
# EMAIL: li.kevin512@gmail.com
# ID: 123456789

CC=gcc
CFLAGS=-Wall -Wextra -pthread -g

LIST_EXEC=lab2_list
LIST_SRC=lab2_list.c

TAR=lab2b-123456789.tar.gz

ADDITIONAL_DELIVERABLES=lab2b_list.csv
ADDITIONAL_DELIVERABLES+=lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png

build: $(LIST_EXEC)

$(LIST_EXEC):
	$(CC) $(CFLAGS) $(LIST_SRC) SortedList.c -o $(LIST_EXEC)

tests: clean build
	-./lab2b_list.sh 2>/dev/null

profile: clean $(LIST_EXEC)
	rm -f ./raw.gperf
	LD_PRELOAD=/usr/local/cs/gperftools-2.7/lib/libprofiler.so.0 CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf >profile.out
	pprof --list=process_list ./lab2_list ./raw.gperf >>profile.out
	rm -f ./raw.gperf

graphs: tests
	-./lab2b_list.gp

dist: graphs profile
	tar -cvzf $(TAR) SortedList.h SortedList.c $(LIST_SRC) Makefile $(ADDITIONAL_DELIVERABLES) README lab2b_list.gp lab2b_list.sh profile.out

clean:
	rm -f $(LIST_EXEC) $(TAR)
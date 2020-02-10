# NAME: Kevin Li
# EMAIL: li.kevin512@gmail.com
# ID: 123456789

# The gnuplot reduction script is highly based off the given scripts for part 2a
# http://web.cs.ucla.edu/classes/fall19/cs111/labs/P2A_lock/ASSIGNMENT/lab2_list.gp

#! /usr/local/cs/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2b_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#   8. average lock wait time (ns)
#
# output:
#	lab2b_1.png ... throughput vs threads with mutex and spin locks
#	lab2b_2.png ... op time and wait time for mutex lists
#	lab2b_3.png ... Multi-list correctness with and without locks
#	lab2b_4.png ... effect of #lists on throughput with mutex
#	lab2b_5.png ... effect of #lists on throughput with spinlocks
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#   They have been additionally modified to use sed and tail to choose
#   from which lines to pull data
#

# general plot parameters
set terminal png
set datafile separator ","

### looking at the effect of # threads on throughput
set title "Scalability-1: Throughput of Synchronized Lists"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput (ops/second)"
set logscale y 10
set output 'lab2b_1.png'

# all greps are prefixed by sed's that grab only the lines relevant
# the line numbers are based on the output form lab2b_list.sh

# grep out only no yield, lock protected results with 1k iterations and 1 list
plot \
     "< sed -n 1,14p lab2b_list.csv | grep -E 'list-none-s,[0-9]+,1000,1,'" using ($2):(1000000000/$7) \
	title 'throughput w/ spinlock' with linespoints lc rgb 'red', \
     "< sed -n 1,14p lab2b_list.csv | grep -E 'list-none-m,[0-9]+,1000,1,'" using ($2):(1000000000/$7) \
	title 'throughput w/ mutex' with linespoints lc rgb 'green', \

### looking at the effect of # threads on time waiting w/ mutex
set title "Scalability-2: Mutex protected list operation timings"
set xlabel "Threads"
set logscale x 2
set ylabel "Average time (ns)"
set logscale y 10
set output 'lab2b_2.png'

# grep out only no yield, mutex protected results with 1k iterations and 1 list
plot \
     "< sed -n 15,20p lab2b_list.csv | grep -E 'list-none-m,[0-9]+,1000,1,'" using ($2):($7) \
	title 'Operation time' with linespoints lc rgb 'green', \
     "< sed -n 15,20p lab2b_list.csv | grep -E 'list-none-m,[0-9]+,1000,1,'" using ($2):($8) \
	title 'Lock waiting time' with linespoints lc rgb 'blue', \

### looking at what succeeded and what failed with multi lists + locks
set title "Scalability-3: Multi-list accuracy"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'
# note that unsuccessful runs should have produced no output
plot \
     "< grep list-id-s lab2b_list.csv" using ($2):($3) \
	title 'spinlock' with points lc rgb 'blue', \
     "< grep list-id-m lab2b_list.csv" using ($2):($3) \
	title 'mutex' with points lc rgb 'red', \
     "< grep list-id-none lab2b_list.csv" using ($2):($3) \
	title 'unprotected' with points lc rgb 'green', \

### scalability of multi lists with mutex
set title "Scalability-4: Throughput of Mutex protected Partitioned Lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.5:24]
set ylabel "Throughput (ops/second)"
set logscale y 10
set output 'lab2b_4.png'

# grep out only no yield, mutex protected results with 1k iterations from the end
plot \
     "< tail -n 40 lab2b_list.csv | grep -E 'list-none-m,[0-9]+,1000,1,'" using ($2):(1000000000/$7) \
	title '1 list' with linespoints lc rgb 'green', \
     "< tail -n 40 lab2b_list.csv | grep -E 'list-none-m,[0-9]+,1000,4,'" using ($2):(1000000000/$7) \
	title '4 lists' with linespoints lc rgb 'red', \
     "< tail -n 40 lab2b_list.csv | grep -E 'list-none-m,[0-9]+,1000,8,'" using ($2):(1000000000/$7) \
	title '8 lists' with linespoints lc rgb 'blue', \
     "< tail -n 40 lab2b_list.csv | grep -E 'list-none-m,[0-9]+,1000,16,'" using ($2):(1000000000/$7) \
	title '16 lists' with linespoints lc rgb 'orange', \

### scalability of multi lists with mutex
set title "Scalability-5: Throughput of Spinlock protected Partitioned Lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.5:24]
set ylabel "Throughput (ops/second)"
set logscale y 10
set output 'lab2b_5.png'

# grep out only no yield, spin protected results with 1k iterations from the end
plot \
     "< tail -n 40 lab2b_list.csv | grep -E 'list-none-s,[0-9]+,1000,1,'" using ($2):(1000000000/$7) \
	title '1 list' with linespoints lc rgb 'green', \
     "< tail -n 40 lab2b_list.csv | grep -E 'list-none-s,[0-9]+,1000,4,'" using ($2):(1000000000/$7) \
	title '4 lists' with linespoints lc rgb 'red', \
     "< tail -n 40 lab2b_list.csv | grep -E 'list-none-s,[0-9]+,1000,8,'" using ($2):(1000000000/$7) \
	title '8 lists' with linespoints lc rgb 'blue', \
     "< tail -n 40 lab2b_list.csv | grep -E 'list-none-s,[0-9]+,1000,16,'" using ($2):(1000000000/$7) \
	title '16 lists' with linespoints lc rgb 'orange', \

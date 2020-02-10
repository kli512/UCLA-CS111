# NAME: Kevin Li
# EMAIL: li.kevin512@gmail.com
# ID: 123456789

# Script that generates the required lab2_add.csv
# Takes a fair amount of time

rm lab2_add.csv 2>/dev/null

for t in 1 2 4 8 12; do
  for i in 1 10 100 1000 10000 100000; do
    ./lab2_add --iterations=$i --threads=$t >>lab2_add.csv
  done
done

for t in 1 2 4 8 12; do
  for i in 10 20 40 80 100 1000 10000 100000; do
    ./lab2_add --yield --iterations=$i --threads=$t >>lab2_add.csv
    ./lab2_add --iterations=$i --threads=$t >>lab2_add.csv
  done
done

for t in 1 2 4 8 12; do
  for i in 10 20 40 80 100 1000 10000; do
    for lock in m c; do
      ./lab2_add --yield --iterations=$i --threads=$t --sync=$lock >>lab2_add.csv
      ./lab2_add --iterations=$i --threads=$t --sync=$lock >>lab2_add.csv
    done
  done
done

for t in 1 2 4 8 12; do
  for i in 10 20 40 80 100 1000 10000; do
    ./lab2_add --yield --iterations=$i --threads=$t --sync=s >>lab2_add.csv
    ./lab2_add --iterations=$i --threads=$t --sync=s >>lab2_add.csv
  done
done

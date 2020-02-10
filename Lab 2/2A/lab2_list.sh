# NAME: Kevin Li
# EMAIL: li.kevin512@gmail.com
# ID: 123456789

# Script that generates lab2_list.csv
# Takes a pretty long time to run

rm lab2_list.csv 2>/dev/null

for t in 1; do
  for i in 10 100 1000 10000 20000; do
    ./lab2_list --iterations=$i --threads=$t >>lab2_list.csv
  done
done

for t in 2 4 8 12; do
  for i in 10 100 1000 ; do
    ./lab2_list --iterations=$i --threads=$t >>lab2_list.csv
  done
done

for t in 2 4 8 12; do
  for i in 1 2 4 8 16 32; do
    for y in i d il dl; do
      ./lab2_list --iterations=$i --threads=$t --yield=$y >>lab2_list.csv
    done
  done
done

for t in 2 4 8 12; do
  for i in 1 2 4 8 16 32; do
    for y in i d il dl; do
      for l in m s; do
        ./lab2_list --iterations=$i --threads=$t --yield=$y --sync=$l >>lab2_list.csv
      done
    done
  done
done

for t in 1 2 4 8 12 16 24; do
  for i in 1000; do
    for l in m s; do
      ./lab2_list --iterations=$i --threads=$t --sync=$l >>lab2_list.csv
    done
  done
done

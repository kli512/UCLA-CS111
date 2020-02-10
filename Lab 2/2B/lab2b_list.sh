# NAME: Kevin Li
# EMAIL: li.kevin512@gmail.com
# ID: 123456789

rm lab2b_list.csv 2>/dev/null

# lab2b_1.png
for t in 1 2 4 8 12 16 24; do
  for i in 1000; do
    for l in s m; do
      ./lab2_list --iterations=$i --threads=$t --sync=$l >>lab2b_list.csv
    done
  done
done

# lab2b_2.png
for t in 1 2 4 8 16 24; do
  for i in 1000; do
    for l in m; do
      ./lab2_list --iterations=$i --threads=$t --sync=$l >>lab2b_list.csv
    done
  done
done

# lab2b_3.png
lists=4
for t in 1 4 8 12 16; do
  for i in 1 2 4 8 16; do
    for y in id; do
      ./lab2_list --iterations=$i --threads=$t --yield=$y --lists=$lists >>lab2b_list.csv
    done
  done
done

for t in 1 4 8 12 16; do
  for i in 10 20 40 80; do
    for y in id; do
      for l in m s; do
        ./lab2_list --iterations=$i --threads=$t --yield=$y --sync=$l --lists=$lists >>lab2b_list.csv
      done
    done
  done
done

# lab2b_4.png
for t in 1 2 4 8 12; do
  for i in 1000; do
    for l in m s; do
      for lists in 1 4 8 16; do
        ./lab2_list --iterations=$i --threads=$t --sync=$l --lists=$lists >>lab2b_list.csv
      done
    done
  done
done

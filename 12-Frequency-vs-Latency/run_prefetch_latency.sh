#!/bin/bash
gcc prefetch_latency.c -o prefetch_latency -lpthread 
#gcc prefetch_latency.c -o prefetch_latency -lpthread -mprefetchwt1

for i in 0 1 2 3 4 5 6 7
do
  for j in 0 1 2 3 4 5 6 7
  do
    ./prefetch_latency ${i} ${j} > out_${i}_${j}
    python process.py out_${i}_${j} > o_${i}_${j}
  done
done

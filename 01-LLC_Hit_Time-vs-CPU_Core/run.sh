#!/bin/bash


rm -rf data.csv;

n_cpus=16
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/farabi/gcc-11.2.0-build/lib;
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/farabi/gcc-11.2.0-build/lib64;
# for ((i=0; i<$n_cpus; i=i+1));
# do
#     sudo setarch $(uname -m) -R ./a.out --num-address 1 --cpu $i >> "data.csv";
# done
# dests="26 40 48 66 70"
make multi;
# Check the exit status of g++
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1 
else
    echo "Compilation successful."
fi


n_samples=1000; 
for((i=0; i<$n_samples; i=i+1));
do
  for ((src=0; src<2; src=src+2))
  do 
    for ((dest=0; dest<$n_cpus; dest=dest+2))
    do
      if [[ $src == $dest ]]; then
        continue
      fi 
      sudo LD_LIBRARY_PATH=${LD_LIBRARY_PATH} setarch $(uname -m) -R ./multithreaded-l2-latency.out --source-cpu $src --remote-cpu $dest  >> data.csv;
      sleep 0.0005;
    done 
  done 
done 
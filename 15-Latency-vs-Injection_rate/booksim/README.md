# Compiling BookSim
Go to src directory
```
cd src
```
Install Dependencies 
```
sudo apt-get install build-essential gdb flex bison -y
```

Make 
```
make 
```

This should create a executable BookSim under `booksim`

## Simulate a single sample with fixed injection rate 
```
./booksim mesh88_lat 
```
This mesh88_lat file uses a 8x8 mesh network. Output will contain a line like this 
```
Worst Case,(0-63-0.4),0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
```
Which is a histogram of data with 100 bins each of size 5.0. You can change the bin size and count by changing the variable inside the `mesh88_lat` file
```
pair_plat_num_bins = 100;
pair_plat_bin_size = 5.0;
```
--- 

## Simulate a network sweep with variable rates 
Make sure you are in the booksim root directory. 

use the `utils/sweep.sh` with following configuration 
```
./utils/sweep.sh ./src/booksim ./src/mesh88_lat > output.log 
```
This will generate the output sweep until the network saturates

This will generate a huge file (>GBs) if you want to include all stats. However, you can only collect the longest delay by setting the following variable at the `mesh88_lat` file to be 1
```
only_longest_delay = 1;
```

## Parsing Simulation Results for Plot
To parse the results use the `prepare_data.py` 
Make sure the `raw_logfile` name points correctly to the output log file generated from the simulator
```
python3 prepare_data.py 
```

It would create a new output file. By default the name of the output file is `newdata.csv`


## Plotting Simulation Results from Parsed Data
To plot the data you need to use the `plot.py` 
Make sure the `input_filename` variable is adjusted to the correct parsed output from the previous step.

This would create two output files
```
worst-case-simulation-8x8.pdf
worst-case-simulation-8x8.png
```

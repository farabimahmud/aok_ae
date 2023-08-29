# Classifer Accuracy

To calculate this accuracy, we need to generate the trace using the attack in a KNL machine. We have generated the trace in `data/timer-10k-10p-100t.csv` file. 
Each line of this file contains the folowing information 
```15,0x4221,0x4342,0x2314,0x1422,229,229,15285,1350,690,975```
Here 15 denotes that all the cache entries are used in last round. This is a 4 bit quantity which denotes which cache set is being accessed at the last level of the aes. Since, there are 4 cache sets in the last round between each measurement, we can have either `0000~1111` total 16 options. 
Next 4 entries show cache access patterns. 

For example, 0x4221 means cache set 4, 2, 2 and 1 was accssed by the aes decrypt last round. 

Next two entries denote corresponding plaintext and ciphertext bytes for that particular round

Last 4 elements denote the time measured by the attacker using the method explained in the paper. Our ML model use these information to predict key correctly. 


# How to plot 
To plot this graph, we can type `python3 plot.py` and the graphs will be generated after rounds of RandomForestClassifer algorithm. 

We need to adjust the filename in the plot.py file to denote exact tracefile being used. 

# Trace Generation
To generate the trace, you need to run the trace generator at `aes-attack-trace-generation/trace-generator`

To compile this - 
```
cd aes-attack-trace-generation;
make trace-generator;
```

Now launch this with proper parameters. 
```
Optional arguments:
-h --help               shows help message and exits [default: false]
-v --version            prints version information and exits [default: false]
-s --source-cpu         Which CPU core we should pin one thread to, default 0 [default: 0]
-r --remote-cpu         Which CPU core we should pin one thread to, default 66 [default: 66]
-c --timer-cpu          Which CPU core we should pin one thread to, default 12 [default: 12]
-a --address-offset     What is the offset address to probe [default: 0]
-t --trials             Number of trials per run, default 1 [default: 1]
-k --key                first byte of key [default: 0]
-p --plain              first byte of plain [default: 0]
```

Also, we need to enabel HugePages to allocate memory. Or we can allocate physical page using `sudo` to make sure different iterations generate same virtual to physical mapping. However, if we increase number of trials and we should be able to generate enough samples to classify and predict the keys from.
# Accuracy vs Number of Trials 

We run the experiments from the previously generated traces to calculate the ML model accuracy. Now, we utilize this prediction accuracy to generate the key extraction accuracy for the attack.

## Build Key Extraction Program
We use the AES based key extraction algorithm by leveraging the last round decryption. To do that we need to run
```
make AES-key-extraction 
```

## Generate Data
Once the program is build, we need to run this program multiple times to generate required data. 
To do that, first we need to modify and run the `generate_data.py` file

```
python3 generate_data.py
```
This will create a new file inside teh results directory which will contain trials, number of keys and accuracy values. 

## Plotting the Data
We can generate the plots by renaming the data file and executing the following - 
```
python3 plot.py 
```




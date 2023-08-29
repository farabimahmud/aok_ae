# Setup
1. Clone the repository
  ```
  git clone --recurse-submodules https://github.com/farabimahmud/aok_ae

  ```
2. Go to the cloned directory and create a virtual environment 
  ```
  cd aok_ae;
  python3 -m venv aok 
  ```
3. Activate the environment 
  ```
  source aok/bin/activate
  ```

4. Install the requirements 
  ```
  pip3 install -r requirements.txt
  ```

# Generate Plots With Existing Data

For each plot cd to the directory and run the following command. For example to generate 01-LLC_Hit_Time-vs-CPU_Core graph which is the Figure 1 of the paper, you need to run the following code - 
```
cd 01-LLC_Hit_Time-vs-CPU_Core;
python3 plot_knl_data.py;
```
This assumes that the appropriate data files, for example, in this case `knl_data.csv` is present in the folder. This repository contains all the existing data that was used to create the graphs in the paper. 

# Generate New Data 
Each folder contains the code and separate README to guide in generating new data. Please follow along. 

#!/home/farabi/anaconda3/bin/python3

import os
import datetime 
import numpy as np 

program_name = "./AES-key-extraction"
output_file = "results/data.csv"

# trials = np.logspace(1,6,101)
trials = list(np.logspace(7,18,12, base=2))

# print(trials)
# keys = np.logspace(1,16,16,base=2)
keys = [128,256,512,1024]
# probabilities = np.linspace(0.5,1.0,19)
probabilities= [0.5]

for t in trials:
  for k in keys:
    for p in probabilities:
      cmdline = []
      cmdline.append(program_name)
      
      cmdline.append("--trials")
      cmdline.append(str(int(t)))
      cmdline.append("--keys")
      cmdline.append(str(int(k)))
      cmdline.append("--probability")
      cmdline.append(str(float(p)))          
      cmdline.append(">>")
      cmdline.append(output_file)
      cmdline.append(";")
        
      cmdline_str = " ".join(cmdline)
      print("Currently Executing ", cmdline_str)
      os.system(cmdline_str)
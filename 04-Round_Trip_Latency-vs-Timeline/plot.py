import pandas as pd 
import seaborn as sns 
import matplotlib.pyplot as plt 

# 2,3,4 bit == 0 

threshold = 70
def get_one_latency(x):
  if x < threshold:
    return x 
  return None 

def get_zero_latency(x):
  if x >= threshold:
    return x 
  return None 

def main():
  plt.figure(figsize=(12,3),dpi=600)
  df = pd.read_csv("data.csv")
  df['Value'] = (1 - df['Value'])*120
  df['one_latency']  = df['Latency'].apply(get_one_latency)
  df['zero_latency'] = df['Latency'].apply(get_zero_latency)
  
  ax = sns.barplot(
    x='Timeline',
    y='Value',
    data = df,
    color='green',
    alpha=0.35,
  )  
  
  ax = sns.scatterplot(
    x='Timeline',
    y='zero_latency',
    data = df,
    color='darkblue',
    marker='X',
    s=65,

  )
  
  ax = sns.scatterplot(
    x='Timeline',
    y='one_latency',
    data = df,
    color='red',
    marker='X',
    s=65,
  )

  
  # print(df)
  plt.ylabel("Round Trip Latency\n(Cycles)", fontsize=16)
  plt.xlabel("Timeline", fontsize=16)
  plt.ylim((0,110))
  plt.xticks(df['Timeline'][::5],rotation=45)
  plt.tight_layout()
  plt.savefig("Proof of Concept.pdf")
  plt.savefig("Proof of Concept.png")

if __name__ == "__main__":
  main()
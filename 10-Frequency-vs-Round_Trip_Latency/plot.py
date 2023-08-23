import pandas as pd 
import seaborn as sns
from scipy.ndimage.filters import gaussian_filter1d
import matplotlib.pyplot as plt 
import matplotlib.patches as patches 

SMOOTHING_FACTOR=0.37


plt.figure(figsize=(8,4), dpi=600)
df = pd.read_csv('data.csv')
df = df.reset_index()
# print(df)


df['smooth-farthest']  = gaussian_filter1d(df['Farthest'], sigma=SMOOTHING_FACTOR)
df['smooth-nearest']   = gaussian_filter1d(df['Closest'],  sigma=SMOOTHING_FACTOR)
sns.set_theme(style="whitegrid", palette="hls")

ax = sns.lineplot(
  x = 'RTT',
  y = 'smooth-farthest',
  data = df,   
  color='red'
)
ax = sns.lineplot(
  x = 'RTT',
  y = 'smooth-nearest',
  data = df,  
  color = 'green' 
)

custom_legends = [
  patches.Rectangle(
    (0,0),10,10, facecolor='green', edgecolor='black',alpha=0.8,
  ),
  patches.Rectangle(
    (0,2),10,10, facecolor='red', edgecolor='black', alpha=0.8,
  ),
]

plt.fill_between(df['RTT'], df['smooth-farthest'], color='red',lw=2,alpha=0.4)
plt.fill_between(df['RTT'], df['smooth-nearest'],  color='green',lw=2,alpha=0.4)
plt.xticks(fontsize=16)
plt.yticks(fontsize=16)
plt.xlim((20,100))
plt.ylim((0,100))
plt.ylabel("Frequency (%)",fontsize=18)
plt.xlabel("Round Trip Latency (Cycles)", fontsize=18)
plt.legend(
  handles=custom_legends, 
  labels=['Nearest Node','Farthest Node'], 
  fontsize=16,
  loc="upper center"
)
plt.tight_layout()
plt.savefig("simulated-attack.png")
plt.savefig("simulated-attack.pdf")
print(df)
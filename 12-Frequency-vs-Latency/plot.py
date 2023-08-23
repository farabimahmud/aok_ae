import pandas as pd 
import seaborn as sns 
import matplotlib.pyplot as plt 
from scipy.ndimage.filters import gaussian_filter1d
import matplotlib.patches as patches 


df = pd.read_csv("data.csv")
print(df)
plt.figure(figsize=(8,4), dpi=600)

df['L1_hit'] = df['L1_hit']/df['L1_hit'].sum()*100.0
df['L1_miss'] = df['L1_miss']/df['L1_miss'].sum()*100.0

df['L1_hit'] = gaussian_filter1d( df['L1_hit'] , sigma=0.5)
df['L1_miss'] = gaussian_filter1d(df['L1_miss'], sigma=0.5)

sns.set_theme(style="whitegrid", palette="hls")

ax = sns.lineplot(
  x = 'latency',
  y = 'L1_hit',
  data = df, 
  color = 'green',
  alpha=0.6,
)

ax = sns.lineplot(
  x = 'latency',
  y = 'L1_miss',
  data = df,
  color = 'red',
  alpha = 0.6,
)
# plt.fill_between(x, y1)

custom_legends = [
  patches.Rectangle(
    (0,0),10,10, facecolor='green', edgecolor='black',alpha=0.8,
  ),
  patches.Rectangle(
    (0,2),10,10, facecolor='red', edgecolor='black', alpha=0.8,
  ),
]
plt.legend(
  handles=custom_legends, 
  labels=['PREFETCHW Hit','PREFETCHW Miss'], 
  fontsize=16
)
plt.fill_between(df['latency'], df['L1_miss'],  color='red',lw=2,alpha=0.4)
plt.fill_between(df['latency'], df['L1_hit'], color='green',lw=2,alpha=0.4)

plt.ylim((0,65))
plt.xlim((0,350))
plt.xlabel("Latency", fontsize=18)
plt.ylabel("Frequency(%)", fontsize=18)

plt.tight_layout()
plt.savefig("prefetch-timer.png")
plt.savefig("prefetch-timer.pdf")
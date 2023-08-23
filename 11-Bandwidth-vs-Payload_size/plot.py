
from distutils.log import error
from matplotlib import markers
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt 
import scipy.stats
import matplotlib.patches as mpatches
from scipy.interpolate import BivariateSpline
from matplotlib.ticker import ScalarFormatter
import seaborn as sns 

filename = "data.csv"


def mean_confidence_interval(data, confidence=0.95):
    a = 1.0 * np.array(data)
    n = len(a)
    m, se = np.mean(a), scipy.stats.sem(a)
    h = se * scipy.stats.t.ppf((1 + confidence) / 2., n-1)
    return m, m-h, m+h
# sns.set_theme(style="whitegrid", palette="hls")

df = pd.read_csv(filename)
mean_latencies = df.groupby(['payload'])['latency'].mean().to_list()
payloads = df['payload'].unique()
bw = list()
error_rates = list() 
lows = list() 
highs = list() 
error_lows = list() 
error_highs = list() 
for p in payloads:
  samples = df['latency'].loc[df['payload']==p].to_list()
  mean_latency, low, high = mean_confidence_interval(samples)
  bw.append((p*8)/(mean_latency * 10e-6)/1024)
  lows.append((p*8)/(low * 10e-6)/1024)
  highs.append((p*8)/(high * 10e-6)/1024)

  mean_error_rate, error_low, error_high = mean_confidence_interval(
    (1- df['accuracy'].loc[df['payload']==p]).to_list())
  error_rates.append(mean_error_rate)
  error_lows.append(error_low)
  error_highs.append(error_high)
  
  # print(samples)
# print(bw)


plt.figure(figsize=(8,4), dpi=600)
fig, ax1 = plt.subplots() 
ax1.set_ylabel('Bandwidth (KBps)', fontsize=18)
ax1.set_xlabel('Payload Size (B)',fontsize=18)
plt.xticks(fontsize=16)
plt.yticks(fontsize=16)
ax2 =  ax1.twinx()
ax2.set_ylabel('Error Rate(%)',fontsize=18)

ax1.scatter(x=payloads,y=bw,color='g',marker='^')
ax2.scatter(x=payloads,y=error_rates,color='r',marker='x')
# print(lows)
# print(highs)
ax1.fill_between(payloads,lows,highs,alpha=0.1,color='g')
print(bw)
print(max(error_rates))
ax1.plot(payloads,bw,color='g',marker='^')
ax2.plot(payloads,error_rates,color='r',marker='x')
ax2.fill_between(payloads,error_lows,error_highs,alpha=0.1,color='r')
ax1.set_xscale('log', base=2)
ax1.get_xaxis().set_major_formatter(ScalarFormatter())
ax1.tick_params(axis='x',rotation=20)
ax2.set_ylim((0,0.05))
ax1.set_ylim((150,205))



plt.legend(handles=[
  plt.Line2D([],[],color='g',marker='^',linewidth=0, markersize=15, label='Bandwidth'),
  plt.Line2D([],[],color='r',marker='x',linewidth=0, markersize=15,label='Error Rate'),
],
fontsize=16,
loc="center right")
plt.yticks(fontsize=16)
# ax1.set_xticks(fontsize=16)
# ax1.set_yticks(fontsize=16)
# ax2.set_yticks(fontsize=16)
plt.savefig("bandwidth.png",bbox_inches='tight')
plt.savefig("bandwidth.pdf",bbox_inches='tight')

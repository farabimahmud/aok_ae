#!/bin/python3 

import os
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns 
import numpy as np 
import colorsys

list_of_injection_rate = []
total_buckets = []

def parse_stats(filename):
    with open(filename, "r") as f:
        for line in f:
            row_data = line.strip().split(",")
            meta_data = row_data[0].split("-")
            source_node = int(meta_data[0][1:])
            dest_node = int(meta_data[1])
            injection_rate = float(meta_data[2][:-1])

            if (injection_rate not in list_of_injection_rate):
                list_of_injection_rate.append(injection_rate)
                total_buckets.append([0]*100)
            item_index = list_of_injection_rate.index(injection_rate)
            buckets = row_data[1:-1]
            # print(len(buckets))
            for i in range(len(buckets)):
                total_buckets[item_index][i] += int(buckets[i])
        for i in range(len(list_of_injection_rate)):
            print(list_of_injection_rate[i], total_buckets[i])
        return list_of_injection_rate, total_buckets

def plot_data():
    x = range(0,100)
    for i in range(len(list_of_injection_rate)):
        plt.title("injection rate = {}".format(list_of_injection_rate[i]))
        # plt.xticks(x, range(0,500,5),rotation="vertical")
        plt.bar(x, total_buckets[i])
        plt.savefig("bar_{}.png".format(list_of_injection_rate[i]))
        plt.cla()

input_filename = "data.csv"

# parse_stats(input_filename)
# plot_data()



column_names = ['injrate']
for i in range(100):
    column_names.append(i)
df = pd.read_csv(input_filename, header=None, names=column_names)

df = df.groupby('injrate').sum().reset_index()
# df = df.loc[(df['injrate'] < 0.1)  ]

xrange  = range(0,100)
injrates = []
list_of_means = []
list_of_sds = []
for index,row in df.iterrows():
    l = row.to_list() 
    injrate = l[0]
    injrates.append(injrate)
    hist_data = l[1:]
    ydata = [x/sum(hist_data) for x in hist_data]
    ysum = 0
    count = 0
    for i in range(len(hist_data)):
        ysum += (i*hist_data[i])
        count += hist_data[i]
    list_of_means.append(ysum/count)

    counts = ydata[1:100] 
    bins = np.arange(1,101)
    
    mids = 0.5*(bins[1:] + bins[:-1])
    probs = counts / np.sum(counts)

    mean = np.sum(probs * mids)  
    sd = np.sqrt(np.sum(probs * (mids - mean)**2))
    list_of_sds.append(sd)

def create_color_palette(start_color, end_color, num_steps):
    start_hue = colorsys.rgb_to_hsv(*start_color)[0]
    end_hue = colorsys.rgb_to_hsv(*end_color)[0]
    
    palette = []
    for step in range(num_steps):
        hue = start_hue + (end_hue - start_hue) * (step / (num_steps - 1))
        rgb = colorsys.hsv_to_rgb(hue, 1, 1)  # Keep saturation and lightness constant
        palette.append(rgb)
    
    return palette

print(list_of_means)
print(list_of_sds)
injrate_labels = []
for i in injrates:
    injrate_labels.append("{:0.4f}".format(i))
print(injrate_labels)
plt.figure(figsize=(6,3))


start_color = (0, 0.5, 0)  # Green (RGB values)
end_color   = (1, 0.1, 0)    # Red (RGB values)

colors = create_color_palette(start_color, end_color, len(injrate_labels))
ax = sns.barplot(
    x = injrate_labels,
    y = list_of_means,
    yerr = list_of_sds,
    palette=colors,
    edgecolor="black", linewidth=1
)
plt.xticks(fontsize=18, rotation=45)

plt.xlabel("Injection Rate",fontsize=24)
plt.ylabel("Latency\n(Cycles)", fontsize=24)
plt.tight_layout()

plt.savefig("worst-case-simulation-8x8.pdf")
plt.savefig("worst-case-simulation-8x8.png")

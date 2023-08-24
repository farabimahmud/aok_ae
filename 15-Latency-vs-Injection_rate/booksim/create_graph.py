#!/bin/python3 

import os
import pandas as pd
import matplotlib.pyplot as plt

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

input_filename = "results_8x8_0_63.txt"

parse_stats(input_filename)
plot_data()

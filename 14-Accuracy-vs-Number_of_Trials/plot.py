import pandas as pd
import numpy as np 
import seaborn as sns 
import matplotlib.pyplot as plt 
from matplotlib.ticker import MaxNLocator 

filename = "data"
xs = range(18)
labels = np.logspace(1,17,17,base=2)

def format_fn(tick_val, tick_pos):
  if int(tick_val) in xs:
    return labels[int(tick_val)]
  else:
    return ''

def plot_accuracices(df):
    plt.figure(figsize=(8,4), dpi=600)

    sns.set_theme(style="whitegrid")
    sns.color_palette("husl",8)
    ax = sns.lineplot(
        data = df,
        x = df['trials'],
        y = df['accuracy'],
        linewidth = 2,
        # hue = df['keys'],
        
        marker='X',
        markersize=10,
        color='red'
    )
    # plt.plot(df['trials'],df['accuracy'])
    ax.set_xscale('log', base=2)
    # plt.legend(fontsize=18,loc='lower left')

    plt.yticks(fontsize=18)
    plt.xticks(fontsize=18,ha='center',va='top',rotation=60)
    # xtick_values = range(1,18)
    xtick_labels = np.logspace(1,18,17,base=2)
    # ax.set_xticks(xtick_values)
    # ax.set_xticklabels(xtick_labels)
    plt.xlabel("Number of Trials", fontsize=24)
    plt.ylabel("Accuracy", fontsize=24)
    plt.ylim((0,1.1))
    plt.tight_layout()
    plt.savefig("key-extraction-accuracy.pdf")
    plt.savefig(filename+"_accuracy.png")



def process_df(filename):
    column_names = ["trials","keys","accuracy"]
    df = pd.read_csv(filename+".csv", header=None, names= column_names)  
    df = df[df['keys']==1024]
    print(df)
    return df 

if __name__=="__main__":
    df = process_df(filename)
    plot_accuracices(df)
    # X,y = create_dataset(df)
    # print(majority_results(X,y,AdaBoostClassifier))

    # l2_far_hit(0x3244,0x1234)
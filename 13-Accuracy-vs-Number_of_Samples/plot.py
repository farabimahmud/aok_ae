from more_itertools import sample
import pandas as pd
import numpy as np 
import seaborn as sns 
import matplotlib.pyplot as plt 
from sklearn.ensemble import AdaBoostClassifier
from sklearn.model_selection import train_test_split

def l2_far_hit(b0,b1):
    # cache_indices_in_b0 = set(list("{:0x}".format(b0)))
    # cache_indices_in_b1 = set(list("{:0x}".format(b1)))

    cache_indices_in_b0 = set(list(b0))
    cache_indices_in_b1 = set(list(b1))    
    new_accesses = cache_indices_in_b1.difference(cache_indices_in_b0)
    if ("2" in new_accesses) or ("4" in new_accesses):
        return 1
    return 0

filename = "timer-10k-10p-100t"
filename = "data"

def boxpl(filename):
    column_names = ["cache","b0","b1","b2","b3","plain","key","t0","t1","t2","t3"]
    df = pd.read_csv("data/"+filename+".csv", header = None, names= column_names)
    df = df[['b0','b1','t1','plain']]
    df['l2_hits'] = df.apply(lambda x: l2_far_hit(x.b0,x.b1))
    df = df.set_index('plain')
    grouped = df.groupby(level=['plain'])
    grouped.boxplot(figsize=(12,4),layout=(2,4),rot=45, fontsize=12)
    plt.savefig("outputs/"+filename+"_boxplot.png")
    print(df)

def process_df(filename):
    column_names = ["cache","b0","b1","b2","b3","plain","key","t0","t1","t2","t3"]
    df = pd.read_csv("data/"+filename+".csv", header = None, names= column_names)
    df = df[['b0','b1','t1','plain']]
    df['l2_hits'] = df.apply(lambda x: l2_far_hit(x.b0,x.b1),axis=1)
    df = df.set_index('plain')
    # grouped = df.groupby(level=['plain'])
    # grouped.boxplot(figsize=(12,4),layout=(2,4),rot=45, fontsize=12)
    # plt.savefig("outputs/"+filename+"_boxplot.png")
    print(df)
    print(df['l2_hits'].value_counts())
    return df 


def majority_results(X,y,classifier):
    clf = None
    clf = classifier(n_estimators=10)
    X_train, X_test, y_train, y_test = train_test_split(X,y,test_size=0.90, 
        shuffle=True)
    clf.fit(X_train,y_train)

    df_test = pd.DataFrame()
    df_test['X'] = X_test.ravel()
    df_test['y'] = y_test.ravel()
    preds = []
    df_test = df_test.sort_values(by='y').reset_index()
    for idx,row in df_test.iterrows():
        preds.append(clf.predict(row['X'].reshape(-1,1))[0])
    df_test['preds'] = preds
    # print(df_test)
    df_0 = df_test[df_test['y']==0]
    df_1 = df_test[df_test['y']==1]

    correct = 0
    N_TRIALS  = 1000
    np.random.seed(42)
    accuracies = []
    sample_range = range(1,100,5)
    for N_SAMPLES in range(1,100,5):
        correct = 0
        for t in range(N_TRIALS):
            df_sample = pd.DataFrame()
            if np.random.rand() < 0.5:
                df_sample = df_0.sample(n=N_SAMPLES)
            else:
                df_sample = df_1.sample(n=N_SAMPLES)
            prediction = df_sample.preds.mode()[0]        
            actual = df_sample['y'].to_numpy()[0]
            # print(prediction, actual)
            if prediction == actual:
                correct += 1 
        accuracies.append(correct/N_TRIALS)
        # print("{}".format(correct/N_TRIALS))
    return accuracies, sample_range

def plot_accuracices(accuracies, sample_range):
    accuracies.insert(0,0)
    sample_range.insert(0,0)
    plt.figure(figsize=(6,3), dpi=600)

    sns.set_theme(style="whitegrid", palette="hls")
    sns.lineplot(
        x = sample_range,
        y = accuracies,
        linewidth = 2,
        marker='X',
        markersize=10,
        color='red',
    )
    plt.yticks(fontsize=18)
    plt.xticks(fontsize=18,ha='center')

    plt.xlabel("Number of Samples", fontsize=24)
    plt.ylabel("Accuracy", fontsize=24)
    plt.ylim((0,1.1))
    plt.tight_layout()
    plt.savefig("ml-classifier-accuracy.pdf")
    plt.savefig(filename+"_accuracies.png")


def create_dataset(df):
    X = df['t1'].to_numpy().reshape(-1, 1)
    y = df['l2_hits'].to_numpy()
    return X,y  


if __name__=="__main__":
    # df = process_df(filename)
    # X,y = create_dataset(df)
    # print(majority_results(X,y,AdaBoostClassifier))
    
    acc_100 = [0.838, 0.885, 0.967, 0.96, 0.983, 0.987, 0.996, 0.996, 0.995, 0.998, 1.0, 0.999, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]
    sample_range = range(1,100,5)
    sample_range = list(sample_range)
    plot_accuracices(acc_100, sample_range)
    # l2_far_hit(0x3244,0x1234)
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import seaborn as sns 
import pandas as pd 
import collections

data_file='knl_data.csv'

target_file='all-core-one-addr.png'
target_pdf='all-core-one-addr.pdf'


def remove_outliers(df):
  cols = ['latency'] # one or more

  Q1 = df[cols].quantile(0.25)
  Q3 = df[cols].quantile(0.75)
  IQR = Q3 - Q1

  df = df[~((df[cols] < (Q1 - 1.5 * IQR)) |(df[cols] > (Q3 + 1.5 * IQR))).any(axis=1)]
  return df

column_names = ['vaddr','latency','core']
df = pd.read_csv(data_file,names=column_names)
df = remove_outliers(df)
grouped  = df.groupby('core').mean(numeric_only=True)
print("Here")
data = grouped.to_dict()['latency']
data = sorted(data.items(), key=lambda x:x[1], reverse=True)
# print(data)
# sorted_dict = collections.OrderedDict(data)
# print(sorted_dict)


# plt.gcf().set_size_inches(18, 4)
# plt.ylim([0,400])

plt.figure(figsize=(6,3))
ax = sns.barplot(
    x=df['core'],
    y=df['latency'],
    data = df,
    palette='husl',
    estimator = np.mean,
    ci = 0.95,
    # ci = 0.95,
)
cores = df['core'].unique()
xlabels = np.arange(0,72,4)

plt.yticks(fontsize=18)
plt.xticks(fontsize=18,ha='center')
plt.xticks(xlabels)
ax.set_xticklabels(
    xlabels, fontsize=14
)
plt.xlabel("CPU Core", fontsize=16)
plt.ylabel("LLC Hit Time (Cycle)", fontsize=16)
plt.tight_layout()
plt.savefig(target_file)
plt.savefig(target_pdf)

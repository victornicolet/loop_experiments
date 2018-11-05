import pandas as pd
import matplotlib.pyplot as plt

DATAFILE = '../data/explog_compsbk2.csv'


data = pd.read_csv(DATAFILE, sep=",")
data_summary = {'time': ['mean', 'std']}

all_groups = data.groupby(['name', 'threads']).agg(data_summary)
all_groups.columns = ["_".join(x) for x in all_groups.columns.ravel()]
all_groups = all_groups.reset_index()


def f(example):
    def spdu(s, x):
        if x != 0:
            return s/x
        else:
            return 1.

    seq_time = example[example['threads'] == 0]['time_mean'].sum()
    a = example['time_mean'].map(lambda x: spdu(seq_time, x))
    size_group = example.assign(speedup_mean=a)
    return size_group

examples = all_groups.groupby(['name'])

examples = [(s, f(x)) for s, x in examples]


fig = plt.figure()
ax = fig.add_subplot(1,1,1)

for s, example in examples:
    example.plot(x='threads', y=['speedup_mean'], label=[str(s)], ax=ax)

ax.set_ylim(0, 64)
plt.show()

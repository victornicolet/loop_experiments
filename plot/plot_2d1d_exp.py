import pandas as pd
import matplotlib.pyplot as plt

DATAFILE = '../data/explog_2d1d.csv'


data = pd.read_csv(DATAFILE, sep=",")
data_summary = {'OMP_TIME': ['mean', 'std'],'TBB_TIME': ['mean', 'std']}

all_groups = data.groupby(['ExpName', 'N', 'N2', 'NUM_THREADS']).agg(data_summary)
all_groups.columns = ["_".join(x) for x in all_groups.columns.ravel()]
all_groups = all_groups.reset_index()


def f(size_group):
    def spdu(s, x):
        if x != 0:
            return s/x
        else:
            return 1.
    seq_time = size_group[size_group['NUM_THREADS'] == 0]['TBB_TIME_mean'].sum()
    a = size_group['TBB_TIME_mean'].map(lambda x: spdu(seq_time, x))
    b = size_group['OMP_TIME_mean'].map(lambda x: spdu(seq_time, x))
    size_group = size_group.assign(TBB_TIME_mean_speedup=a, OMP_TIME_mean_speedup=b)
    return size_group


size_groups = all_groups.groupby(['ExpName', 'N', 'N2'])
size_groups = [(s, f(x)) for s, x in size_groups]

ax = {}
fig = {}

for (exp_name, n, n2), sgroup in size_groups:
    if exp_name not in ax:
        fig = plt.figure()
        ax[exp_name] = fig.add_subplot(1, 1, 1)
        ax[exp_name].set_ylim(0, 16)

for s, size_group in size_groups:
    exp_name, n, n2 = s
    print(n)
    if int(n) > 50000:
        size_group.plot(x='NUM_THREADS', y=['TBB_TIME_mean_speedup', 'OMP_TIME_mean_speedup'], label=['TBB ' + str(s), 'OMP ' + str(s)], ax=ax[exp_name])

    group_plot = size_group.plot.bar(x='NUM_THREADS',
                    y=['TBB_TIME_mean_speedup', 'OMP_TIME_mean_speedup'], label=['TBB ' + str(s), 'OMP ' + str(s)])

    group_plot.set_title(str(s))
    group_plot.set_ylim(0, 16)


plt.show()

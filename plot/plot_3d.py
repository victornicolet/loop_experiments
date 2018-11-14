import pandas as pd
import matplotlib.pyplot as plt
import sys

DATAFILE = '../data/explog_omp_mtb.csv'


data = pd.read_csv(DATAFILE, sep=",")
data_summary = { 'OMP_TIME': ['mean', 'std'],'TBB_TIME': ['mean', 'std']}

all_groups = data.groupby(['ExpName', 'N', 'M','L', 'NUM_THREADS']).agg(data_summary)
all_groups.columns = ["_".join(x) for x in all_groups.columns.ravel()]
all_groups = all_groups.reset_index()

def f(s,size_group):
    def spdu(s, x):
        if x != 0:
            return s/x
        else:
            return 1.
    seq_time = size_group[size_group['NUM_THREADS'] == 0]['TBB_TIME_mean'].sum()
    a = size_group['TBB_TIME_mean'].map(lambda x: spdu(seq_time, x))
    b = size_group['OMP_TIME_mean'].map(lambda x: spdu(seq_time, x))
    size_group = size_group.assign(TBB_TIME_mean_speedup=a, OMP_TIME_mean_speedup=b)
    tbb_thread16 = size_group[size_group['NUM_THREADS'] == 16]['TBB_TIME_mean_speedup'].iloc[0]
    omp_thread16 = size_group[size_group['NUM_THREADS'] == 16]['OMP_TIME_mean_speedup'].iloc[0]
    print(s, tbb_thread16, omp_thread16)
    return size_group

size_groups = all_groups.groupby(['ExpName', 'N', 'M', 'L'])

size_groups = [(s, f(s,x)) for s, x in size_groups]



fig = plt.figure()
ax = fig.add_subplot(1,1,1)

for s, size_group in size_groups:
    size_group.plot(x='NUM_THREADS',
                    y=['TBB_TIME_mean_speedup', 'OMP_TIME_mean_speedup'], label=['TBB ' + str(s), 'OMP ' + str(s)], ax=ax)

    group_plot = size_group.plot.bar(x='NUM_THREADS',
                                     y=['TBB_TIME_mean_speedup', 'OMP_TIME_mean_speedup'], label=['TBB ' + str(s), 'OMP ' + str(s)])

    group_plot.set_title(str(s))
    group_plot.set_ylim(0, 16)

ax.set_ylim(0, 16)
plt.show()

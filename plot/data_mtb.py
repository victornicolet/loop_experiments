import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

DATAFILE = '../data/explog_omp_mtb.csv'

dt = np.dtype([
    ('exname', np.string_),
    ('N', np.int32),
    ('M', np.int32),
    ('L', np.int32),
    ('num_threads', np.int8),
    ('tbb_time', np.float32),
    ('omp_time', np.float32)])

data = pd.read_csv(DATAFILE, sep=",").drop(['ExpName'], axis=1)
data_summary = { 'OMP_TIME': {'OMP_mean':'mean', 'OMP_std': 'std'},
                 'TBB_TIME': {'TBB_mean': 'mean', 'TBB_std': 'std'}}

group_stats = data.groupby(['N', 'M', 'L', 'NUM_THREADS']).agg(data_summary)
print(group_stats)
group_stats.to_csv('../data/omp_mtb_summary.csv')
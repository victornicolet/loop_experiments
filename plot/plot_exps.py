import sys
import pandas as pd
import matplotlib.pyplot as plt

DATAFILE = sys.argv[1]

MODE_64 = len(sys.argv) > 2

if MODE_64:
    best_num_threads = [0,1,2,3,4,5,6,7,8,9,10,12,14,16,18,20,24,28,32,40,48,52,64]
else:
    best_num_threads = [0,1,2,3,4,5,6,7,8,9,10,12,14,16,18,20,24,28,32]

data = pd.read_csv(DATAFILE, sep=",")
data = data[data['threads'].isin(best_num_threads)]
data_summary = {'time': ['mean', 'std']}

all_groups = data.groupby(['name', 'threads']).agg(data_summary)
all_groups.columns = ["_".join(x) for x in all_groups.columns.ravel()]
all_groups = all_groups.reset_index()

print("Name | Mean speedup | Seq time stddev | x16 tmean | x16 stddev")
def f(example):
    def spdu(s, x):
        if x != 0:
            return s/x
        else:
            return 1.

    seq_time = example[example['threads'] == 0]['time_mean'].sum()
    seq_time_std = example[example['threads'] == 0]['time_std'].sum()
    a = example['time_mean'].map(lambda x: spdu(seq_time, x))
    size_group = example.assign(speedup_mean=a)
    thread16 = size_group[size_group['threads'] == 16]['speedup_mean'].iloc[0]
    thread16_stddev = size_group[size_group['threads'] == 16]['time_std'].iloc[0]
    thread16_tmean = size_group[size_group['threads'] == 16]['time_mean'].iloc[0]
    perc_stddev = thread16_stddev / thread16_tmean
    print(size_group['name'].iloc[0], thread16, seq_time_std, thread16_tmean, thread16_stddev, perc_stddev)
    return size_group

examples = all_groups.groupby(['name'])

examples = [(s, f(x)) for s, x in examples]

colors = {
 '2d-sorted' :           ('xkcd:grey',         ':', 'sorted',            ',', 2),
 '2d-sum'  :             ('xkcd:purple',       ':', 'sum',               ',', 2),
 'gradient1' :           ('xkcd:green',        ':', 'vertical grad.',    ',', 2),
 'gradient2' :           ('xkcd:pink',         ':', 'diagonal grad.',    ',', 2),
 'minmax' :              ('xkcd:black',        ':', 'min-max',           ',', 2),
 'max-dist' :            ('mediumblue',        ':', 'max dist',          ',', 2),
 'increasing-ranges' :   ('xkcd:forest green', ':', 'increasing ranges', ',', 2),
 'max-balanced-substr' : ('xkcd:red',          ':', 'balanced substr.',  ',', 2),
 'max-top-strip' :       ('xkcd:turquoise',    ':', 'max top strip',     ',', 2),
 'max-bot-strip' :       ('xkcd:teal',         ':', 'max bot strip',     ',', 2),
 'max-seg-strip' :       ('xkcd:aqua',         ':', 'max seg strip',     ',', 2),

 #    3D, scalar examples
 'max-bottom-box' : ('xkcd:rust',          '--', 'mbbs',            ',', 3),
 'max-seg-box' :    ('xkcd:blue',          '--', 'max segment box', ',', 3),
 'max-top-box' :    ('xkcd:bluish green',  '--', 'max top box',     ',', 3),
 #    Linear stv
 'max-left-box' : (  'xkcd:aqua',        '-.', 'max left box',      ',', 2),
 'max-left-rect' : ( 'xkcd:brown',       '-.', 'max bot-left rect', ',', 2),
 'minmax-col' : (    'xkcd:salmon',      '-.', 'min-max-col',       ',', 2),
 'max-left-strip' : ('xkcd:magenta',     '-.', 'max left strip',    ',', 2),
 'mtlr' : (          'xkcd:indigo',      '-.', 'mtlr (Sec 2.2)',    ',', 2),
 'mtrr' : (          'xkcd:bright green','-.', 'max top-right rect',',', 2),
 'saddle-point' : (  'xkcd:hot pink',     '-', 'saddle point',      ',', 2),
 #    'ranges' examples
 'overlapping-ranges' :  ('xkcd:navy blue',  '-', 'overlapping r.', ',', 2),
 'pyramid-ranges' :      ('xkcd:grey',       '-', 'pyramid r.',     ',', 2),
 'intersecting-ranges' : ('xkcd:deep lilac', '-', 'intersecting r.',',', 2),
 'mode' :                ('xkcd:lime',       '-', 'mode',           ',', 2),
 #  Well balanced
 'well-balanced' : (      'xkcd:violet',     '-', 'bp (Sec 2.1)',  'x', 3)
}



fig = plt.figure(figsize=(14, 10), dpi=80, facecolor='w', edgecolor='k')
ax = fig.add_subplot(1,1,1)
# ax.plot(best_num_threads, best_num_threads, color=(0,0,0), linestyle='-')

for s, example in examples:
    example.to_csv('experiments.csv', header=None, index=None, sep=',', mode='a')
    cex, lex , lege, mrk, elw = colors[str(s)]
    example.plot(x='threads', y=['speedup_mean'],
                 label=[lege],
                 marker=mrk,
                 markersize=6,
                 linestyle=lex,
                 lw=elw,
                 color=cex, ax=ax)

if MODE_64:
    ax.set_ylim(0, 64)
else:
    ax.set_ylim(0, 35)

axes_fontsize=20
legend_fontsize=16

ax.set_ylabel('speedup parallel / sequential',fontsize=axes_fontsize)
ax.set_xlabel('number of threads', fontsize=axes_fontsize)
plt.setp(ax.get_xticklabels(), fontsize=axes_fontsize)
plt.setp(ax.get_yticklabels(), fontsize=axes_fontsize)
ax.grid(b=True, color=(0.8, 0.8, 0.8))


plt.legend(bbox_to_anchor=(0.01, 0.55, 0.45, 1.102), loc=3,
           ncol=2, mode="expand", borderaxespad=0., prop={'size': legend_fontsize})
fig.tight_layout()
plt.savefig("/home/victorn/repos/consynth/pldi19/figures/graphs/chart.pdf")
plt.show()

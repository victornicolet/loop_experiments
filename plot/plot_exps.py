import pandas as pd
import matplotlib.pyplot as plt

DATAFILE = '../data/explog_compsbk2.csv'

best_num_threads = [0,1,2,3,4,5,6,7,8,9,10,12,14,16,18,20,24,28,32]#,40,48,52,64]

data = pd.read_csv(DATAFILE, sep=",")
data = data[data['threads'].isin(best_num_threads)]
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
    thread16 = size_group[size_group['threads'] == 16]['speedup_mean'].iloc[0]
    thread16_stddev = size_group[size_group['threads'] == 16]['time_std'].iloc[0]
    print(size_group['name'].iloc[0], thread16, thread16_stddev)
    return size_group

examples = all_groups.groupby(['name'])

examples = [(s, f(x)) for s, x in examples]

colors = {
 '2d-sorted' : ('xkcd:sky blue', '-', 'sorted'), # high stddev, redo with larger input?
 '2d-sum'  : ('xkcd:purple', '-', 'sum'), # high stddev, redo with larger input?
 'gradient1' : ('xkcd:green', '-', 'vertical grad.'),
 'gradient2' : ('xkcd:pink', '-', 'diagonal grad.'),
 'max-dist' : ('mediumblue', '-', 'max dist'),
 'increasing-ranges' : ('xkcd:forest green', '-', 'increasing ranges'),
 'max-balanced-substr' : ('xkcd:red', '-', 'balanced substr.'),
 'max-top-strip' : ('xkcd:turquoise', '-', 'max top strip'),
 'max-bot-strip' : ('xkcd:teal', '-', 'max bot strip'),
 'max-seg-strip' : ('xkcd:aqua', '-', 'max seg strip'),
 'max-bottom-box' : ('xkcd:mustard', ':', 'mbbs'),
 'max-seg-box' : ('xkcd:light brown', ':', 'max segment box'),
 'max-top-box' : ('xkcd:tan', ':', 'max top box'), # redo and suppress data: stddev is too high.
 'max-left-rect' : ('xkcd:brown', '-.', 'max bot-left rect'),
 'minmax' : ('xkcd:black', '-', 'min-max'),
 'minmax-col' : ('xkcd:salmon', '-', 'min-max-col'),
 'mode' : ('xkcd:lime', '-', 'mode'),
 'max-left-strip' : ('xkcd:magenta', '-.', 'max left strip'),
 'mtlr' : ('xkcd:indigo', '-.', 'mtlr (Sec 2.2)'),
 'mtrr' : ('xkcd:bright green', '-.', 'max top-right rect'),
 'overlapping-ranges' : ('xkcd:navy blue', '-', 'overlapping r.'),
 'pyramid-ranges' : ('xkcd:grey', '-', 'pyramid r.'),
 'saddle-point' : ('xkcd:hot pink', '-', 'saddle point'),
 'well-balanced' : ('xkcd:violet', '--', 'wb (Sec 2.1)')
}



fig = plt.figure()
ax = fig.add_subplot(1,1,1)
# ax.plot(best_num_threads, best_num_threads, color=(0,0,0), linestyle='-')
for s, example in examples:
    cex, lex , lege = colors[str(s)]
    example.plot(x='threads', y=['speedup_mean'],
                 label=[lege],
                 linestyle=lex,
                 color=cex, ax=ax)

ax.set_ylim(0, 35)
ax.set_ylabel('speedup parallel / sequential input')
ax.grid(b=True, color = (0.8,0.8,0.8))

# box = ax.get_position()
# ax.set_position([box.x0, box.y0 + box.height * 0.1,
#                  box.width, box.height * 0.9])
#
# # Put a legend below current axis
# ax.legend(loc='lower center', bbox_to_anchor=(0.5, -0.05),
#           fancybox=True, shadow=True, ncol=5)
plt.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,
           ncol=5, mode="expand", borderaxespad=0.)
plt.show()

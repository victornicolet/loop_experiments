import pandas as pd
import matplotlib.pyplot as plt

DATAFILE = '../data/explog_compsbk2.csv'

best_num_threads = [0,1,2,3,4,5,6,7,8,9,10,12,14,16,18,20,24,28,32] #,40,48,52,64]

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
    return size_group

examples = all_groups.groupby(['name'])

examples = [(s, f(x)) for s, x in examples]

#
# 2d-sorted
# 2d-sum
# gradient1
# gradient2
# increasing-ranges
# max-balanced-substr
# max-bot-strip
# max-bottom-box
# max-left-rect
# max-seg-strip
# max-top-box
# minmax
# minmax-col
# mode
# mtlr
# mtrr
# overlapping-ranges
# pyramid-ranges
# saddle-point
# well-balanced

colors = {
 '2d-sorted' : ('xkcd:sky blue', '-'),
 '2d-sum'  : ('xkcd:purple', '-'),
 'gradient1' : ('xkcd:green', '-'),
 'gradient2' : ('xkcd:pink', '-'),
 'max-dist' : ('mediumblue', '-'),
 'increasing-ranges' : ('xkcd:forest green', '-'),
 'max-balanced-substr' : ('xkcd:red', '-'),
 'max-top-strip' : ('xkcd:turquoise', '-'),
 'max-bot-strip' : ('xkcd:teal', '-'),
 'max-left-strip' : ('xkcd:cyan', '-'),
 'max-seg-strip' : ('xkcd:aqua', '-'),
 'max-bottom-box' : ('xkcd:mustard', ':'),
 'max-seg-box' : ('xkcd:light brown', ':'),
 'max-top-box' : ('xkcd:tan', ':'),
 'max-left-rect' : ('xkcd:brown', '-.'),
 'minmax' : ('xkcd:black', '-'),
 'minmax-col' : ('xkcd:salmon', '-'),
 'mode' : ('xkcd:lime', '-'),
 'mtlr' : ('xkcd:indigo', '-.'),
 'mtrr' : ('xkcd:bright green', '-.'),
 'overlapping-ranges' : ('xkcd:navy blue', '-'),
 'pyramid-ranges' : ('xkcd:grey', '-'),
 'saddle-point' : ('xkcd:hot pink', '-'),
 'well-balanced' : ('xkcd:violet', '--')
}



fig = plt.figure()
ax = fig.add_subplot(1,1,1)

for s, example in examples:
    cex, lex = colors[str(s)]
    example.plot(x='threads', y=['speedup_mean'],
                 label=[str(s)],
                 linestyle=lex,
                 color=cex, ax=ax)

ax.set_ylim(0, 40)

# box = ax.get_position()
# ax.set_position([box.x0, box.y0 + box.height * 0.1,
#                  box.width, box.height * 0.9])
#
# # Put a legend below current axis
# ax.legend(loc='lower center', bbox_to_anchor=(0.5, -0.05),
#           fancybox=True, shadow=True, ncol=5)
plt.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,
           ncol=4, mode="expand", borderaxespad=0.)
plt.show()

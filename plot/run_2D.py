
from subprocess import call


sizes_distrib = [
    [10, 5],
    [5, 10],
    [5, 5],
    [40, 5],
]

sizes_factor = 6000
sizes_offset = 145

sizes = [[x * sizes_factor + sizes_offset for x in y] for y in sizes_distrib]

for i in range(0, 10):
    for n, m in sizes:
        with open('data/explog_2d.csv', 'a+') as myoutfile:
            print("SIZE: %i x %i.\n" % (n, m))
            call(["./" + "ExpGlobalBal", str(n), str(m)], stdout=myoutfile)


from subprocess import call


sizes_distrib = [
    [20, 0],
    [14, 0],
    # [21, 0],
    # [17, 0],
]

sizes_factor = 5000
sizes_offset = 943

sizes = [[x * sizes_factor + sizes_offset for x in y] for y in sizes_distrib]

for i in range(0, 5):
    for n, m in sizes:
        with open('data/explog_2d1d.csv', 'a+') as myoutfile:
            print("SIZE: %i x %i.\n" % (n, m))
            call(["./" + "ExpMode", str(n), str(m)], stdout=myoutfile)

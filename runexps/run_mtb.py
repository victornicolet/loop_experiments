
from subprocess import call


sizes_distrib = [
    # [10, 5, 20],
    # [5, 10, 20],
    # [5, 5, 40],
    [20, 10, 10],
]

sizes_factor = 130
sizes_offset = 5

sizes = [[x * sizes_factor + sizes_offset for x in y] for y in sizes_distrib]

for i in range(0, 10):
    for n, m, l in sizes:
        with open('data/explog_omp_mtb.csv', 'a+') as myoutfile:
            print("SIZE: %i x %i x %i.\n" % (n, m, l))
            # call(["./" + "OMPExpMaxTopBox", str(n), str(m), str(l)], stdout=myoutfile)
            call(["./" + "OMPExpMaxBotBox", str(n), str(m), str(l)], stdout=myoutfile)

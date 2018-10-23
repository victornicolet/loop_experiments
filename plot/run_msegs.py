from subprocess import call


sizes = [
         [8000, 512, 512],
         [8000, 64, 64]
         ]

num_cores = [1, 2, 3, 4, 8, 12, 16]

for n,m,l in sizes:
    for nc in num_cores:
        with open('explog_msegs.csv', 'a+') as myoutfile:
            print("SIZE: %i x %i x %i.\n" % (n, m, l))
            call(["./" + "ExpMaxSegBox", str(nc), str(n), str(m), str(l)], stdout=myoutfile)

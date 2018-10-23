import csv

# INFILE = "explog_compsbk2.csv"
INFILE = "explog_msegs.csv"
# OUTFILE = "speedups_compsbk2.csv"
OUTFILE = "speedups_msegs.csv"

data = {}

exp_cores = [1, 2, 3, 4, 8, 12, 16]

with open(INFILE, 'r') as csvfile:
    sreader = csv.reader(csvfile, delimiter=',')
    for row in sreader:
        exnm = row[0][5:]
        n = int(row[1])
        m = int(row[2])
        l = int(row[3])
        ex_key = "%s_%i-%i-%i" % (exnm, n, m, l)
        n_cores = int(row[4])
        time = float(row[5])

        if ex_key in data:
            data[ex_key][n_cores] = time
        else:
            data[ex_key] = {n_cores: time}

with open(OUTFILE, 'w') as csvout:

    csvout.write("Example key," + ",".join([str(x) for x in exp_cores]) + "\n")

    for ex, stats in data.items():
        onec = stats[1]
        st = [onec / stats[k] for k in sorted(stats.keys())]
        csvout.write(ex + "," + ",".join([str(x) for x in st]) + "\n")

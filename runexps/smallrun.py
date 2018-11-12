from subprocess import call

experiments = [
    # ('Exp2DSorted', int(5e5), int(1e4), 0),
    # ('Exp2DSum', int(1e5), int(5e4), 0),
    # ('ExpGradient1', int(5e5), int(1e4), 0),
    # ('ExpGradient2', int(5e5), int(1e4), 0),
    # ('ExpMinMax', int(3e5), int(1e4), 0),
    # ('ExpMaxDist', int(5e4), int(5e4), 0),
    # ('ExpMinMaxCol', int(3e5), int(1e4), 0),
    # ('ExpMaxLeftRect', int(9e3), int(9e3), 0),
    # ('ExpMaxTopStrip', int(3e5), int(2e4), 0),
    ('ExpMaxBottomStrip', int(1e4), int(5e4), 0),
    # ('ExpMaxSegStrip', int(1e5), int(5e4), 0),
    # ('ExpMaxLeftStrip', int(9e3), int(9e3), 0),
    # ('ExpMaxTopBox', int(2e3), int(2e3), int(2e3)),
    # ('ExpMaxBottomBox', int(2e3), int(2e3), int(1e3)),
    # ('ExpMaxSegBox', int(2e3), int(2e3), int(1e3)),
    # ('ExpMaxLeftBox', int(2e3), int(2e3), int(2e3)),
    # ('ExpMTLR', int(9e3), int(9e3), 0),
    # ('ExpMTRR', int(9e3), int(9e3), 0),
    # ('ExpSaddlePoint', int(15e4), int(1e4), 0),
    # ('ExpMode', int(7e4), 0, 0),
    # ('ExpOverlappingRanges', int(2e5), int(1e4), 0),
    # ('ExpPyramidRanges', int(2e5), int(1e4), 0),
    # ('ExpIntersectingRanges', int(2e4), 0, 0),
    # ('ExpIncreasingRanges', int(2e5), int(1e4), 0),
    # ('ExpMaxBalSub', int(5e4), 0, 0),
    # ('ExpWellBalanced', int(2e5), int(1e4), 0)
]


for expr, n, m, l in experiments:
    for i in range(1, 3):
        with open('data/small_exp_explog.csv', 'a+') as myoutfile:
            call(["./" + expr, str(n), str(m), str(l)], stdout=myoutfile)

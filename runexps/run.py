from subprocess import call

experiments = [
    ('Exp2DSorted', int(1e6), int(1e4), 0),
    ('Exp2DSum', int(1e5), int(5e4), 0),
    ('ExpGradient1', int(5e6), int(1e4), 0),
    ('ExpGradient2', int(5e6), int(1e4), 0),
    ('ExpMinMax', int(3e6), int(1e4)),
    ('ExpMaxDist', int(4e5), int(4e5)),
    ('ExpMinMaxCol', int(3e6), int(1e4)),
    ('ExpMaxLeftRect', int(3e6), int(1e4)),
    ('ExpMaxTopStrip', int(3e6), int(2e4)),
    ('ExpMaxBottomStrip', int(1e6), int(5e4)),
    # 'ExpMaxSegStrip',
    # 'ExpMaxTopBox',
    # 'ExpMaxTopBoxInPar'
    # 'ExpMaxBottomBox',
    # 'ExpMaxSegBox',
    #'ExpMTLR',
 #   'ExpMTRR',
  #  'ExpSaddlePoint',
#    'ExpMode'
    # 'ExpOverlapping',
    # 'ExpMaxBalSub',
    # 'ExpPyramidRange',
    # 'ExpIncreasingRange'
]


for expr, n, m, l in experiments:
    for i in range(1, 20):
        with open('data/explog.csv', 'a+') as myoutfile:
            call(["./" + expr, str(n), str(m), str(l)], stdout=myoutfile)

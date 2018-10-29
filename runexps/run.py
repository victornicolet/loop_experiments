from subprocess import call

experiments = [
    ('Exp2DSorted', int(10e6), int(10e4), 0),
    ('Exp2DSum', int(10e6), int(10e4), 0),
    # 'ExpGradient1',
    # 'ExpGradient2',
    # 'ExpMinMax',
    # 'ExpMaxDist',
    # 'ExpMinMaxCol',
    #  'ExpMaxLeftRect'
    # 'ExpMaxTopStrip',
    # 'ExpMaxBottomStrip',
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
    for i in range(1,10):
        with open('data/explog.csv', 'a+') as myoutfile:
            call(["./" + expr, str(n), str(m), str(l)], stdout=myoutfile)

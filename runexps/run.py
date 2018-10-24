from subprocess import call

experiments = [ # 'Exp2DSorted',  
    # 'Exp2DSum',
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
    'ExpMTLR',
 #   'ExpMTRR',
  #  'ExpSaddlePoint',
#    'ExpMode'
    # 'ExpOverlapping',
    # 'ExpMaxBalSub',
    # 'ExpPyramidRange',
    # 'ExpIncreasingRange'
]

num_cores = [0, 1, 2, 3, 4, 6, 8, 10, 12, 16]
# 24, 32, 46, 58, 64]


for expr in experiments:
    for nc in num_cores:
        with open('explog.csv', 'a+') as myoutfile:
            call(["./" + expr, str(nc)], stdout=myoutfile)

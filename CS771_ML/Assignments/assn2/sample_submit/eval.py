import utils
import predict
import time as tm
import numpy as np

# This file is intended to demonstrate how we would evaluate your code
# The data loader needs to know how many feature dimensions are there
dictSize = 225
(X, y) = utils.loadData( "train", dictSize = dictSize )

# Get error class predictions from predict.py and time the thing
tic = tm.perf_counter()
yPred = predict.findErrorClass( X, 5 )
toc = tm.perf_counter()

print( "Total time taken is %.6f seconds " % (toc - tic) )

preck = utils.getPrecAtK( y, yPred, 5 )
# The macro precision code takes a bit longer to execute due to the for loop over labels
mpreck = utils.getMPrecAtK( y, yPred, 5 )

# According to our definitions, both prec@k and mprec@k should go up as k goes up i.e. for your
# method, prec@i > prec@j if i > j and mprec@i > mprec@j if i > j. See the assignment description
# to convince yourself why this must be the case.

print( "prec@1: %0.3f" % preck[0], "prec@3: %0.3f" % preck[2], "prec@5: %0.3f" % preck[4] )
# Dont be surprised if mprec is small -- it is hard to do well on rare error classes
print( "mprec@1: %0.3e" % mpreck[0], "mprec@3: %0.3e" % mpreck[2], "mprec@5: %0.3e" % mpreck[4] )
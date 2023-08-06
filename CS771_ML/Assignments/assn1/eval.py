import numpy as np
from submit import get_renamed_labels
from submit import solver
from submit import get_features
import time as tm

# Find out how much hinge loss the learnt model incurred
def get_hinge_loss( X, y, w, b ):
	XX = get_features( X )
	scores = XX.dot( w ) + b
	hinge_loss = 1 - np.multiply( scores, get_renamed_labels( y ) )
	hinge_loss[ hinge_loss < 0 ] = 0
	return np.average( hinge_loss )

# Find out the misclassification error rate of the model
def get_misclassification_rate( X, y, w, b ):
	XX = get_features( X )
	scores = XX.dot( w ) + b
	predictions = np.ones_like( scores )
	predictions[ scores < 0 ] = -1
	return 1 - np.average( get_renamed_labels( y ) == predictions )

Z_trn = np.loadtxt( "secret_train.dat" )
Z_tst = np.loadtxt( "secret_test.dat" )

# To avoid unlucky outcomes try running the code several times
num_trials = 5

# Try various timeouts - the timeouts are in seconds
timeouts = np.array( [ 0.2, 0.5, 1, 2, 5 ] )

# Try checking for timeout every 10 iterations
spacing = 10

result = np.zeros( ( len( timeouts ), 4 ) )

for i in range( len( timeouts ) ):
	to = timeouts[i]
	print( "\ttimeout =", to )
	avg_hinge = 0
	avg_error = 0
	avg_time_reported = 0
	avg_time_wrapper = 0
	for t in range( num_trials ):
		print( "\t\tTrial %d of %d" % ( t + 1, num_trials ) )
		tic = tm.perf_counter()
		( w, b, totTime ) = solver( Z_trn[:,:-1], Z_trn[:,-1], to, spacing )
		toc = tm.perf_counter()
		avg_hinge += get_hinge_loss( Z_tst[:,:-1], Z_tst[:,-1], w, b )
		avg_error += get_misclassification_rate( Z_tst[:,:-1], Z_tst[:,-1], w, b )
		avg_time_reported += totTime
		avg_time_wrapper += toc - tic
	result[i, 0] = avg_hinge/num_trials
	result[i, 1] = avg_error/num_trials
	result[i, 2] = avg_time_reported/num_trials
	result[i, 3] = avg_time_wrapper/num_trials

np.savetxt( "result", result, fmt = "%.6f" )
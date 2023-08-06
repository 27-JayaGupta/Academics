'''
	Package: cs771
	Module: assn2Utils
	Author: Puru
	Institution: CSE, IIT Kanpur
	License: GNU GPL v3.0
	
	Various utilities for multi-label learning problems
'''

import numpy as np
from sklearn.datasets import load_svmlight_file
from sklearn.datasets import dump_svmlight_file
from scipy import sparse as sps

def loadData( filename, dictSize = 225 ):
	X, y = load_svmlight_file( filename, multilabel = False, n_features = dictSize, offset = 1 )
	return (X, y)

def dumpData( X, y, filename ):
	(n, d) = X.shape
	assert len(y) == n, "Mismatch in number of feature vectors and number of label vectors"
	dump_svmlight_file( X, y, filename, multilabel = False, zero_based = True, comment = "%d, %d" % (n, d) )

# Not the best way to do things in Python but I could not find a neater workaround
# Let me know if you know one that avoids a messy loop
def removeDuplicates( pred, imputation ):
	# Create a new array so that the original input array pred is unaffected
	deDup = np.ones( pred.shape ) * imputation
	for i in range( pred.shape[0] ):
		# Retain only the first occurrence of a label in every row
		(u, idx) = np.unique( pred[i,:], return_index = True )
		deDup[i,idx] = u
	return deDup

# Validate that data is nice and well behaved
# Return a copy of the predicted error classes that removes duplicates
# The original data is not affected i.e. this method can be called repeatedly
# Also return a one-hot representation of the gold labels for easier processing
# without affecting the arguments sent as inputs
def validateAndCleanup( yGold, yPred, k ):
	n = len(yGold)

	# Make sure the prediction matrix is in correct shape
	assert yPred.shape[0] == n, "Mismatch in number of test data points and number of predictions"
	assert yPred.shape[1] == k, "Mismatch in number of predictions received and number expected"

	# Penalize duplicates in yPred by replacing them with predictions of the dummy error class 0
	# Since error classes are numbered from 1 to 50, the 0 error class is a safe dummy choice 
	yPredNew = removeDuplicates( yPred, 0 )
	
	# Need to convert the gold labels into a one-hot representation to make things easier later on
	yGoldNew = np.zeros( (n, 50) )
	# The -1 step is required since gold labels are indexed 1 ... 50 whereas Python expects zero_based indices
	yGoldNew[ np.arange(n), yGold[:,np.newaxis].astype(int).T - 1 ] = 1

	return (yGoldNew, yPredNew)

# For a given value of k, return prec@1, prec@2, ..., prec@k
def getPrecAtK( yGold, yPred, k ):
	n = len(yGold)
	(yGoldNew, yPredNew) = validateAndCleanup( yGold, yPred, k )
	
	# Use some fancy indexing (yes, this is the formal term for the technique)
	# to find out where all did we predict the correct error class
	# Python indexing with arrays creates copies of data so we are safe
	# The -1 step is required since predicted labels are indexed 1 ... 50 whereas Python expects zero_based indices
	wins = yGoldNew[ np.arange( n )[:,np.newaxis], yPredNew.astype(int) - 1 ]
	
	# Find how many times did we correctly predict the correct class at the blah-th position
	totWins = np.sum( wins, axis = 0 )
	# Find how many times did we correctly predict the correct class at any one of the top blah positions
	cumWins = np.cumsum( totWins )
	
	# Normalize and return
	precAtK = cumWins / n
	return precAtK

# For a given value of k, return mprec@1, mprec@2, ..., mprec@k
def getMPrecAtK( yGold, yPred, k ):
	C = 50 # There are 50 error classes
	CEff = 0 # Number of classes that actually have a presence in the test data
	(yGoldNew, yPredNew) = validateAndCleanup( yGold, yPred, k )
	mPrecAtK = np.zeros( k )

	# For all real error classes (exclude the dummy error class)
	for cls in range( C ):
		# Find data points for which this is the error class
		pointsWithThisErrorClass = (yGoldNew[:, cls] == 1).reshape( -1 )
		n_cls = np.sum( yGoldNew[:, cls] )

		# If there exist data points for which this is the correct error class
		if n_cls > 0:
			# Find all data points for which this class was predicted
			# Create a new array so that the array yPredNew is unaffected since it has to be reused
			# The +1 is required since the counter cls runs from 0 to 49 whereas the predictions are from 1 to 50
			winsThisClass = np.zeros( yPredNew.shape )
			winsThisClass[ yPredNew == cls + 1 ] = 1
			# Remove cases of data points for which this is not the correct error class
			winsThisClass[ ~pointsWithThisErrorClass, : ] = 0
			# How many times did we correctly predict this error class at the blah-th position?
			totWinsThisClass = np.sum( winsThisClass, axis = 0 )
			# Find how many times did we correctly predict this error class at any one of the top blah positions
			cumWins = np.cumsum( totWinsThisClass )
			# Add the wins to mprec@blah
			mPrecAtK += cumWins / n_cls
			CEff += 1
	return mPrecAtK / CEff

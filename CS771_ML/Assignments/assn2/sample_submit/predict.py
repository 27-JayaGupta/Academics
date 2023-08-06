# import numpy as np
from numpy import random as rand
import numpy as np 
import scipy
import sklearn 
from sklearn.model_selection import train_test_split
import gc
gc.collect()
import scipy
from sklearn.linear_model import LogisticRegression
import pickle
import pandas as pd
from tensorflow.keras.models import load_model
import tensorflow as tf
# DO NOT CHANGE THE NAME OF THIS METHOD OR ITS INPUT OUTPUT BEHAVIOR

# PLEASE BE CAREFUL THAT ERROR CLASS NUMBERS START FROM 1 AND NOT 0. THUS, THE FIFTY ERROR CLASSES ARE
# NUMBERED AS 1 2 ... 50 AND NOT THE USUAL 0 1 ... 49. PLEASE ALSO NOTE THAT ERROR CLASSES 33, 36, 38
# NEVER APPEAR IN THE TRAINING SET NOR WILL THEY EVER APPEAR IN THE SECRET TEST SET (THEY ARE TOO RARE)

# Input Convention
# X: n x d matrix in csr_matrix format containing d-dim (sparse) bag-of-words features for n test data points
# k: the number of compiler error class guesses to be returned for each test data point in ranked order

# Output Convention
# The method must return an n x k numpy nd-array (not numpy matrix or scipy matrix) of classes with the i-th row 
# containing k error classes which it thinks are most likely to be the correct error class for the i-th test point.
# Class numbers must be returned in ranked order i.e. the label yPred[i][0] must be the best guess for the error class
# for the i-th data point followed by yPred[i][1] and so on.

# CAUTION: Make sure that you return (yPred below) an n x k numpy nd-array and not a numpy/scipy/sparse matrix
# Thus, the returned matrix will always be a dense matrix. The evaluation code may misbehave and give unexpected
# # results if an nd-array is not returned. Please be careful that classes are numbered from 1 to 50 and not 0 to 49.



def findErrorClass( X, k ):
	n = X.shape[0]
	saved_model =load_model('romit.h5')
	probs=saved_model.predict(X)
	ind_1=np.flip(np.argsort(probs, axis=1), axis=1) + 1
	ind=ind_1[:,:k]

	return ind
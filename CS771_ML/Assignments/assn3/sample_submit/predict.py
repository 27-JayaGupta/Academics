import cv2 as cv
import numpy as np
from tensorflow.keras.models import load_model
import tensorflow as tf
# DO NOT CHANGE THE NAME OF THIS METHOD OR ITS INPUT OUTPUT BEHAVIOR

# INPUT CONVENTION
# filenames: a list of strings containing filenames of images

# OUTPUT CONVENTION
# The method must return a list of strings. Make sure that the length of the list is the same as
# the number of filenames that were given. The evaluation code may give unexpected results if
# this convention is not followed.

scale=1
trim = 50

def process_img(img):

    process_img = []
    img = cv.resize(img, (img.shape[1]*scale, img.shape[0]*scale))

    hsv = cv.cvtColor(img,cv.COLOR_BGR2HSV)
    channels = cv.split(hsv)
    V = channels[2]
    _, thresh = cv.threshold(V, 200, 255, cv.THRESH_BINARY)

    vertical_hist = thresh.shape[0] - np.sum(thresh,axis=0,keepdims=True)/255

    flag = 0
    j = 10
    char_count = 0
    it = 0
    segment = np.zeros((thresh.shape[0], (thresh.shape[1]//3)- trim) , dtype=np.uint8)
    segment.fill(255)

    for v in vertical_hist[0]:
        it += 1

        if(v != 0):
            flag = 1
            if(j < ((thresh.shape[1]//3) - trim)):
                segment[:,j] = thresh[:, it]
                j += 1
            continue
        
        if(flag == 1):
            char_count += 1
            process_img.append(segment)
            flag = 0
            j = 10
            segment = np.zeros((thresh.shape[0], (thresh.shape[1]//3) - trim) , dtype=np.uint8)
            segment.fill(255)
    
    return process_img

labels_dict = {'OMICRON': 0, 'TAU': 1, 'RHO': 2, 'UPSILON': 3, 'MU': 4, 'KAPPA': 5, 
				'NU': 6, 'THETA': 7, 'GAMMA': 8, 'CHI': 9, 'ZETA': 10, 'EPSILON': 11, 
				'PI': 12, 'DELTA': 13, 'PHI': 14, 'PSI': 15, 'LAMDA': 16, 'BETA': 17, 
				'IOTA': 18, 'OMEGA': 19, 'SIGMA': 20, 'XI': 21, 'ETA': 22, 'ALPHA': 23}

def decaptcha( filenames ):
	# The use of a model file is just for sake of illustration

	# load model
	saved_model =load_model('greeek.h5')

	labels = []
	images= []

	for filename in filenames:
		img = cv.imread(filename, cv.IMREAD_COLOR)
		if(img.size == 0):
			print("Could not read the image: ",filename)
		process_segmented_img = process_img(img)
		images.append(process_segmented_img[0])
		images.append(process_segmented_img[1])
		images.append(process_segmented_img[2])

	images = np.array(images)
	images = np.reshape(images, (images.shape[0], images.shape[1], images.shape[2], 1))
	
	pred_labels = saved_model.predict(images)
	
	result = tf.argmax(pred_labels, axis=1)

	result = np.reshape(result, (len(result)//3, 3))
	
	dict_list = list(labels_dict)

	for label_list in result:
		pred_labels = ""
		pred_labels = pred_labels + dict_list[label_list[0]] + "," + dict_list[label_list[1]] + "," + dict_list[label_list[2]]
		labels.append(pred_labels)
	
	return labels
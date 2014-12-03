import numpy as np
import random
from sklearn import linear_model

# open file to get file name
nameFile = open('log.txt', 'r')

# load data from file
dataFile = open('complete_data.txt', 'r')

# skip first line
dataFile.readline()

# loop through data in file
features = []
targets = []
names = []

for line in dataFile:

    vals = line.rstrip().split(' ')
    
    # get targets
    names.append(vals[0])
    targets.append(float(vals[1]))

    # get features
    temp = []
    count = 0
    for i in vals[2:]:
        if(count == 0 and float(i) == 0):
            print i, vals[0]
        temp.append(float(i))
        count+=1
    features.append(temp)

# randomize the data
size = len(targets)
randomVals = range(size)
random.shuffle(randomVals)
#print randomVals

testTargets = []
testFeatures = []
testNames = []
trainingTargets = []
trainingFeatures = []
trainingNames = []

# generate training data
for i in range(len(randomVals))[:int(.1*size)]:
    trainingTargets.append(targets[randomVals[i]])
    trainingFeatures.append(features[randomVals[i]])
    trainingNames.append(names[randomVals[i]])

# generate test data
for i in range(len(randomVals))[int(size * .9):]:
    testTargets.append(targets[randomVals[i]])
    testFeatures.append(features[randomVals[i]])
    testNames.append(names[randomVals[i]])

# create linear regression object
regr = linear_model.LinearRegression()

# train the model using the training sets
regr.fit(trainingFeatures, trainingTargets)

# output results
for i in range(0, len(testFeatures)):
    weights = regr.coef_ * testFeatures[i]
    sum = 0
    for j in range(len(weights)):
        sum += weights[j]

    errorsPerLine = 0
    if testFeatures[i][2] > 0:
        errorsPerLine = testTargets[i]/testFeatures[i][2]
    else: 
        errorsPerLine = testTargets[i]

    #print testNames[i], round(sum), testTargets[i], testFeatures[i][2], errorsPerLine 

#print regr.coef_
#print regr.score(testFeatures, testTargets)

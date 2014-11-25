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
    for i in vals[2:]:
        temp.append(float(i))
    features.append(temp)

# randomize the data
size = len(targets)
randomVals = range(size)
random.shuffle(randomVals)
print randomVals

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

#print features
#separate data into training and test
#testTargets = targets[int(size * .9):]
#testFeatures = features[int(size * .9):]
#testNames = names[int(size * .9):]
#features = features[:int(.1*size)]
#targets = targets[:int(.1*size)]
#names = names[:int(.1*size)]

#print size, len(testTargets), len(targets)

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
    print testNames[i], round(sum), testTargets[i] 

print regr.coef_
print regr.score(testFeatures, testTargets)

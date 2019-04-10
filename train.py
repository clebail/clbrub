#!/usr/bin/env python
# -*- coding: utf-8 -*-

from keras.models import Sequential
from keras.layers import Dense
from keras.layers import Conv2D
from keras.layers import Dropout
from keras.layers import Flatten
import matplotlib.pyplot as plt
import numpy

numpy.random.seed(7)

dataset = numpy.loadtxt("trains.csv", delimiter=";")

X = dataset[:,0:324]
Y = dataset[:,324:]

model = Sequential()

X = X.reshape(X.shape[0], 54, 6, 1).astype('int32')

model.add(Conv2D(324, kernel_size=(3, 3), activation='relu', input_shape=(54, 6, 1)))
model.add(Conv2D(54, kernel_size=(3, 3), activation='relu'))
model.add(Flatten())
model.add(Dense(6, activation='relu'))
model.add(Dropout(0.5))
model.add(Dense(18, activation='softmax'))

model.compile(loss='cosine_proximity', optimizer='adam', metrics=['accuracy'])

history = model.fit(X, Y, steps_per_epoch=50, epochs=10, verbose=1, validation_data=None, initial_epoch=0)

model_json = model.to_json()
with open("model.json", "w") as json_file:
    json_file.write(model_json)
model.save_weights("model.h5")



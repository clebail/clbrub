#!/usr/bin/env python
# -*- coding: utf-8 -*-

from keras.models import Sequential
from keras.layers import Dense
import numpy

numpy.random.seed(7)

dataset = numpy.loadtxt("trains.csv", delimiter=";")

X = dataset[:,0:27]
Y = dataset[:,27:]

model = Sequential()
model.add(Dense(72, input_dim=27, activation='softmax'))
model.add(Dense(36, activation='softmax'))
model.add(Dense(18, activation='sigmoid'))

model.compile(loss='cosine_proximity', optimizer='adam', metrics=['accuracy'])

model.fit(X, Y, validation_split=0.33, epochs=100, batch_size=10, verbose=1)

model_json = model.to_json()
with open("model.json", "w") as json_file:
    json_file.write(model_json)
model.save_weights("model.h5")



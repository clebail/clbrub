#!/usr/bin/env python
# -*- coding: utf-8 -*-

from keras.models import Sequential
from keras.layers import Dense
from keras.layers import Conv1D
from keras.layers import MaxPooling1D
from keras.layers import Flatten
import matplotlib.pyplot as plt
import numpy

numpy.random.seed(7)

dataset = numpy.loadtxt("trains.csv", delimiter=";")

X = dataset[:,0:324]
Y = dataset[:,324:]

model = Sequential()
#model.add(Dense(324, input_dim=324, activation='relu'))
#model.add(Dense(54, activation='sigmoid'))
#model.add(Dense(6, activation='relu'))
#model.add(Dense(18, activation='sigmoid'))

X = X.reshape(X.shape[0], 54, 6).astype('int32')

model.add(Conv1D(324, kernel_size=(5), activation='relu', input_shape=(54, 6)))
model.add(MaxPooling1D())
model.add(Conv1D(54, kernel_size=(5), activation='relu'))
model.add(MaxPooling1D())
model.add(Conv1D(6, kernel_size=(5), activation='relu'))
model.add(MaxPooling1D())
model.add(Flatten())
model.add(Dense(18, activation='relu'))
model.add(Dense(18, activation='softmax'))

model.compile(loss='cosine_proximity', optimizer='adam', metrics=['accuracy'])

history = model.fit(X, Y, validation_split=0.33, epochs=100, batch_size=10, verbose=1)

print(history.history.keys())

plt.plot(history.history['acc'])
plt.plot(history.history['val_acc'])
plt.title('model accuracy')
plt.ylabel('accuracy')
plt.xlabel('epoch')
plt.legend(['train', 'test'], loc='upper left')
plt.show()

plt.plot(history.history['loss'])
plt.plot(history.history['val_loss'])
plt.title('model loss')
plt.ylabel('loss')
plt.xlabel('epoch')
plt.legend(['train', 'test'], loc='upper left')
plt.show()

model_json = model.to_json()
with open("model.json", "w") as json_file:
    json_file.write(model_json)
model.save_weights("model.h5")



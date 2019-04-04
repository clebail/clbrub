#!/usr/bin/env python
# -*- coding: utf-8 -*-

import rubik
from keras.models import model_from_json
import numpy

mvt = ["U", "U'", "D", "D'", "R", "R'", "L", "L'", "F", "F'", "B", "B'", "M", "M'", "E", "E'", "S", "S'"]

json_file = open('model.json', 'r')
loaded_model_json = json_file.read()
json_file.close()
model = model_from_json(loaded_model_json)
model.load_weights("model.h5")

model.compile(loss='cosine_proximity', optimizer='adam', metrics=['accuracy'])

rubik.init()
rubik.melange(50, False)

while rubik.score != 0:
    datas = [None] * (27)
    j = 0
    for z in range (3):
            for y in range (3):
                    for x in range (3):
                            datas[j] = rubik.distance(x, y ,z)
                            j = j + 1

    p = model.predict(numpy.array([datas]), batch_size=10, verbose=0)
    p = p[0]

    id = -1
    max = 0.0
    for i in range (18):
        if id == -1 or p[i] > max:
            id = i
            max = p[i]

    print (str(id)+" "+mvt[id])

    rubik.exec(mvt[id])

    print (mvt[id])
    print (rubik.score())

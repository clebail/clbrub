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
rubik.melange(20, False)

old = ""
while not rubik.win():
    datas = numpy.empty((54, 6, 1))
    map = rubik.map()
    i = 0
    j = 0
    for m in map:
        datas[i][j] = m
        j = (j + 1) % 6
        if j == 0:
            i = i + 1

    p = model.predict(numpy.array([datas]), batch_size=10, verbose=0)
    p = p[0]

    id = -1
    max = 0.0
    for i in range (18):
        if (id == -1 or p[i] > max) and mvt[i] != old:
            id = i
            max = p[i]

    rubik.exec(mvt[id])

    if len(mvt[id]) == 1:
        old = mvt[id] + "'"
    else:
        old = mvt[id][:-1]

    print (mvt[id]+" ---- "+old)

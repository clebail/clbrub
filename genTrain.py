import rubik
from random import randint

mvt = ["U", "U'", "D", "D'", "R", "R'", "L", "L'", "F", "F'", "B", "B'", "M", "M'", "E", "E'", "S", "S'"]

pos = [None] * 1000
nb = 0

while nb != 1000:
        lastMvt = rubik.melange(1, False)

        if len(lastMvt) == 1:
            lastMvt = lastMvt + "'"
        else:
            lastMvt = lastMvt[:-1]

        j = 0
        datas = [None] * (324 + 18)
        key = ''
        map = rubik.map()
        for m in map:
            datas[j] = m
            key = key + str(m)
            j = j + 1

        if key not in pos:
                for j in range (324, 342):
                        if mvt[j-324] == lastMvt :
                                datas[j] = 1
                        else:
                                datas[j] = 0

                print (datas)
                pos[nb] = key
                nb = nb + 1

                if nb % 10 == 0:
                    rubik.init()

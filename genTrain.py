import rubik
from random import randint

mvt = ["U", "U'", "D", "D'", "R", "R'", "L", "L'", "F", "F'", "B", "B'", "M", "M'", "E", "E'", "S", "S'"]

pos = [None] * 100000
nb = 0

while nb != 100000:
        score = 0
        rubik.init()
        while score == 0:
                rubik.melange(randint(1, 50), False)
                score = rubik.score()

        j = 0
        datas = [None] * (27 + 18)
        key = ''
        for z in range (3):
                for y in range (3):
                        for x in range (3):
                                datas[j] = rubik.distance(x, y ,z)
                                key = key + str(datas[j])
                                j = j + 1

        if key not in pos:
                output = rubik.lastmouvement()
                if output.endswith("'"):
                        output = output[:1]
                else:
                        output = output + "'"

                for j in range (27, 45):
                        if mvt[j-27] == output :
                                datas[j] = 1
                        else:
                                datas[j] = 0

                print (datas)
                pos[nb] = key
                nb = nb + 1

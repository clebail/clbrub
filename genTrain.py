import rubik
from random import randint

for i in range (50000):
	rubik.init()
	rubik.melange(randint(1, 9), False)

	j = 0
	datas = [None] * 28
	for z in range (3):
		for y in range (3):
			for x in range (3):
				datas[j] = rubik.distance(x, y ,z)
				j = j + 1
	output = rubik.lastmouvement()
	if output.endswith("'"):
		output = output[:1]
	else:
		output = output + "'"

	datas[27] = output

	print (datas)

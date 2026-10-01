import heapq
import numpy as np
import rubik
import rubikcore as rc

BUT = rc.solved()

def h(s):
    # Un quart de tour déplace au plus 20 autocollants : heuristique admissible
    return (int(np.count_nonzero(s != BUT)) + 19) // 20

def astar(depart):
    s0 = np.frombuffer(depart, dtype=np.uint8)
    tas = [(h(s0), 0, s0.tobytes(), [])]
    vus = {s0.tobytes(): 0}
    while tas:
        f, g, cle, chemin = heapq.heappop(tas)
        s = np.frombuffer(cle, dtype=np.uint8)
        if rc.is_solved(s):
            return chemin
        if g > vus[cle]:
            continue                      # entrée périmée : un chemin plus court a été trouvé
        for m, v in enumerate(rc.children(s)):
            k = v.tobytes()
            if g + 1 < vus.get(k, 99):
                vus[k] = g + 1
                heapq.heappush(tas, (g + 1 + h(v), g + 1, k, chemin + [m]))
    return None

rubik.init()
print(rubik.melange(5, True, False))
sol = astar(rubik.get_state())
print(sol and " ".join(rc.moves()[m] for m in sol))
rubik.exec(" ".join(rc.moves()[m] for m in sol))
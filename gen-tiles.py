#!/usr/bin/env python
from random import shuffle
tiles = 2*["RGBK"[i//13] + str(1 + i%13) for i in range(4*13)]
shuffle(tiles)
print '.'.join(tiles)

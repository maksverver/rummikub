#!/usr/bin/env python

import random
from glob import glob
import os.path
import sys

if len(sys.argv) < 3:
	print("Usage: " + sys.argv[0] + " <competition-dir> <matches-per-player> [<seed>]")
	sys.exit(1)

comp_dir   = sys.argv[1]
player_dir = os.path.join(comp_dir, 'players')
match_dir  = os.path.join(comp_dir, 'matches')
M          = int(sys.argv[2])

if len(sys.argv) >= 4: random.seed(int(sys.argv[3]))

# Grab list of players
players = map(os.path.basename, glob(os.path.join(player_dir, '*')))

# Build list of consecutive players
L = []
for i in range(M):
    chunk = list(players)
    random.shuffle(chunk)
    L += chunk

assert(len(L)%4 == 0)

# Shift players around so we don't have twice the same player in a single match
matches = []
for i in range(0, len(L), 4):
    for j in range(i, i + 4):
        k = j
        while L[k] in L[i:j]: k += 1
        L = L[:j] + L[k:k+1] + L[j:k] + L[k+1:]
    matches.append(L[i:i+4])

# Make dir for matches
if os.path.exists(match_dir):
	print(match_dir + ' exists!')
	sys.exit(1)
os.mkdir(match_dir)

# Write out match files
for i, match in enumerate(matches):
	print >>file(os.path.join(match_dir, 'm%04d'%(i+1)), 'wt'), '\t'.join(match)

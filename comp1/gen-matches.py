#!/usr/bin/env python

import os, glob, itertools

players = sorted(map(os.path.basename, glob.glob('players/*')))
for i, perm in enumerate(itertools.permutations(players)):
	f = file('matches/m%04d'%(i + 1), 'wt')
	print >>f, '\t'.join(perm[:4])

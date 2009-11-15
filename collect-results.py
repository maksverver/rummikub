#!/usr/bin/env python

import elementtree.ElementTree as ET
import sys, os.path, glob

# some hard-coded constants: (ugly)
url_prefix='http://hell.student.utwente.nl/rummikub/transcripts/comp1/'

def th(text, colspan = None):
    tag = '[th align=center bgcolor=#cf0'
    if colspan is not None: tag += ' colspan=' + str(colspan)
    return tag + ']' + str(text) + '[/]'

def b(text):
    return '[b]' + str(text) + '[/]'

def td(text, color='#fff', align='left'):
    tag = '[td'
    if align != 'left':   tag += ' align='+align
    if color is not None: tag += ' bgcolor='+color
    return tag + ']' + str(text) + '[/]'

def print_rondes_head():
    # print('[h2]Scores per ronde[/h2]')
    print('[table]')
    line = ''.join([ th('Speler '+str(i)) + th('(score)')
                     for i in [1,2,3,4] ])
    print('[tr]' + th('Ronde') + line + th('Details') + '[/]')

def print_ronde(i, path, scores):
    html = '[url=' + url_prefix + path.replace('.xml', '.html') + ']html[/]'
    xml  = '[url=' + url_prefix + path + ']xml[/]'
    colors = ['#eee', '#ddd', '#eee', '#ddd']
    line = ''.join([ td([naam, b(naam)][score == 0], color=col) +
                     td(score, color=col, align='right')
                     for col, (naam, score) in zip(colors, scores) ])
    print('[tr]' + td(i, align='center') + line + td(html + ', ' + xml) + '[/]')

def print_rondes_tail():
    print('[tr]' + th(' ', colspan=10) + '[/tr]')
    print('[/table]')

def print_totaal_head():
    print('[br]')
    # print('[h2]Totaalscores[/h2]')
    print('[table]')
    print('[tr]' + th('Plaats') + th('Deelnemer') + th('Score') + '[/]')

def print_totaal(i, name, score):
    print('[tr]' + td(i, align='center') + td(name) + td(score, align='right') + '[/]')

def print_totaal_tail():
    print('[/table]')

def get_scores(path):
    doc = ET.parse(path)
    names  = {}
    for player in doc.findall('setup/players/player'):
        names[int(player.attrib['id'])] = player.find('name').text
    scores = {}
    for score in doc.findall('scores/score'):
        scores[int(score.attrib['player'])] = int(score.text)
    return [ (names[id], scores[id]) for id in [1,2,3,4] ]

def collect(dir):
    total = {}

    print_rondes_head()
    for i, path in enumerate(glob.glob(os.path.join(dir, '*.xml'))):
        scores = get_scores(path)
        print_ronde(i + 1, os.path.basename(path), scores)
        for name, score in scores:
            if name not in total: total[name] = 0
            total[name] += score
    print_rondes_tail()

    print_totaal_head()
    scores = [ (total[name], name) for name in total ]
    for i, (score, name) in enumerate(sorted(scores)):
        print_totaal(i + 1, name, score)
    print_totaal_tail()


if __name__ == '__main__':
    if len(sys.argv) != 2 or not os.path.isdir(sys.argv[1]):
        print('usage: ' + sys.argv[0] + ' <results-dir>')
    else:
        collect(sys.argv[1])


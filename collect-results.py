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

def print_ronde(ronde, path, scores):
    html = '[url=' + url_prefix + path.replace('.xml', '.html') + ']html[/]'
    xml  = '[url=' + url_prefix + path + ']xml[/]'
    line = ''
    for i, (naam, score, failed) in enumerate(scores):
        col = ['#eee', '#ddd', '#eee', '#ddd'][i]
        if score == 0: naam = b(naam)
        if failed: naam = '[red]' + naam + '[/red]'
        line += td(naam, color=col) + td(score, color=col, align='right')
    print('[tr]' + td(ronde, align='center') + line + td(html + ', ' + xml) + '[/]')

def print_rondes_tail():
    print('[tr]' + th(' ', colspan=10) + '[/tr]')
    print('[/table]')

def print_totaal_head():
    print('[br]')
    # print('[h2]Totaalscores[/h2]')
    print('[table]')
    print('[tr]' + th('Plaats') + th('Deelnemer') + th('Cumulatieve score') + '[/]')

def print_totaal(i, name, score):
    print('[tr]' + td(i, align='center') + td(name) + td(score, align='right') + '[/]')

def print_totaal_tail():
    print('[/table]')

def get_scores(path):
    doc = ET.parse(path)
    names  = {}
    failed = {}
    for player in doc.findall('setup/players/player'):
        names[int(player.attrib['id'])] = player.find('name').text
    for turn in doc.findall('turns/turn'):
        if turn.find('error') is not None:
            failed[int(turn.attrib['player'])] = True
    scores = {}
    for score in doc.findall('scores/score'):
        scores[int(score.attrib['player'])] = int(score.text)
    return [ (names[id], scores[id], failed.get(id, False))
             for id in [1,2,3,4] ]

def collect(dir):
    total = {}

    print_rondes_head()
    files = glob.glob(os.path.join(dir, '*.xml'))
    files.sort()
    for i, path in enumerate(files):
        if i > 0 and i%80 == 0:
            print_rondes_tail()
            print_rondes_head()

        scores = get_scores(path)
        print_ronde(i + 1, os.path.basename(path), scores)
        min_score = 0
        # min_score = min([ score for _, score, _ in scores ])
        for name, score, _ in scores:
            if name not in total: total[name] = 0
            total[name] += score - min_score
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


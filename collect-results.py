#!/usr/bin/env python

import elementtree.ElementTree as ET
import sys, os.path, glob

# some hard-coded constants: (ugly)
url_prefix='http://hell.student.utwente.nl/rummikub/transcripts/comp2/'

class Score:
    def __init__(self, name, points, failures, delays):
        self.name      = name
        self.points    = points
        self.failures  = failures
        self.delays    = delays

def th(text, colspan = None, rowspan = None, width = None):
    tag = '[th align=center bgcolor=#cf0'
    if colspan is not None: tag += ' colspan=' + str(colspan)
    if rowspan is not None: tag += ' rowspan=' + str(rowspan)
    if width is not None: tag += ' width=' + str(width)
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
    for i, score in enumerate(scores):
        col = ['#eee', '#ddd', '#eee', '#ddd'][i]
        name = score.name
        if score.points == 0:   name = b(name)                      # winner!
        if score.failures:      name = '[red]' + name + '[/red]'    # failed!
        line += td(name, color=col) + td(score.points, color=col, align='right')
    print('[tr]' + td(ronde, align='center') + line + td(html + ', ' + xml) + '[/]')

def print_rondes_tail():
    print('[tr]' + th(' ', colspan=10) + '[/tr]')
    print('[/table]')

def print_totaal_head():
    print('[br]')
    print('[h2]Eindklassement[/h2]')
    print('[br]')
    print('[table]')
    print('[tr]' + th('Plaats', rowspan=2)
                 + th('Deelnemer', rowspan=2, width=120)
                 + th('Totaalscore', rowspan=2)
                 + th('Aantal gewonnen', rowspan=2)
                 + th('Aantal fouten', rowspan=2)
                 + th('Tijd', colspan=3, width=300) + '[/tr]')
    print('[tr]' + th('Totaal') + th('Gemiddeld') + th('Maximaal') + '[/tr]')

def print_totaal(i, name, points, delays, failures, won):
    print('[tr]' + td(i, align='center')
                 + td(name)
                 + td(sum(points), align='right')
                 + td(won,      align='right')
                 + td(failures, align='right')
                 + td('%0.3fs' % sum(delays), align='right')
                 + td('%0.3fs' % (sum(delays)/len(delays)), align='right')
                 + td('%0.3fs' % max(delays), align='right') + '[/]')

def print_totaal_tail():
    print('[tr]' + th(' ', colspan=8) + '[/tr]')
    print('[/table]')

def get_scores(path):
    doc = ET.parse(path)
    names     = {}
    failed    = {}
    delays    = {}
    for player in doc.findall('setup/players/player'):
        id = int(player.attrib['id'])
        names[id]   = player.find('name').text
        failed[id]  = 0
        delays[id]  = []
    for turn in doc.findall('turns/turn'):
        id = int(turn.attrib['player'])
        if turn.find('error') is not None:
            failed[id] += 1
        delays[id].append(float(turn.attrib['rpc-delay'].rstrip('s')))
    scores = {}
    for score in doc.findall('scores/score'):
        scores[int(score.attrib['player'])] = int(score.text)
    return [ Score(names[id], scores[id], failed[id], delays[id])
             for id in [1,2,3,4] ]

def collect(dir):
    total_points   = {}
    total_delays   = {}
    total_failures = {}
    total_won      = {}

    print_rondes_head()
    files = glob.glob(os.path.join(dir, '*.xml'))
    files.sort()
    for i, path in enumerate(files):
        if i > 0 and i%80 == 0:
            print_rondes_tail()
            print_rondes_head()

        scores = get_scores(path)
        print_ronde(i + 1, os.path.basename(path), scores)
        min_points = 0
        # min_points = min([ score.points for score in scores ])
        for score in scores:
            name = score.name
            if name not in total_points:
                total_points[name]   = []
                total_delays[name]   = []
                total_failures[name] = 0
                total_won[name]      = 0
            total_points[name].append(score.points - min_points)
            total_delays[name]   += score.delays
            total_failures[name] += score.failures
            total_won[name]      += (score.points == 0)
    print_rondes_tail()

    print_totaal_head()
    scores = [ (sum(total_points[name]), name) for name in total_points ]
    for i, (points, name) in enumerate(sorted(scores)):
        print_totaal(i + 1, name, total_points[name], total_delays[name],
                            total_failures[name], total_won[name])
    print_totaal_tail()


if __name__ == '__main__':
    if len(sys.argv) != 2 or not os.path.isdir(sys.argv[1]):
        print('usage: ' + sys.argv[0] + ' <results-dir>')
    else:
        collect(sys.argv[1])


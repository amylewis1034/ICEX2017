#! /usr/bin/python

import os
import subprocess
import re

ICEX = os.path.abspath('/Users/zwood/Desktop/ICEX2017/samy/build')

weights=[]

for i in range(1, 3):
    p = subprocess.Popen([os.path.join(ICEX, 'icex'), 'renderthirds.json'], stdout=subprocess.PIPE)
    filename = '../resources/paths/path_test%d.txt' %i
    os.rename(os.path.join(ICEX, '../resources/paths/path_test.txt'), os.path.join(ICEX, filename))

    for line in p.communicate()[0].decode('ascii').split('\n'):
        weight = re.search(r'percent in high weight list: .*', line)
       
        if weight:
            weights.append((filename, weight.group(0).split(' ')[-1]))

weights.sort(key=lambda x: -(float(x[1])))
print weights
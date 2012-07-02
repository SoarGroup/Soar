#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, re
import pylab
from matplotlib import rc
from pylab import arange,pi,sin,cos,sqrt

if os.name is 'posix':
  golden_mean = (sqrt(5)-1.0)/2.0     # Aesthetic ratio
  fig_width = 3.375                   # width in inches
  fig_height = fig_width*golden_mean  # height in inches
  fig_size =  [fig_width,fig_height]
  params = {'backend': 'ps',
            'axes.labelsize': 8,
            'text.fontsize': 8,
            'legend.fontsize': 6,
            'xtick.labelsize': 6,
            'ytick.labelsize': 6,
            'text.usetex': True,
            'ps.usedistiller': 'xpdf',
            'figure.figsize': fig_size}
  pylab.rcParams.update(params)
  rc('font',**{'family':'serif','serif':['Times']})

import sys, getopt, random, time, datetime
import numpy as np
import matplotlib
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
from optparse import OptionParser
from matplotlib.ticker import ScalarFormatter
#from matplotlib2tikz import save

print 'matplotlib.__version__ is ' + matplotlib.__version__

class CommaFormatter(ScalarFormatter):
  def pprint_val(self, x):
    px = ScalarFormatter.pprint_val(self, x)
    if os.name is 'posix':
      px = px[1:len(px)-1]
    px = self.add_commas(px)
    if os.name is 'posix' and len(px) is not 0:
      px = "$" + px + "$"
    return px
  
  def add_commas(self, arg):
    s = arg.split('.')
    if len(s) is 2 and s[1][0] is not '0':
      return ""
    if s[0][0] is '-':
      c = '-' + self.recurse(s[0][1:])
    else:
      c = self.recurse(s[0])
    return c
  
  def recurse(self, arg):
    if len(arg) < 4:
      return arg
    s = len(arg) - 3
    return self.recurse(arg[:s]) + ',' + arg[s:]

def main():
  if len(sys.argv) == 1:
    f = open('cartpole-rl.soar', 'r')
    reg = {}
    reg['x'] = re.compile('\^index-x ([-0123456789]+)')
    reg['x-dot'] = re.compile('\^index-x-dot ([-0123456789]+)')
    reg['theta'] = re.compile('\^index-theta ([-0123456789]+)')
    reg['theta-dot'] = re.compile('\^index-theta-dot ([-0123456789]+)')
    rang = {}
    for r in reg:
      rang[r] = [0,0]
    #rset = {}
    recent = {}
    idx = 4
    count = 0
    while True:
      line = f.readline()
      if not line:
        break
      for r in reg:
        ex = reg[r].search(line)
        if not ex == None:
          val = int(ex.group(1))
          if val < rang[r][0]:
            rang[r][0] = val
          elif val > rang[r][1]:
            rang[r][1] = val
          recent[r] = val
          idx = (idx + 1) % 4
      if idx == 0:
        #rset[tuple([recent['x'], recent['x-dot'], recent['theta'], recent['theta-dot']])] = 1
        idx = 4
        count += 1
    f.close()
    print rang
    print str(count) + " / " + str(2 * (rang['x'][1] - rang['x'][0] + 1) * (rang['x-dot'][1] - rang['x-dot'][0] + 1) * (rang['theta'][1] - rang['theta'][0] + 1) * (rang['theta-dot'][1] - rang['theta-dot'][0] + 1)) + " Q-values"
    
    f = open('cartpole.out', 'r')
    seed = int(f.readline().split(' ', 1)[1])
    smith = []
    while True:
      line = f.readline()
      if not line or line == '':
        break
      else:
        smith.append(int(line.split(' ', 4)[3]))
    f.close()
  else:
    files = []
    for filename in sys.argv[1:]:
      f = open(filename, 'r')
      seed = int(f.readline().split(' ', 1)[1])
      files.append(f)
    
    smith = {}
    smith['avg'] = []
    smith['min'] = []
    smith['max'] = []
    smith['med'] = []
    done = False
    while not done:
      vals = []
      for f in files:
        line = f.readline()
        if not line or line == '':
          done = True
          break
        else:
          vals.append(int(line.split(' ', 4)[3]))
      if not done:
        vals = sorted(vals)
        smith['avg'].append(sum(vals) / len(vals))
        smith['min'].append(vals[0])
        smith['max'].append(vals[len(vals) - 1])
        if len(vals) % 2 == 1:
          smith['med'].append(vals[int(len(vals) / 2)])
        else:
          smith['med'].append((vals[int(len(vals) / 2)] + vals[int(len(vals) / 2 + 1)]) / 2)
    
    for f in files:
      f.close()
  
  fig = plt.figure()
  fig.canvas.set_window_title('Cart Pole')
  
  pylab.axes([0.125,0.15,0.8375,0.75])
  
  if len(sys.argv) == 1:
    x = []
    i = 0
    for s in smith:
      i += 1
      x.append(i)
    
    pylab.plot(x, smith, label="Values", color='blue', linestyle='solid')
  else:
    x = []
    i = 0
    for s in smith['avg']:
      i += 1
      x.append(i)
    
    pylab.plot(x, smith['avg'], label="Average", color='brown', linestyle='solid')
    pylab.plot(x, smith['max'], label="Maximum", color='blue', linestyle='solid')
    pylab.plot(x, smith['med'], label="Median", color='blue', linestyle='solid')
    pylab.plot(x, smith['min'], label="Minimum", color='blue', linestyle='solid')
  
  pylab.legend(loc=1, handlelength=4.2, numpoints=2)
  
  pylab.grid(True)
  
  pylab.xlabel('Episode Number', fontsize=8)
  pylab.ylabel('Number of Steps', fontsize=8)
  pylab.title('Blocks World with Tie Impasses for RL-Rules', fontsize=10)
  
  fig.axes[0].xaxis.set_major_formatter(CommaFormatter())
  fig.axes[0].yaxis.set_major_formatter(CommaFormatter())
  
  xlabels = fig.axes[0].xaxis.get_ticklabels()
  last_xlabel = xlabels[len(xlabels) - 1]
  last_xlabel.set_horizontalalignment('right')
  last_xlabel.set_x(0)
  #fig.axes[0].yaxis.set_scale('log')
  #print last_xlabel.get_size()
  #print last_xlabel.get_position()
  #print last_xlabel.get_text()
  #print last_xlabel
  
  pylab.savefig('cartpole.eps')
  plt.show()

if __name__ == "__main__":
  main()

#!/usr/bin/env python
# -*- coding: utf-8 -*-

import hashlib, os, re
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

pylab.rcParams['path.simplify'] = True

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

class Handle:
  def __init__(self, f, filename, seed):
    self.f = f
    self.filename = filename
    self.seed = seed

class Handles:
  def __init__(self):
    self.handles = []
    self.smith = {}

def main():
  if len(sys.argv) == 1:
    f = open('puddleworld-rl.soar', 'r')
    reg = {}
    reg['x'] = re.compile('\^index-x ([-0123456789]+)')
    reg['y'] = re.compile('\^index-y ([-0123456789]+)')
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
    print str(count) + " / " + str(2 * (rang['x'][1] - rang['x'][0] + 1) * (rang['y'][1] - rang['y'][0] + 1)) + " Q-values"
    
    f = open('puddleworld.out', 'r')
    seed = int(f.readline().split(' ', 1)[1])
    smith = []
    while True:
      line = f.readline()
      if not line or line == '':
        break
      else:
        smith.append(float(line.split(' ', 6)[5]))
    f.close()
    
    directory=''
    title='Puddle World (seed ' + str(seed) + ')'
  else:
    files = {}
    for filename in sys.argv[1:]:
      f = open(filename, 'r')
      seed = int(f.readline().split(' ', 1)[1])
      
      directory=re.search('(^.*[^/]+)/+[^/]*$', filename).group(1) #filename.rsplit('/', 1)[0]
      try:
        files[directory].handles.append(Handle(f, filename, seed))
      except KeyError:
        files[directory] = Handles()
        files[directory].handles.append(Handle(f, filename, seed))
    
    for group in files:
      files[group].smith['avg'] = []
      files[group].smith['min'] = []
      files[group].smith['max'] = []
      files[group].smith['med'] = []
      done = False
      while not done:
        vals = []
        for handle in files[group].handles:
          line = handle.f.readline()
          if not line or line == '':
            done = True
            break
          else:
            vals.append(float(line.split(' ', 6)[5]))
        if not done:
          vals = sorted(vals)
          files[group].smith['avg'].append(sum(vals) / len(vals))
          files[group].smith['min'].append(vals[0])
          files[group].smith['max'].append(vals[len(vals) - 1])
          if len(vals) % 2 == 1:
            files[group].smith['med'].append(vals[int(len(vals) / 2)])
          else:
            files[group].smith['med'].append((vals[len(vals) / 2 - 1] + vals[len(vals) / 2]) / 2)
      
      for handle in files[group].handles:
        handle.f.close()
    
    if len(files) == 1:
      title='Puddle World (' + group.rsplit('/',1)[1].replace('_', '\_') + ')'
      smith = files[group].smith
      mode = 'single experiment evaluation'
    else:
      title='Puddle World (' + group.rsplit('/',1)[0].replace('_', '\_') + ')'
      
      smith = {}
      for group in files:
        smith[group.rsplit('/',1)[1].replace('_', '\_')] = files[group].smith['avg']
      
      mode = 'multiple experiment evaluation'
  
  fig = plt.figure()
  fig.canvas.set_window_title('Puddle World')
  
  pylab.axes([0.125,0.15,0.8375,0.75])
  
  if len(sys.argv) == 1:
    x = []
    i = 0
    for s in smith:
      i += 1
      x.append(i)
    
    for i in range(1, len(smith)):
      smith[i] = 0.95 * smith[i - 1] + 0.05 * smith[i];
    
    pylab.plot(x, smith, label="Values", color='blue', linestyle='solid')
  else:
    x = []
    i = 0
    r = 0
    for agent in smith:
      r = len(smith[agent])
    for s in range(0,r):
      i += 1
      x.append(i)
    
    for a in smith:
      for i in range(1, len(smith[a])):
        smith[a][i] = 0.95 * smith[a][i - 1] + 0.05 * smith[a][i];
    
    if mode == 'single experiment evaluation':
      pylab.plot(x, smith['max'], label="Maximum", color='green', linestyle='solid')
      #pylab.plot(x, smith['med'], label="Median", color='brown', linestyle='solid')
      pylab.plot(x, smith['min'], label="Minimum", color='teal', linestyle='solid')
      pylab.plot(x, smith['avg'], label="Average", color='blue', linestyle='solid')
    else:
      for agent in smith:
        pylab.plot(x, smith[agent], label=agent, linestyle='solid')
  
  pylab.legend(loc=4, handlelength=4.2, numpoints=2)
  
  pylab.grid(True)
  
  pylab.xlabel('Episode Number', fontsize=8)
  pylab.ylabel('Number of Steps', fontsize=8)
  pylab.title(title, fontsize=10)
  pylab.ylim(ymin=-500, ymax=0)
  
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
  
  if len(sys.argv) == 1:
    pylab.savefig('puddleworld.eps')
    pylab.savefig('puddleworld.png')
    plt.show()
  else:
    splitd = directory.rsplit('/', 1)
    
    if mode == 'single experiment evaluation':
      filename = splitd[1]
    else:
      m = hashlib.md5()
      for agent in smith:
        m.update(agent)
      filename = str(m.hexdigest())
    
    if not os.path.exists(splitd[0] + '/eps'):
      os.makedirs(splitd[0] + '/eps')
    pylab.savefig(splitd[0] + '/eps/' + filename + '.eps')
    
    if not os.path.exists(splitd[0] + '/png'):
      os.makedirs(splitd[0] + '/png')
    pylab.savefig(splitd[0] + '/png/' + filename + '.png', dpi=1200)

if __name__ == "__main__":
  main()

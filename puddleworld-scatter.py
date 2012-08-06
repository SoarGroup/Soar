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
  if len(sys.argv) == 2:
    f = open(sys.argv[1], 'r')
    reg = {}
    #reg['name'] = re.compile('^([^ \t\r\n]+)[ \t\r\n]')
    reg['updates'] = re.compile('updates ([0123456789]+)')
    reg['value'] = re.compile('value ([-0123456789.]+)')
    reg['variance'] = re.compile('variance ([-0123456789.]+)')
    reg['influence'] = re.compile('influence ([-0123456789.]+)')
    smith = {}
    for r in reg:
      smith[r] = []
    smith['varinf'] = []
    while True:
      line = f.readline()
      if not line or line == "\n":
        break
    while True:
      line = f.readline()
      if not line:
        break
      for r in reg:
        ex = reg[r].search(line)
        smith[r].append(float(ex.group(1)))
        if r == 'variance' and float(ex.group(1)) < 0.0:
          print line
          exit(1)
      smith['varinf'].append(float(reg['variance'].search(line).group(1)) * float(reg['influence'].search(line).group(1)))
    f.close()
    
    directory=''
    title='Puddle World'
  
  fig = plt.figure()
  fig.canvas.set_window_title('Puddle World')
  
  pylab.axes([0.125,0.15,0.8375,0.75])
  
  #pylab.scatter(smith['updates'], smith['value'], label="Value")
  pylab.scatter(smith['updates'], smith['variance'], label="Variance")
  #pylab.scatter(smith['updates'], smith['influence'], label="Influence")
  #pylab.scatter(smith['updates'], smith['varinf'], label="Variance * Influence")
  
  pylab.legend(loc=4, handlelength=4.2, numpoints=2)
  
  pylab.grid(True)
  
  pylab.xlabel('Updates', fontsize=8)
  pylab.ylabel('Number', fontsize=8)
  pylab.title(title, fontsize=10)
  
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
  
  if len(sys.argv) == 2:
    pylab.savefig('puddleworld-scatter.eps')
    pylab.savefig('puddleworld-scatter.png')
    plt.show()

if __name__ == "__main__":
  main()

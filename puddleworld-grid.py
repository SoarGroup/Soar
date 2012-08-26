#!/usr/bin/env python
# -*- coding: utf-8 -*-

import hashlib, os, re
import pylab
from matplotlib import rc
from pylab import arange,pi,sin,cos,sqrt

import matplotlib
from matplotlib.patches import CirclePolygon
from matplotlib.collections import PolyCollection
import pylab 

if os.name is 'posix':
  #golden_mean = (sqrt(5)-1.0)/2.0     # Aesthetic ratio
  fig_width = 3.375                   # width in inches
  fig_height = fig_width#*golden_mean  # height in inches
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

class Tree:
  def __init__(self):
    self.type = None
    self.first = None
    self.second = None
    self.value = 0
  
  def split(self, type):
    if self.type == None:
      self.type = type
      self.first = Tree()
      self.second = Tree()
    elif self.type != type:
      raise Exception("Tree cannot be resplit with different type.")
  
  def insert(self, x, y, type='x'):
    if len(x) == 0 and len(y) == 0:
      return self
    elif type == 'x':
      #print '<' + x + ',' + y + '>:x ' + str(self)
      self.split('x')
      if x[0] == '0':
        return self.first.insert(x[1:], y, 'y')
      elif x[0] == '1':
        return self.second.insert(x[1:], y, 'y')
      else:
        raise Exception("Bad value in x")
      #print '<' + x + ',' + y + '>:x ' + str(self)
    elif type == 'y':
      #print '<' + x + ',' + y + '>:y ' + str(self)
      self.split('y')
      if y[0] == '0':
        return self.first.insert(x, y[1:], 'x')
      elif y[0] == '1':
        return self.second.insert(x, y[1:], 'x')
      else:
        raise Exception("Bad value in y")
      #print '<' + x + ',' + y + '>:y ' + str(self)
    else:
      raise Exception("Bad type given to Tree::insert")
  
  def to_str(self):
    if self.first == None or self.second == None:
      return '[]'
    else:
      return '[' + self.type + ':' + self.first.to_str() + ',' + self.second.to_str() + ']'
  
  def plot(self, axes, x, y, maxdepth=-1):
    #print str(x) + ',' + str(y)
    if self.type == None or maxdepth == 0:
      return
    elif self.type == 'x':
      midpt = (x[0] + x[1]) / 2.0
      axes.add_line(pylab.Line2D([midpt,midpt], y, color='black'))
      self.first.plot(axes, (x[0], midpt), y, maxdepth - 1)
      self.second.plot(axes, (midpt, x[1]), y, maxdepth - 1)
    elif self.type == 'y':
      midpt = (y[0] + y[1]) / 2.0
      axes.add_line(pylab.Line2D(x, [midpt,midpt], color='black'))
      self.first.plot(axes, x, (y[0], midpt), maxdepth - 1)
      self.second.plot(axes, x, (midpt, y[1]), maxdepth - 1)
    else:
      raise Exception("Bad type in Tree::plot")

def main():
  t = Tree()
  tn = Tree()
  ts = Tree()
  te = Tree()
  tw = Tree()
  #print t.to_str()

  f = open('puddleworld-rl.soar', 'r')
  regd = re.compile('\^direction ([^ ]+)')
  dir = ''
  reg = re.compile('\^id \|(.+)\|')
  regx = re.compile('in-bounds,x([01]+)\)')
  regy = re.compile('in-bounds,y([01]+)\)')
  while True:
    line = f.readline()
    if not line:
      break
    d = regd.search(line)
    if not d == None:
      dir = d.group(1)
    ex = reg.search(line)
    if not ex == None:
      id = ex.group(1)
      
      idx = regx.search(id)
      if idx != None:
        idx = idx.group(1)
      else:
        idx = ''
      
      idy = regy.search(id)
      if idy != None:
        idy = idy.group(1)
      else:
        idy = ''
      
      #print "x" + idx + ", y" + idy
      t.insert(idx, idy)
      if dir == 'north':
        tn.insert(idx, idy)
      elif dir == 'south':
        ts.insert(idx, idy)
      elif dir == 'east':
        te.insert(idx, idy)
      elif dir == 'west':
        tw.insert(idx, idy)
  f.close()

  #print t.to_str()
  
  #return
  
  directory=''
  title='Generated Value Function for Puddle World'
  
  
  if len(sys.argv) == 2:
    if sys.argv[1] == 'north':
      t = tn
      title += ' (North)'
    elif sys.argv[1] == 'south':
      t = ts
      title += ' (South)'
    elif sys.argv[1] == 'east':
      t = te
      title += ' (East)'
    elif sys.argv[1] == 'west':
      t = tw
      title += ' (West)'
  
  
  fig = plt.figure()
  fig.canvas.set_window_title('Puddle World')
  
  pylab.axes([0.125,0.15,0.8375,0.75])
  
    
  t.plot(fig.axes[0], (0, 1), (1, 0))
  
  
  #pylab.legend(loc=4, handlelength=4.2, numpoints=2)
  
  pylab.grid(False)
  
  pylab.xlabel('X', fontsize=8)
  pylab.ylabel('Y', fontsize=8)
  pylab.title(title, fontsize=10)
  pylab.xlim(xmin=0, xmax=1)
  pylab.ylim(ymin=0, ymax=1)
  
  #fig.axes[0].xaxis.set_major_formatter(CommaFormatter())
  #fig.axes[0].yaxis.set_major_formatter(CommaFormatter())
  fig.axes[0].set_xticks([0,1])
  fig.axes[0].set_yticks([0,1])
  
  #xlabels = fig.axes[0].xaxis.get_ticklabels()
  #last_xlabel = xlabels[len(xlabels) - 1]
  #last_xlabel.set_horizontalalignment('right')
  #last_xlabel.set_x(0)
  ##fig.axes[0].yaxis.set_scale('log')
  ##print last_xlabel.get_size()
  ##print last_xlabel.get_position()
  ##print last_xlabel.get_text()
  ##print last_xlabel
  
  
  if len(sys.argv) == 2:
    pylab.savefig('puddle-world-' + sys.argv[1] + '.eps')
    pylab.savefig('puddle-world-' + sys.argv[1] + '.png', dpi=1200)
  else:
    pylab.savefig('puddle-world.eps')
    pylab.savefig('puddle-world.png', dpi=1200)
    plt.show()

if __name__ == "__main__":
  main()

#!/usr/bin/env python

"""Provides BarChart class which creates HTML 1D bar charts,
and StackedBarChart class to deal with multiple data plotting
for web pages. Only works for positive data values.
Also provides a DataList class to support the data handling needed.
"""
__version__ = '$Id$'
__author__ = 'Robin Friedrich'
__date__ = 'June 30, 1997'
# barchart.py
# COPYRIGHT (C) 1997  ROBIN FRIEDRICH  email:Robin.Friedrich@pdq.net
# Permission to  use, copy, modify, and distribute this software and its
# documentation  for  any  purpose  and  without fee  is hereby granted,
# provided that the above copyright notice appear in all copies and that
# both that copyright notice and this permission notice appear in
# supporting documentation.
# THE  AUTHOR  DISCLAIMS  ALL  WARRANTIES WITH  REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
# EVENT  SHALL THE  AUTHOR  BE  LIABLE  FOR  ANY  SPECIAL,   INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
# USE, DATA OR PROFITS,  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

import string
import UserList
from types import IntType, FloatType
from HTMLgen import SimpleDocument, Image, Font, TD, TR, TableLite, Caption, Bold, Pre
from HTMLcolors import *

# set these paths to valid URLs corresponding to where you placed the gifs
barfiles = {  'blue' : '../image/bar-blue.gif',
               'red' : '../image/bar-red.gif',
             'green' : '../image/bar-green.gif',
            'orange' : '../image/bar-orange.gif',
            'yellow' : '../image/bar-yellow.gif',
            'purple' : '../image/bar-purple.gif'  }


class BarChart:
    """Class which takes a DataList object and creates
    the HTML to represent a color coded bar chart.

    Values to be charted are limited to non-negative values.
    """
    title = ''
    zero = 0
    label_shade = WHITE
    value_shade = CADETBLUE
    bar_shade = GRAY2
    max_width = 400
    
    def __init__(self, datalist=None):
        "datalist is an instance of class DataList"
        if datalist is None:
            self.datalist = DataList()
        else:
            self.datalist = datalist
        self.initialize()

    def initialize(self):
        """Initialize the chart properties.

        This method is overloaded by child classes.
        """
        self.table = TableLite(cellpadding=3, cellspacing=0)
        self.barfiles = barfiles
        # color keys for low, norm and hi indications
        self.barcolors = ('yellow', 'blue', 'red')
        # numeric values which determine the boundaries between color ranges
        self.thresholds = (0, 1E+09)
        # 
        self.bound_zero = "no"

    def normalize(self):
        """Scale data to values between 0 and 400.

        Determine peak value and set scale accordingly.  If the values
        are clustered within 30% of each other, will shift the zero
        point for the barchart up to exagerate the value differences.
        To inhibit this, set the .bound_zero attribute to "yes".
        """
        
        self.datalist.sum_totals()
        self.average = self.datalist.mean('total')
        
        # Compute max and min
        low, hi = self.datalist.min('total'), self.datalist.max('total')
        # if data is clustered together rescale by shifting zero point
        if hi < (1.3 * low) and self.bound_zero != "yes" and self.zero == 0:
            self.zero = low * 0.8
            s = '%9.2e' % self.zero  # hack to round to a natural value
            x = string.atoi(s[-3:])
            self.zero = round(self.zero, -x)
        self.scale = float(hi - self.zero) / self.max_width
        
    def __str__(self):
        """Generate HTML for the entire table and caption.
        """
        self.normalize() #calculate the scaling of the data
        self.table.prepend(Caption(Bold(self.title))) #add title as caption
        for item in self.datalist:
            row = TR()
            # 1st cell is the text description
            row.append(TD(item['label'], align='left', width=70,
                          bgcolor = self.label_shade))
            # 2nd cell is the decimal sum value
            row.append(TD("%9.1f"%item['total'], align='right',
                          width=70, bgcolor=self.value_shade))
            # 3rd cell contains the scaled bar graphic
            row.append(self.make_bar_cell(item))
            self.table.append(row) # add the row to the table
        # now tack on a legend at the bottom
        self.table.append(TR( TD('AVERAGE', align='left', width=70,
                                 bgcolor = self.label_shade),
                              TD("%9.1f" % self.average, align='right',
                                 width=70, bgcolor=self.value_shade), 
                              TD(self.gen_legend(),
                                 bgcolor=self.label_shade,
                                 html_escape="OFF")))
        return str(self.table)

    def make_bar_cell(self, dict):
        """return a TD object containing the scaled bar
        """
        cell = TD(bgcolor=self.bar_shade, html_escape="OFF")
        cell.append(self.gen_bar(dict['value'],
                                 self.color_code(dict['value'])))
        return cell
    
    def color_code(self, value):
        """return a color string based on whether the given datum
        falls beyond thresholds. Works off self.thresholds=(low,hi).
        """
        low, hi = self.thresholds
        if value > hi:
            return self.barcolors[2] #'red'
        elif value < low:
            return self.barcolors[0] #'yellow'
        else:
            return self.barcolors[1] #'blue'
        
    def pixels(self, magnitude):
        """returns the integer number of pixels to represent a given magnitude.
        """
        return int( (magnitude - self.zero) / self.scale )
    
    def gen_bar(self, value, color='blue'):
        """return Image object for the scaled bar graphic
        """
        if value <= 0:
            return ""
        else:
            bar = Image(self.barfiles[color], 
                    width=self.pixels(value), height=13, alt=value)
        return bar

    def gen_legend(self):
        """Return an HTML string which displays the legend for the chart.
        """
        sample = Image(self.barfiles['blue'], height=13, width=40)
        return '<b>^%7.1f</b> lower bound<br> SCALE: %s = %7.1f units' % \
          (self.zero, sample, self.scale * 40)


class StackedBarChart(BarChart):
    """Represent up to six data values in a stacked barchart.
    """
    def initialize(self):
        """Define StackedBarChart specific attributes.
        """
        self.table = TableLite(cellpadding=3, cellspacing=0)
        self.barfiles = barfiles
        self.colors = ('blue','red','yellow','purple','orange','green')
        self.bound_zero = "yes"
        
    def make_bar_cell(self, dict):
        """return a TD object containing the scaled bar
        """
        cell = TD(bgcolor=self.bar_shade, html_escape="OFF")
        values = []
        for name in self.datalist.segment_names:
            values.append(dict[name])
        cell.append(self.gen_bar(values, self.colors))
        return cell
            
    def gen_bar(self, values, colors):
        """return HTML string for the stacked bar graphic.
        """
        bar = ''
        if len(values) > len(colors): raise ValueError
        for i in range(len(values)):
            bar = bar + self.segment(values[i], colors[i])
        return bar
    
    def segment(self, value, color='blue'):
        """return HTML string for a bar segment given a magnitude and color.
        """
        if value <= 0:
            return ""
        else:
            return str(Image(self.barfiles[color], 
                         width=self.pixels(value),
                         height=13, alt=value) )

    def gen_legend(self):
        """Return an HTML string which displays the legend for the chart.
        """
        s = []
        for i in range(len(self.datalist.segment_names)):
            s.append(str(Image(self.barfiles[self.colors[i]],
                           height=13, width=30)))
            s.append(str(Font(self.datalist.segment_names[i], size= -1)))
            s.append("&nbsp;&nbsp;")
        return string.join(s, ' ')


class DataList(UserList.UserList):
    """Class supporting tabular storage and access of data.

    Used by BarChart classes. Takes a list of sequences and loads
    them into a list of dictionaries using the first item from the
    sequence as a label key. The column names are from the
    *segment_names* attribute and *must be set* prior to loading
    multidimensional data. 

    Supports some simple data processing methods.
    """
    segment_names = ('value',) # set to tuple of column names

    def __init__(self, list = None):
        self.data = []
        if list is not None:
            self.load_tuples(list)
            
    def load_tuple(self, t):
        """Load individual record of data into new dictionary.

        Use first item in given sequence as label key and assigns
        remaining items with keys from *segment_names* in order.
        """
        d = {'label': t[0]}
        for i in range(len(self.segment_names)):
            if type(t[i+1]) in (IntType, FloatType):
                d[self.segment_names[i]] = t[i+1]
            else:
                d[self.segment_names[i]] = 0
                print '%s value %s invalid; was set to zero' % (t[0], t[i+1])
        self.append(d)

    def load_tuples(self, tt):
        """Load each item from the given sequence of sequences.
        """
        for t in tt:
            self.load_tuple(t)

    def __str__(self):
        """Return tabular string representation of internal data.
        """
        s = []
        ncols = len(self.segment_names)
        heading = '\n     Label' + '%11s'*ncols
        s.append(heading % self.segment_names)
        body = ['%(label)10s']
        for name in self.segment_names:
            body.append('%(' + '%s)10g' % name)
        body = string.join(body)
        for datum in self.data:
            s.append(body % datum)
        s.append('\n')
        return string.join(s, '\n')

    def add_column(self, name, pairs):
        """Take list of (label,value) pairs and add the data as a
        new column named *name*.
        """
        self.segment_names = self.segment_names + (name,)
        for label, value in pairs:
            try:
                self.data[self.index(label)][name] = value
            except TypeError:
                print '%s not found' % label

    def index(self, label):
        """return dictionary corresponding to *label* string.
        """
        try:
            return self._labelcache[label]
        except AttributeError:
            self.cache_labels()
            return self.index(label)
        except KeyError:
            return None

    def cache_labels(self):
        """Create cache of indexes corresponding to the labels
        to speed calles to index method.
        """
        if not hasattr(self, '_labelcache'):
            self._labelcache = {}
        for i in range(len(self.data)):
            self._labelcache[self.data[i]['label']] = i
         
    def max(self, key='value'):
        """return maximum value in column key
        """
        hi = 0
        for datum in self.data:
            if datum[key] > hi: hi = datum[key]
        return hi

    def min(self, key='value'):
        """return minimum value in column key
        """
        low = self.data[0][key]
        for datum in self.data:
            if datum[key] < low: low = datum[key]
        return low

    def sum(self, key='value'):
        """return sum of values in column key
        """
        sum = 0
        for datum in self.data:
            sum = sum + datum[key]
        return sum

    def mean(self, key='value'):
        """return mean (average) of values in column key
        """
        if len(self) > 0:
            return self.sum(key) / len(self)
        else:
            return 0.0
    
    def sum_totals(self):
        """add new key 'total' to each dictionary
        """
        for datum in self.data:
            sum = 0
            for name in self.segment_names:
                sum = sum + datum[name]
            datum['total'] = sum
    
    def sort(self, key='label', direction='increasing'):
        """Sort list according to key in direction.

        Example: DLobject.sort('height', 'decreasing')
        """
        self._sortkey = key
        if direction[0] == 'i':
            self.data.sort(self.increasing)
        else:
            self.data.sort(self.decreasing)
        # index cache is now invalid and will be regenerated 
        self.cache_labels()
        
    def increasing(self, a, b):
        return cmp(a[self._sortkey], b[self._sortkey])

    def decreasing(self, a, b):
        return cmp(b[self._sortkey], a[self._sortkey])



def simple_test():
    from time import time
    print "running barchart test routine" 
    dum = [ ('fddo4', 1318), ('cn1', 1472), ('cn2', 1411),
            ('fddo3', 1280), ('asc8', 1371), ('orb3', 1390),
            ('fddo1', 1418), ('asc4', 1292), ('dn2', 1381),
            ('fddo2', 1341), ('asc1', 1352), ('dn1', 1441)
            ]
    t0 = time()
    doc = SimpleDocument(title='Bar Chart', bgcolor=GREY1)
    dummydata = DataList()
    dummydata.load_tuples(dum)
    dummydata.sort()
    b = BarChart(dummydata)
    b.thresholds = (1300, 1400)
    b.title = "System Throughput (jobs/week)"
    doc.append(b)
    doc.append(Pre(str(dummydata)))
    doc.write('./html/bar.html')
    print "took", time() - t0, 'sec.'

def stacked_test():
    from time import time
    print "running stacked barchart test routine" 
    dum = [ ('fddo4', 1318, 456, 235, 290),
            ('fddo3', 1280, 560, 129, 295), 
            ('fddo1', 1418, 1201, 490, 125),
            ('fddo2', 1341, 810, 466, 203)
            ]
    t0 = time()
    doc = SimpleDocument(title='Stacked Bar Chart', bgcolor=GREY1)
    dummydata = DataList()
    dummydata.segment_names = ('User','System','I/O','Wait')
    dummydata.load_tuples(dum)
    dummydata.sort()
    b = StackedBarChart(dummydata)
    b.title = "System Load"
    doc.append(b)
    doc.append(Pre(str(dummydata)))
    doc.write('./html/stackedbar.html')
    print "took", time() - t0, 'sec.'

def test():
    import profile, pstats
    #profile.run("simple_test()")
    profile.run('stacked_test()', 'barchart.prof')
    p = pstats.Stats('barchart.prof')
    p.sort_stats('cumulative').print_stats(20)
    #p.sort_stats('time').print_stats(20)
    #simple_test()
    #stacked_test()


if __name__ == '__main__': test()
    

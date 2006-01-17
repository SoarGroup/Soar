#!/usr/bin/env python
# COPYRIGHT (C) 1997  ROBIN FRIEDRICH
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
"""Generate HTML calendars

This module reads a text file containing appointment information and
generates web pages containing those scheduled appointments in a
nicely formatted linear table for each month."""

import string, time
from HTMLcolors import *
from HTMLgen import SimpleDocument, TableLite, TD, TR, Font, Name, H, Center, Href
from types import IntType
from calendar import day_name, month_name, mdays, weekday

__version__ = '$Id$'
__author__ = 'Robin Friedrich'
__date__ = '10/13/97'

# Constants
day_month_order = 0 # set to 1 to parse day/month ordering (europe)
                    # set to 0 to parse month/day ordering (usa)
DateError = 'date error'
PAGECOLOR    = PEACH
WEEKDAYCOLOR = GRAY1
WEEKENDCOLOR = GREEN3
TIMECOLOR    = BLUE
SOURCETEXTFILE = 'appt.txt'
HTMLSUFFIX = 'html' # use htm if you must

def main():
    """Main routine to drive script and serve as usage example.
    """
    parsed_appts = read_appt_file(SOURCETEXTFILE)
    current_year = Year()
    current_year.load_appointments(parsed_appts)
    current_year.write('./', 'Calendar')
    
class Appointment:
    """Represent an appointment entry.
    """
    def __init__(self, hr='9', min='00', text='Nothing'):
        self.hr = hr
        self.min = min
        self.text = text
    def __str__(self):
        return "%s %s" % (Font(self.hr +':'+ self.min,
                               size = -1, color = TIMECOLOR),
                          Font(self.text, size = -1))

class Day:
    """Represent a day record.

    Appointment instances are appended to instances of this class.
    """
    def __init__(self, year=1997, month=1, day=1):
        self.weekday = day_name[weekday(year,month,day)]
        self.year = `year`
        self.month = `month`
        self.day = `day`
        self.appointments = []
        
    def append(self, appt):
        self.appointments.append(appt)
        
    def repr_date_cell(self):
        return '%s %s' % (self.weekday,
                     Name(self.month +'-'+self.day, self.day))
        
    def repr_appt_cell(self):
        strlist = map(str, self.appointments)
        return string.join(strlist, '<BR>')

    def make_row(self):
        if self.weekday in ('Saturday','Sunday'):
            row = TR(bgcolor=WEEKENDCOLOR)
        else:
            row = TR(bgcolor=WEEKDAYCOLOR)
        row.append(TD(self.repr_date_cell(), align='right',html_escape='off'))
        row.append(TD(self.repr_appt_cell(),html_escape='off'))
        return row
        
    def __str__(self):
        return str(self.make_row())
        
class DateTable( TableLite ):
    """Table of days used to represent the month in HTML.

    Adds methods allowing direct indexing by day of month.
    datetable[7] will return the Day object corresponding
    to the 7th day of the month.
    """
    def __setitem__(self, i, value):
        self.contents[i-1] = value
    def __getitem__(self, i):
    	return self.contents[i-1]

class Month( SimpleDocument ):
    """Class representing a month as an HTML document (page).

    Tweeks to the appearance of the calendar would mostly be done here.
    Indexing into this class instance will reference the DateTable object
    contained within.
    """
    def __init__(self, year=1997, month=1):
        SimpleDocument.__init__(self, bgcolor = PAGECOLOR)
        self.year = year
        self.month = month
        self.monthname = month_name[month]
        self.title = self.monthname + ' ' + str(self.year)
        self.ndays = mdays[month]
        now = time.strftime("%c", time.localtime(time.time()))
        self.append( '<P>', Font("Updated " + now, size = -1) )
        self.append( Center( H(3,self.title) ) )
        self.datetable = DateTable(cellpadding=6, cellspacing=2, border=3)
        self.jumper = ''
        for i in range(1, self.ndays+1):
            self.jumper = self.jumper + str(Href('#'+`month`+'-'+`i`, `i`)) + ' '
            self.datetable.append(Day(self.year, self.month, i))
        self.append( Center(self.jumper, html_escape='off') )
        self.append('<P>')
        self.append( self.datetable )
        
    def add_appt(self, appt):
        """Argument is a 6-tuple.

        Form:
           (year, month, day, hour, minute, text)
            int    int   int  str    str     str  <- type
        """
        (year, month, day, hour, minute, text) = appt
        if (self.year, self.month) != (year, month):
            raise DateError
        self[day].append(Appointment(hour, minute, text))
        
    def __setitem__(self, index, dayobj):
        try:
            self.datetable[index] = dayobj
        except IndexError:
            print 'Not %d days in %s' % (index, self.monthname)

    def __getitem__(self, index):
        return self.datetable[index]
        

class Year:
    """Represent a year as a series of 12 Month instances.
    """
    def __init__(self, year=1997):
        self.year = year
        self.months = [0] #spacer so indexing by month works.
        for m in range(1,13):
            self.months.append(Month(self.year, m))
        
    def load_appointments(self, data):
        """Load each entry in the appointment list.
        """
        for (year, month, day, hour, minute, text) in data:
            if year == self.year:
                self.months[month][day].append(Appointment(hour, minute, text))
    
    def makeindex(self, i):
        """Utility method to generate the navigation hyperlinks
        for each month. To be placed at the top of each page.
        """
        index = []
        for j in range(1,len(month_name)):
            if j == i:
                index.append(month_name[j])
            else:
                index.append(str(Href(self.namefile(j), month_name[j])))
        return string.join(index, ' | ')
        
    def namefile(self, i):
        """Generate the html filenames.
        """
        return "%s%s-%s.%s" % (self.directory,
                               self.prefix,
                               string.zfill(i,2),
                               HTMLSUFFIX)
        
    def write(self, directory='./', prefix = 'cal', mmin=1, mmax=12):
        """Causes the emission of all pages.

        To restrict ranges of months, use 3rd and 4th arg to specify
        starting and stopping months by number.
        """
        self.directory = directory
        self.prefix = prefix
        for i in range(mmin, mmax+1):
            self.months[i].prepend( Center(self.makeindex(i), html_escape='off') )
            self.months[i].write( self.namefile(i) )

def makeint(value):
    """return an integer given either a string or integer
    """
    try:
        return string.atoi(value)
    except TypeError:
        if type(value) == IntType:
            return value
        else:
            raise TypeError, ('cannot convert to int', value)
import regex
datepat = regex.compile('^ *\([0-9*][0-9]?\)[/-]' #first 2 char date field
                        '\([0-9][0-9]?\)[/-]?'    #second 2 char date field
                        '\([12][0-9][0-9][0-9]\)?[ \t]*:') #optional year field
daypat  = regex.compile('^ *\('+string.join(day_name,'\|')+'\)')
timepat = regex.compile('\([0-9][0-9]?\):\([0-9][0-9]\)')

def read_appt_file(filename):
    """Parsing function.

    Setting the day_month_order flag to 1 will make the parser read
    month value from the second position rather than the first.

    Example:
    
        2/15/1997: 3:25  Text for the appointment (year must be 4 digit)
                   4:45  This will be placed on the same day
        5/21:      8:00  Leaving off the year will pick up the current year
        */15:      2:00  This will place the appt on the 15th of every month
                   5:30  Also as above
        Friday:    3:30  Place this one on every Friday of the current year.

    The ":" after the day entry is significant.
    A single appointment text cannot span more than one line.
    """
    data = []
    current_year = time.localtime(time.time())[0]
    f = open(filename, 'r')
    for line in f.readlines():
        if string.strip(line) == '': #skip blank lines
            continue        
        if datepat.search(line) > -1:
            if day_month_order:
                month , day , year = datepat.group(2,1,3)
            else:
                month , day , year = datepat.group(1,2,3)
            if year == None:
                year = current_year
            else:
                year = makeint(year)
            line = line[datepat.regs[0][1]:]
        elif daypat.search(line) > -1:
            dayofweek = daypat.group(1)
            month, day = (None, None)
            year = current_year
            line = line[daypat.regs[0][1]:]
        if timepat.search(line) > -1:
            hour , minute = timepat.group(1,2)
            line = line[timepat.regs[0][1]:]
        else: #if no time is given just nullify the values
            hour = ''
            minute = ''
        text = string.strip(line)
        if month == '*':
            #print 'day of month case'
            for m in range(1,13):
                day = makeint(day)
                data.append((year, m, day, hour, minute, text))
        elif (month, day) == (None, None):
            #print 'day of week case'
            for (y, m, d) in list_day_of_week(year, dayofweek):
                data.append((y, m, d, hour, minute, text))
        else:
            #print 'normal case'
            month = makeint(month)
            day   = makeint(day)
            data.append((year, month, day, hour, minute, text))
            #print (year, month, day, hour, minute, text)
    f.close()
    return data

def list_day_of_week(year, daystring):
    """return a list of ~52 days corresponding to the given day of the week"""
    year = makeint(year)
    for i in range(1,8): #start at bgining of year look for day string
        if day_name[weekday(year, 1, i)] == daystring: break

    # now thay we know the initial day we add it to the list and
    # increment by 7 days
    list = [(year, 1, i)]
    sec_per_week = 60*60*24*7
    secs = time.mktime((year, 1, i, 0, 0, 0, 0, 0, 0))
    maxsecs = time.mktime((year, 12, 31, 1, 0, 0, 0, 0, 0))
    secs = secs + sec_per_week
    while secs < maxsecs:
        list.append(time.localtime(secs)[:3])
        secs = secs + sec_per_week
    return list

if __name__ == '__main__': main()

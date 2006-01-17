#! /usr/bin/env python
"Provide some supporting classes to simplify Input Forms with HTMLgen"
#'$Id$'
# COPYRIGHT (C) 1999  ROBIN FRIEDRICH  email: Robin.Friedrich@pdq.net
# Permission to use, copy, modify, and distribute this software and
# its documentation for any purpose and without fee is hereby granted,
# provided that the above copyright notice appear in all copies and
# that both that copyright notice and this permission notice appear in
# supporting documentation.
# THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
# SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
# FITNESS, IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
# RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
# CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

from HTMLgen import *
from HTMLcolors import *
import string

class InputTable:
    """InputTable(entries, [keyword=value]...)

    Entries is a list of 2 or 3-element tuples (name, input_object, note) where
    name will label the input object on the left column, the input_object
    can be anything, and the note is a string to provide optional notations
    to the right of the input widgets.
    """
    def __init__(self, entries=None, **kw):
        self.entries = []
        self.leftcolor = GREY2   #color used for left label column
        self.centercolor = WHITE #color used for center input column
        self.rightcolor = WHITE  #color used for right label column
        self.notecolor = RED     #color used for notation text
        self.leftalign = 'right' #text alignment for left label column
        self.defaultsize = 20
        if entries is not None:
            for entry in entries:
                self.append(entry)
        for item in kw.keys():
            if self.__dict__.has_key(item):
                self.__dict__[item] = kw[item]
            else:
                detail = "%s not a valid parameter of the %s class." % (item, self.__class__)
                raise KeyError, detail

    def append(self, *items):
        for item in items:
            li = len(item)
            if li == 1:
                self.entries.append((item[0], Input(name=item[0], size=self.defaultsize), ''))
            elif li == 2:
                self.entries.append((item[0], item[1], ''))
            elif li == 3:
                self.entries.append((item[0], item[1], item[2]))
            else:
                raise ValueError('Appended item must be a sequence of 1, 2, or 3 elements')
                                   
    def __str__(self):
        table = TableLite(border=0, cellspacing=4)
        for label, input, note in self.entries:
            row = TR()
            row.append(TD(label, align=self.leftalign, bgcolor=self.leftcolor))
            row.append(TD(input, bgcolor=self.centercolor))
            if note:
                row.append(TD(Font(note, color=self.notecolor),
                              bgcolor=self.rightcolor))
            table.append(row)
        return str(table)

class RadioButtons:
    """RadioButtons(itemlist, keyword=value):

    """
    widgettype = 'radio'
    def __init__(self, items, **kw):
        self.items = items
        self.name = 'choice'
        self.orient = 'vertical'
        self.selected = []
        for (item, value) in kw.items():
            setattr(self, item, value)
    def __str__(self):
        if self.orient[0] == 'v':
            sep = '<BR>\n'
        else:
            sep = ', \n'
        if type(self.selected) is type(""):
            self.selected = [self.selected]
        s = []
        for item in self.items:
            if item in self.selected:
                s.append(str(Input(type=self.widgettype, name=self.name, checked=1)))
            else:
                s.append(str(Input(type=self.widgettype, name=self.name)))
            s.append(str(item))
            s.append(sep)
        return string.join(s[:-1], "")

class CheckBoxes(RadioButtons):
    widgettype = 'checkbox'

def test1():
    doc = SimpleDocument(title='Income Survey', bgcolor=WHITE) # if this in a CGI prog use cgi=1
    F = Form('http://www.kettleman.edu/cgi-bin/income-survey.py') # the URL of the CGI program

    survey = []
    survey.append(['City', Input(name='city', size=30), 'Required'])
    survey.append(['State', Input(name='state', size=20), 'Required'])
    survey.append(['Sex', RadioButtons(['Male', 'Female', 'Other']), ''] )
    incomes = [('$0 - $10K', 0), ('$10K - $25K', 1), ('$25K - $40K', 2), ('$40K +', 3)]
    survey.append(['Income', Select(incomes, name='income', size=1), 'Required'] )
    
    F.append(InputTable(survey))
    doc.append(F)
    doc.write('survey.html')
    
def test2():
    doc = SimpleDocument(title='YourDorm Pizza Delivery', bgcolor=WHITE)
    form = Form()
    stuff = ['Extra Cheese', 'Pepperoni', 'Sausage', 'Mushrooms',
             'Peppers', 'Onions', 'Olives']
    sizes = ['Small', 'Medium', 'Large', 'Extra Large']
    
    tab = ( ('Name', Input(name='Name',size=30)),
            ('Dorm', Input(name='Dorm',size=30)),
            ('Room', Input(name='Room',size=5)),
            ('Size', RadioButtons(sizes, name='size', selected='Large')),
            ('Toppings', CheckBoxes(stuff, name='toppings',orient='horiz'))
            )

    form.append(InputTable(tab))
    doc.append(form)
    doc.write('test.html')

def test():
    doc = SimpleDocument(bgcolor=WHITE)
    form = Form()
    doc.append(form)
    stuff = []
    names = ( ('Name', 'Required'),
              ('Street1', 'Required'),
              ('Street2', ''),
              ('City', 'Required'),
              ('State/Province', 'Required'),
              ('Postal Code', 'Required'),
              ('Country', ''),
              ('Phone', '')  )
    for (name, note) in names:
        stuff.append( [name, Input(name=name, size=30), note] )
    form.append(InputTable(stuff, rightcolor=GREY2))

    doc.write('test.html')



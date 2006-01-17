#!/bin/env python

"""Test script which generates the online documentation for HTMLgen.
"""
import string, regex, regsub, os, time, glob
from HTMLcolors import *
from HTMLgen import *
import HTMLgen  #only so I can pick off the __version__
__version__ = '$Id$'
htmldir = './html'
datadir = './data'
doctitle = 'HTMLgen %s Online Documentation' % HTMLgen.__version__
try:
    os.mkdir('html')
except os.error:
    pass

def test():
    # build_pages list is defined at the bottom of this file
    # to pick up the following function objects.
    t = time.clock()
    for i in range(1, len(build_pages)-1):
        build_pages[i][1](build_pages[i][0],   # current filename
                          build_pages[i-1][0], # previous filename
                          build_pages[i+1][0], # next filename
                          'overview.html')      # manual top
                          #leaving out a HOME button
    import colorcube
    colorcube.main(os.path.join(htmldir, 'colorcube.html'))
    
    print 'Time to generate pages:', time.clock() - t, 'seconds'


def overview(filename, aft=None, fore=None, top=None, home=None):
    doc = SeriesDocument('HTMLgen.rc')
    doc.title = doctitle
    doc.subtitle = 'Overview'
    doc.banner = ('../image/HTMLgen_banner.jpg')
    doc.goprev,doc.gonext,doc.gotop,doc.gohome = aft,fore,top,home
    doc.append_file(os.path.join(datadir, 'overview-txt.html'))
    
    doc.write(os.path.join(htmldir, filename))

    
def document(filename, aft=None, fore=None, top=None, home=None):
    doc = SeriesDocument('HTMLgen.rc')
    doc.title = doctitle
    doc.subtitle = 'Document Objects'
    doc.banner = ('../image/document.gif', 472, 40)
    doc.goprev,doc.gonext,doc.gotop,doc.gohome = aft,fore,top,home
    doc.append_file(os.path.join(datadir, 'document-txt.html'))

    doc.write(os.path.join(htmldir, filename))
    
def lists(filename, aft=None, fore=None, top=None, home=None):
    doc = SeriesDocument('HTMLgen.rc')
    doc.title = doctitle
    doc.subtitle = 'Lists'
    doc.banner = ('../image/lists.gif', 472, 40)
    doc.goprev,doc.gonext,doc.gotop,doc.gohome = aft,fore,top,home
    doc.append_file(os.path.join(datadir, 'lists-txt.html'))
    doc.append(HR())
    
    nlist = ['Items',['First Item', 'Second Item',['SubitemA','SubitemB'], 'Third Item'], 'More']
    doc.append('The following section just exercises most of the markup classes.')
    ilist = [(Image('../image/purple_dot.gif'), Bold('Purple')),
	     (Image('../image/orange_dot.gif'), Bold('Orange')),
	     (Image('../image/red_dot.gif'), Bold('Red')),
	     (Image('../image/blue_dot.gif'), Bold('Blue')),
	     (Image('../image/green_dot.gif'), Bold('Green')),
	     (Image('../image/yellow_dot.gif'), Bold('Yellow')) ]
    doc.append(Heading(3, 'List class'))
    olist = OrderedList(type='I', columns=2, bgcolor="#DDDDDD")
    olist.append('First item in the OrderedList (ImageBulletList)',
                  ImageBulletList(ilist),
                  'Second item in the OrderedList',
                  List(nlist))
    doc.append(olist)
    sample = """"Don't play dumb. You're not as good at it as I am.": Colonel Flagg - M*A*S*H"""
    list = DefinitionList( [
        ('This should be initial upper caps', InitialCaps('Initial Upper Capital Letters')),
        ('This is normal Text with >< &  escaped.', Text('<&>'+sample)),
        ('This is Blockquote markup.', Blockquote(sample)),
        ('This is red text.', Font(sample, color=RED)),
        ('This is Address markup.', Address(sample)),
        ('This is Emphasis markup.', Emphasis(sample)),
        ('This is Cite markup.', Cite(sample)),
        ('This is KBD markup.', KBD(sample)),
        ('This is Sample markup.', Sample(sample)),
        ('This is Code markup.', Code(sample)),
        ('This is Define markup.', Define(sample)),
        ('This is Var markup.', Var(sample)) ] )
    doc.append(list)
    
    doc.write(os.path.join(htmldir, filename))
    
def frames(filename, aft=None, fore=None, top=None, home=None):
    doc = SeriesDocument('HTMLgen.rc')
    doc.title = doctitle
    doc.subtitle = 'Frames'
    doc.goprev,doc.gonext,doc.gotop,doc.gohome = aft,fore,top,home
    doc.banner = ('../image/frames.gif', 472, 40)
    doc.append_file(os.path.join(datadir, 'frames-txt.html'))

    doc.write(os.path.join(htmldir, 'top-'+filename))

    f1 = Frame(name='top', src='top-'+filename)
    f2 = Frame(name='bottom', src='./parrot.html')
    fs = Frameset(f1, f2, rows='50%,50%')
    fdoc = FramesetDocument()
    fdoc.append(fs)
    fdoc.write(os.path.join(htmldir, filename))

def chart(filename, aft=None, fore=None, top=None, home=None):
    """exercises the barchart module"""
    import barchart
    doc = SeriesDocument('HTMLgen.rc')
    doc.title = doctitle
    doc.subtitle = 'Tables'
    doc.banner = ('../image/tables.gif', 472, 40)
    doc.goprev,doc.gonext,doc.gotop,doc.gohome = aft,fore,top,home
    doc.append_file(os.path.join(datadir, 'tables-txt.html'))
    doc.append(HR())
    dum = [ ('fddo4', 1318), ('cn1', 1472), ('cn2', 1411),
            ('fddo3', 1280), ('asc8', 1371), ('orb3', 1390),
            ('fddo1', 1418), ('asc4', 1292), ('dn2', 1381),
            ('fddo2', 1341), ('asc1', 1352), ('dn1', 1441) ]
    dummydata = barchart.DataList()
    dummydata.load_tuples(dum)
    dummydata.sort()
    b = barchart.BarChart(dummydata)
    b.label_shade = AQUA
    b.thresholds = (1300, 1400)
    b.title = "System Throughput (jobs/week)"
    doc.append(b)
    doc.append(Pre(str(dummydata)))
    doc.append(HR())
    dum = [ ('fddo4', 1318, 456, 235, 290),
            ('fddo3', 1280, 560, 129, 295), 
            ('fddo1', 1418, 1201, 490, 125),
            ('fddo2', 1341, 810, 466, 203) ]

    dummydata = barchart.DataList()
    dummydata.segment_names = ('User','System','I/O','Wait')
    dummydata.load_tuples(dum)
    dummydata.sort()
    b = barchart.StackedBarChart(dummydata)
    b.label_shade = AQUA
    b.title = "System Load"
    doc.append(b)
    doc.append(Pre(str(dummydata)))
    doc.write(os.path.join(htmldir, filename))
    
def forms(filename, aft=None, fore=None, top=None, home=None):
    doc = SeriesDocument('HTMLgen.rc', banner=('../image/forms.gif', 472, 40))
    doc.title = doctitle
    doc.subtitle = 'Forms'
    doc.goprev,doc.gonext,doc.gotop,doc.gohome = aft,fore,top,home
    doc.append_file(os.path.join(datadir, 'forms-txt.html'))
    doc.append(HR())
    
    folders = ('.','html')
    F = Form('http://hoohoo.ncsa.uiuc.edu/cgi-bin/post-query')
    for folder in folders:
        files = os.listdir(folder)
        J = Select(files, name=folder, size=4, multiple=1)
        F.append(J)
    F.append(Select(files[:10], name='Popup', size=1))
    F.append(Textarea('You may send stuff in here.'))
    F.append(P())
    for item in ('pick me','no, pick me!', 'forget them, pick me!!'):
        radio = Input(type='radio',  name='rad', rlabel=item)
        F.append(radio, BR())
    F.append(P())
    for item in ('Pepperoni','Sausage','Extra Cheese'):
        check = Input(type='checkbox', name='CB', rlabel=item)
        F.append(check, BR())
    F.append(Input(type='password',  name='pw', llabel='Password'), P())
    F.append(Input(type='file', name='text'))
    F.submit = Input(type='submit', value='Fire off to the server')
    doc.append(F)

    doc.write(os.path.join(htmldir, filename))
    
def imagemaps(filename, aft=None, fore=None, top=None, home=None):
    doc = SeriesDocument('HTMLgen.rc')
    doc.title = doctitle
    doc.subtitle = 'Imagemaps'
    doc.goprev,doc.gonext,doc.gotop,doc.gohome = aft,fore,top,home
    doc.banner = ('../image/imapbanner.gif', 472, 40)
    doc.append_file(os.path.join(datadir, 'imagemaps-txt.html'))
    doc.append(HR())

    doc.append(H(3, 'Moving the mouse pointer over the numbered areas should\
    make Netscape show a URL name numbered accordingly.'))
    a1 = Area(coords='0,0,50,50', href='a1.html')
    a2 = Area(coords='0,50,50,100', href='a2.html')
    a3 = Area(coords='0,100,50,150', href='a3.html')
    a4 = Area(coords='0,150,50,200', href='a4.html')
    a5 = Area(coords='0,200,50,250', href='a5.html')
    csmap = Map(name='rect', areas=(a1,a2,a3,a4,a5))
    csimage = Image('../image/csmap_rect.gif', usemap='#rect')
    doc.append(csimage, csmap)

    a1 = Area(shape='circle', coords='50,25,25', href='a1.html')
    a2 = Area(shape='circle', coords='100,25,25', href='a2.html')
    a3 = Area(shape='circle', coords='150,25,25', href='a3.html')
    a4 = Area(shape='circle', coords='200,25,25', href='a4.html')
    a5 = Area(shape='circle', coords='250,25,25', href='a5.html')
    csmap = Map(name='circle', areas=(a1,a2,a3,a4,a5))
    csimage = Image('../image/csmap_circle.gif', usemap='#circle', align='top')
    doc.append(csimage, csmap)

    doc.write(os.path.join(htmldir, filename))
    
def scripts(filename, aft=None, fore=None, top=None, home=None):
    doc = SeriesDocument('HTMLgen.rc')
    doc.title = doctitle
    doc.subtitle = 'Embedded Scripts'
    doc.banner = ('../image/scripts.gif', 472, 40)
    doc.goprev,doc.gonext,doc.gotop,doc.gohome = aft,fore,top,home
    doc.append_file(os.path.join(datadir, 'scripts-txt.html'))

    doc.write(os.path.join(htmldir, filename))

def sample1(filename, aft=None, fore=None, top=None, home=None):
    doc = SeriesDocument('HTMLgen.rc')
    doc.goprev,doc.gonext,doc.gotop,doc.gohome = aft,fore,top,home
    doc.background = '../image/texturec.jpg'
    doc.banner = ('../image/historic.gif', 472, 60)
    doc.author = '1776 Thomas Jefferson'
    doc.email = 'jefferson@montecello.virginia.gov'
    doc.logo = ('../image/eagle21.gif', 64, 54)
    # parse Declaration of Independence
    re_hline = regex.compile('^--+$')
    re_title = regex.compile('^Title:\(.*$\)')
    font2 = Font(size='+2')
    s = open(os.path.join(datadir, 'DoI.txt')).read()
    paragraphs = regsub.split(s, '\n\([\t ]*\n\)+')
    for para in paragraphs:
        if not para: continue
        if re_title.search(para) > -1:
            doc.title = re_title.group(1)
        elif re_hline.search(para) > -1:
            doc.append(HR())
        else:
            p = Paragraph( para )
            # using \` to match beginning of paragraph
            # ^ won't work because it'll match all the newlines
            n = p.markup('\`\(\w\)', font2, reg_type='regex')
            doc.append(p)
    doc.write(os.path.join(htmldir, filename))

def sample2(filename, aft=None, fore=None, top=None, home=None):
    doc = SeriesDocument('HTMLgen.rc')
    doc.goprev,doc.gonext,doc.gotop,doc.gohome = aft,fore,top,home
    doc.author = '1969 Montgomery Python'
    doc.email = 'cleese@bbc.co.uk'
    doc.banner = ('../image/silywalk2.gif', 450, 110)
    doc.bgcolor = PURPLE
    doc.title = "It's..."
    doc.subtitle = 'The Dead Parrot Sketch'
    #Ok parse that file
    f = open(mpath(os.path.join(datadir, 'parrot.txt')))
    line = f.readline()
    re_dialog = regex.compile('\(^[OC].*:\)\(.*\)')
    while line:
        if re_dialog.search(line) > -1:
            role, prose = re_dialog.group(1,2)
            if role[0]=='C': role = 'Customer: '
            if role[0]=='O': role = 'Owner: '
            doc.append(BR(), Strong(role), ' ')
            doc.append(prose)
        else:
            doc.append(' ', line)
        line = f.readline()
    f.close()
    doc.append(P(), Image('../image/parrot4.gif', width=169, height=104))
    doc.write(os.path.join(htmldir, filename))

################################
# define the pages which are to be generated by the test function
pages = [ ('overview.html', overview),
          ('document.html', document),
          ('lists.html', lists),
          ('frames.html', frames),
          ('tables.html', chart),
          ('forms.html', forms),
          ('imagemaps.html', imagemaps),
          ('scripts.html', scripts),
          ('independence.html', sample1),
          ('parrot.html', sample2) ]
build_pages = [(None, None)] + pages + [(None, None)] 


#################################
if __name__ == '__main__': test()

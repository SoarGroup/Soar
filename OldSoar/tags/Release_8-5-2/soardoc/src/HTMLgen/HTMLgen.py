#'$Id$'

# COPYRIGHT (C) 1996-9  ROBIN FRIEDRICH  email:Robin.Friedrich@pdq.net
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

"""A class library for the generation of HTML documents.

Each HTML tag type has a supporting class which is responsible for
emitting itself as valid HTML formatted text. An attempt is made to
provide classes for newer HTML 3.2 and proposed tag elements.  The
definitive reference for HTML tag elements can be found at
[W3C].  Also, I used the HTML book by Musciano and
Kennedy from [O Reilly] (2nd. Ed.) as the guiding reference.

The Document classes are container objects which act as a focal point
to populate all the contents of a particular web page. It also can
enforce consistent document formating according to the guidelines from
the [Yale Web Style Manual].

Features include customization of document template graphics / colors
through use of resource files, minimizing the need for modifying or
subclassing from the module source code. Support for tables, frames,
forms (persistent and otherwise) and client-side imagemaps are included.

A newer implementation for the Table support is now included,
TableLite().  In support of this there are new tag classes TD, TH, TR
and Caption.  These class instances can be assembled in any way to
populate the TableLite container object. 

.. [W3C] http://www.W3.org/TR/REC-html32.html
.. [O Reilly] http://www.oreilly.com/catalog/html3/index.html
.. [Yale Web Style Manual] http://info.med.yale.edu/caim/manual/contents.html
"""

import string, re, time, os
import UserList, copy
from imgsize import imgsize

__author__ = 'Robin Friedrich   friedrich@pythonpros.com'
__version__ = '2.2.2'

StringType = type('s')
IntType    = type(3)
ListType   = type([1])
TupleType  = type((1,2))
InstanceType = type(UserList.UserList())
CONTYPE = 'Content-Type: text/html\n\n'
DOCTYPE = '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 3.2//EN">\n<HTML>\n'
PRINTECHO = 1


#################
# CLASS LIBRARY #
#################


#======= NEW CLASS STRUCTURE ============
# HTMLgen 1       HTMLgen 2
# --------------- ------------------
# Document        SeriesDocument
# MinimalDocument SimpleDocument
#                 BasicDocument (base class)
#                 TemplateDocument

class BasicDocument:
    """Base class to define an HTML document.

    Non-keyword arguments are taken as the initial contents for this object.

    Keyword arguments:
        title -- HTML TITLE attribute for document
        bgcolor -- background color expressed in hex-triplet or names from HTMLcolors.
        background -- background image filename
        cgi -- flag to indicate if this is used in CGI context (1 if it is)
        textcolor -- color to use for normal text
        linkcolor -- color to use for hyperlinks
        vlinkcolor -- color to use for visited hyperlinks
        alinkcolor -- color to use when hyperlink is active
    """
    title = ''
    cgi = None
    bgcolor = None
    background = None
    textcolor = None
    linkcolor = None
    vlinkcolor = None
    alinkcolor = None
    
    def __init__(self, *args, **kw):
        self.contents = list(args)
        for name, value in kw.items():
            setattr(self, name, value)

    def __str__(self):
        s = []
        if self.cgi:
            s.append('Content-Type: text/html\n\n' + DOCTYPE)
        else:
            s.append(DOCTYPE)
        s.append('\n<!-- This file generated using Python HTMLgen module. -->\n')

        # build the HEAD and BODY tags
        s.append(self.html_head())
        s.append(self.html_body_tag())

        # DOCUMENT CONTENT SECTION and FOOTER added on
        bodystring = '%s\n' * len(self.contents)
        s.append((bodystring % tuple(self.contents)))

        # CLOSE the document
        s.append('\n</BODY> </HTML>\n')
        return string.join(s, '')

    def html_head(self):
        """Generate the HEAD, TITLE and BODY tags.
        """
        return '<HEAD>\n  <META NAME="GENERATOR" CONTENT="HTMLgen %s">\n\
        <TITLE>%s</TITLE> </HEAD>\n' % (__version__, self.title)
    
    def html_body_tag(self):
        """Return BODY tag with attributes.
        """
        s = ['<BODY']
        if self.bgcolor:    s.append(' BGCOLOR="%s"' % self.bgcolor)
        if self.background: s.append(' BACKGROUND="%s"' % self.background)
        if self.textcolor:  s.append(' TEXT="%s"' % self.textcolor)
        if self.linkcolor:  s.append(' LINK="%s"' % self.linkcolor)
        if self.vlinkcolor: s.append(' VLINK="%s"' % self.vlinkcolor)
        if self.alinkcolor: s.append(' ALINK="%s"' % self.alinkcolor)
        s.append('>\n')
        return string.join(s, '')

    def append_file(self, filename, marker_function = None):
        """Add the contents of a file to the document.

        filename -- the filename of the file to be read [string]
        marker_function -- a callable object which the text read from
          the file will be passed through before being added to the
          document.
        """
        f = open(mpath(filename), 'r')
        if marker_function:
            self.append(marker_function(f.read()))
        else:
            self.append(f.read())
        f.close()
        
    def append(self, *items):
        """Add content to the Document object.
        
        Arg *items* can be plain text or objects; multiple arguments supported.
        """
        for item in items:
            self.contents.append(item)

    def prepend(self, *items):
        """Add content to the beginning of the Document object.
        
        Arg *items* can be plain text or objects; multiple arguments supported.
        """
        for item in items:
            self.contents.insert(0, item)

    def copy(self):
        """Return a complete copy of the current Document object.
        """
        return copy.deepcopy(self)

    def write(self, filename = None):
        """Emit the Document HTML to a file or standard output.
        
        In Unix you can use environment variables in filenames.
        Will print to stdout if no argument.
        """
        if filename:
            f = open(mpath(filename), 'w')
            f.write(str(self))
            f.close()
            if PRINTECHO: print 'wrote: "'+filename+'"'
        else:
            import sys
            sys.stdout.write(str(self))

class FramesetDocument(BasicDocument):
    """A minimal document suitable for entering Framesets.

    Arguments are for contents **NOT** a document resource file.

    Keyword Parameters
    
        title -- string to be used as the document title.
        base  -- object of the Base class
        meta  -- object of the Meta class
        cgi   -- if non zero will issue a mime type of text/html
        script -- a single or list of Script objects to be included in the <HEAD>
    
    No <body> markup. Instead add Frameset(s) with the constructor or
    append method.
    """
    base = None
    meta = None
    cgi = None
    script = None

    def __str__(self):
        s = []
        if self.cgi:
            s.append('Content-Type: text/html\n\n' + DOCTYPE)
        else:
            s.append(DOCTYPE)
        s.append('\n<!-- This file generated using Python HTMLgen module. -->\n')

        # build the HEAD tag
        s.append(self.html_head())

        # DOCUMENT CONTENT SECTION
        bodystring = '%s\n' * len(self.contents)
        s.append((bodystring % tuple(self.contents)))

        # CLOSE the document
        s.append('\n</HTML>')        
        return string.join(s, '')


class SimpleDocument(BasicDocument):
    """Supports all features of a self contained document.

    This includes support for CSS1, meta and base tags, and embedded
    scripts.

    First constructor argument is resource file containing document
    attribute settings.
    """
    base = None
    style = None
    stylesheet = None
    meta = None
    onLoad = None
    onUnload = None
    script = None
    
    def __init__(self, resource = None, **kw):
        self.contents = []
        # Read attributes from resource file into instance namespace
        if resource: execfile(mpath(resource), self.__dict__)
        for name, value in kw.items():
            setattr(self, name, value)

    def __str__(self):
        s = []
        if self.cgi:
            s.append('Content-Type: text/html\n\n' + DOCTYPE)
        else:
            s.append(DOCTYPE)
        s.append('\n<!-- This file generated using Python HTMLgen module. -->\n')

        # build the HEAD and BODY tags
        s.append(self.html_head())
        s.append(self.html_body_tag())

        # DOCUMENT CONTENT SECTION and FOOTER added on
        bodystring = '%s\n' * len(self.contents)
        s.append((bodystring % tuple(self.contents)))

        s.append('\n</BODY> </HTML>\n') # CLOSE the document
        return string.join(s, '')

    def html_head(self):
        """Generate the HEAD TITLE and BODY tags.
        """
        s = ['<HEAD>\n  <META NAME="GENERATOR" CONTENT="HTMLgen %s">\n\
        <TITLE>%s</TITLE>\n' % (__version__, self.title)]
        if self.meta: s.append(str(self.meta))
        if self.base: s.append(str(self.base))
        if self.stylesheet:
            s.append('\n <LINK rel=stylesheet href="%s" type=text/css title="%s">\n' \
                     % (self.stylesheet, self.stylesheet))
        if self.style:
            s.append('\n<STYLE>\n<!--\n%s\n-->\n</style>\n' % self.style)
        if self.script: # for javascripts
            if type(self.script) in (TupleType, ListType):
                for script in self.script:
                    s.append(str(script))
            else:
                s.append(str(self.script))
        s.append('</HEAD>\n')
        return string.join(s, '')

    def html_body_tag(self):
        """Return BODY tag with attributes.
        """
        s = ['<BODY']
        if self.bgcolor:    s.append(' BGCOLOR="%s"' % self.bgcolor)
        if self.background: s.append(' BACKGROUND="%s"' % self.background)
        if self.textcolor:  s.append(' TEXT="%s"' % self.textcolor)
        if self.linkcolor:  s.append(' LINK="%s"' % self.linkcolor)
        if self.vlinkcolor: s.append(' VLINK="%s"' % self.vlinkcolor)
        if self.alinkcolor: s.append(' ALINK="%s"' % self.alinkcolor)
        if self.onLoad:     s.append(' onLoad="%s"' % self.onLoad)
        if self.onUnload:   s.append(' onUnload="%s"' % self.onUnload)
        s.append('>\n')
        return string.join(s, '')


class SeriesDocument(SimpleDocument):
    """Primary container class for an HTML document as part of a series.

    Formerly known as Document().

    Navigation mechanisms are provided.

    Single optional string argument for the path to a resource file
    used to specify document parameters. This helps minimize the need
    for subclassing from this class. Keyword parameters may be used
    for any of the following class attributes. See *HTMLtest.py* for
    example usage.

    Class instance attributes and keyword arguments
    
        base -- object of the Base class
        meta -- object of the Meta class
        cgi  -- if non zero will issue a mime type of text/html
        logo -- ('filename', width, height)  All images are specified
                 with a tuple of string, int, int. If the size of the
                 graphic is unknown, use 0, 0.  This one is the little
                 graphic on the footer of each page.
        banner -- ('filename', width, height) Banner graphic at
                 the top of page. Can also be set to a string filename
                 or an Image object. Can be autosized if it's a GIF.
        title --  string to be used as the document title.
        subtitle -- string to be used as the document subtitle.
                 If non-nil, this string will be used for the doc title
                 instead of title.
        author -- String used in the copyright notice
        email -- Email address for feedback mailto: tag
        zone -- string used to label the time zone if datetime
                 is used. By default not used.
        bgcolor -- Color string (can use variables from
                 HTMLcolors.py)
        background -- string filename of a graphic used as the
                 doc background.
        textcolor -- Color string used for text.  (can use
                 variables from HTMLcolors.py)
        linkcolor -- Color string used for hyperlinked text. 
        vlinkcolor -- Color string used for visited hypertext.
        alinkcolor -- Color string used for active hypertext.
        place_nav_buttons -- Flag to enable/disable the use of
                 navigation buttons.
                 Default is on. Set to 0 to disable.
        blank -- Image tuple for the transparent spacer gif
        prev -- Image tuple for the Previous Page button
        next -- Image tuple for the Next Page button
        top -- Image tuple for the Top of Manual button
        home -- Image tuple for the site Home Page button
        goprev -- URL string for the prev button
        gonext -- URL string for the next button
        gotop  -- URL string for the top button
        gohome -- URL string for the home button
        script -- a single or list of Script objects to be included in the <HEAD>
        onLoad -- Script, which is executed when the document is loaded
        onUnload -- Script, which is executed when the document is unloaded
    """
    subtitle = None
    banner = ('/image/banner.gif', 472, 30)
    logo = ('/image/logo.gif', 36, 35)
    author = 'Micky Mouse'
    email = 'micky@disney.com'
    zone = ' Central US'
    place_nav_buttons = 'yes'
    blank = ('../image/blank.gif', 71, 19)
    prev = ('../image/BTN_PrevPage.gif', 71, 19)
    next = ('../image/BTN_NextPage.gif', 71, 19)
    top = ('../image/BTN_ManualTop.gif', 74, 19)
    home = ('../image/BTN_HomePage.gif', 74, 19)
    goprev = None # URLs for above navigation buttons
    gonext = None
    gotop  = None
    gohome = None

    def __str__(self):
        s = []
        if self.cgi:
            s.append(CONTYPE + DOCTYPE)
        else:
            s.append(DOCTYPE)
        s.append('\n<!-- This file generated using Python HTMLgen module. -->\n')
        # build the HEAD and BODY tags
        s.append(self.html_head())
        s.append(self.html_body_tag())
        # HEADER SECTION
        s.append(self.header())

        # DOCUMENT CONTENT SECTION and FOOTER added on
        bodystring = '%s\n' * len(self.contents)
        s.append((bodystring % tuple(self.contents)))
        
        s.append(self.footer())
        s.append('\n</BODY> </HTML>\n') # CLOSE the document
        return string.join(s, '')
    
    def header(self):
        """Generate the standard header markups.
        """
        # HEADER SECTION - overload this if you don't like mine.
        s = []
        if self.banner:
            bannertype = type(self.banner)
            if bannertype in (TupleType, StringType):
                s.append(str(Image(self.banner, border=0)) + '<BR>\n')
            elif bannertype == InstanceType:
                s.append(str(self.banner) + '<BR>\n')
            else:
                raise TypeError, 'banner must be either a tuple, instance, or string.'
        if self.place_nav_buttons:
            s.append(self.nav_buttons())
        s.append(str(Heading(3,self.title)))
        if self.subtitle:
            s.append('<H2>%s</H2>\n' % self.subtitle)
        s.append('<HR>\n\n')
        return string.join(s, '')

    def nav_buttons(self):
        """Generate hyperlinked navigation buttons.

        If a self.go* attribute is null that corresponding button is
        replaced with a transparent gif to properly space the remaining
        buttons.
        """
        s = []
        if self.goprev: # place an image button for previous page
            btn = Image(self.prev, border=0, alt='Previous')
            link = Href(self.goprev, str(btn))
            s.append(str(link) + ' \n')
        else: # place a blank gif as spacer
            #btn = Image(self.blank)
            s.append('<span style="width: 60px"></span> \n')
        if self.gonext: # place an image button for next page
            btn = Image(self.next, border=0, alt='Next')
            link = Href(self.gonext, str(btn))
            s.append(str(link) + ' \n')
        else: # place a blank gif as spacer
            btn = Image(self.blank)
            s.append(str(btn) + ' \n')
        if self.gotop: # place an image button for top of manual page
            btn = Image(self.top, border=0, alt='Top of Manual')
            link = Href(self.gotop, str(btn))
            s.append(str(link) + ' \n')
        else: # place a blank gif as spacer
            btn = Image(self.blank)
            s.append(str(btn) + ' \n')
        if self.gohome: # place an image button for site home page
            btn = Image(self.home, border=0, alt='Home Page')
            link = Href(self.gohome, str(btn))
            s.append(str(link) + ' \n')
        else: # place a blank gif as spacer
            btn = Image(self.blank)
            s.append(str(btn) + ' \n')
        return string.join(s, '')
 
    def footer(self):
        """Generate the standard footer markups.
        """
        # FOOTER SECTION - overload this if you don't like mine.
        t = time.localtime(time.time())
        #self.datetime = time.strftime("%c %Z", t)    #not available in JPython
        self.datetime = time.asctime(t)
        #self.date = time.strftime("%A %B %d, %Y", t)
        x = string.split(self.datetime)
        self.date = x[0] + ' ' + x[1] + ' ' + x[2] + ', ' + x[4]
        s =  ['\n<P><HR>\n']
        if self.place_nav_buttons:
            s.append(self.nav_buttons())
        s.append('<BR>' + str(Image(self.logo, align='bottom')))
        s.append('\n<FONT SIZE="-1"><P>Copyright &#169 %s<BR>All Rights Reserved<BR>\n' \
            % self.author)
        s.append('\nComments to author: ' + str(MailTo(self.email)) )
        s.append('<br>\nGenerated: %s <BR>' % self.date) # can use self.datetime here instead
        s.append('<hr>\n</FONT>')
        return string.join(s, '')

# Aliases for backward compatability with HTMLgen 1.2
Document = SeriesDocument
MinimalDocument = SimpleDocument


class StringTemplate:
    """Generate documents based on a template and a substitution mapping.

    Must use Python 1.5 or newer. Uses re and the get method on dictionaries.

    Usage:
       T = TemplateDocument('Xfile')
       T.substitutions = {'month': ObjectY, 'town': 'Scarborough'}
       T.write('Maine.html')

    A dictionary, or object that behaves like a dictionary, is assigned to the
    *substitutions* attribute which has symbols as keys to objects. Upon every
    occurance of these symbols surrounded by braces {} in the source template,
    the corresponding value is converted to a string and substituted in the output.

    For example, source text which looks like:
     I lost my heart at {town} Fair.
    becomes:
     I lost my heart at Scarborough Fair.

    Symbols in braces which do not correspond to a key in the dictionary remain
    unchanged.

    An optional third argument to the class is a list or two strings to be
    used as the delimiters instead of { } braces. They must be of the same
    length; for example ['##+', '##'] is invalid.
    """
    def __init__(self, template, substitutions=None, **kw):
        self.delimiters = ['{', '}']
        self.__dict__.update(kw)
        if len(self.delimiters) != 2:
            raise ValueError("delimiter argument must be a pair of strings")
        self.delimiter_width = len(self.delimiters[0])
        delimiters = map(re.escape, self.delimiters)
        self.subpatstr = delimiters[0] + "[\w_]+" + delimiters[1]
        self.subpat = re.compile(self.subpatstr)
        self.substitutions = substitutions or {}
        self.set_template(template)

    def set_template(self, template):
        self.source = template
    
    def keys(self):
        return self.substitutions.keys()

    def __setitem__(self, name, value):
        self.substitutions[name] = value
        
    def __getitem__(self, name):
        return self.substitutions[name]
      
    def __str__(self):
        return self._sub(self.source)

    def _sub(self, source, subs=None):
        """Perform source text substitutions.

        *source* string containing template source text
        *subs* mapping of symbols to replacement values
        """
        substitutions = subs or self.substitutions
        dw = self.delimiter_width
        i = 0
        output = []
        matched = self.subpat.search(source[i:])
        while matched:
            a, b = matched.span()
            output.append(source[i:i+a])
            # using the new get method for dicts in 1.5
            output.append(str(substitutions.get(
                   source[i+a+dw:i+b-dw], source[i+a:i+b])))
            i = i + b
            matched = self.subpat.search(source[i:])
        else:
            output.append(source[i:])
        return string.join(output, '')
    
    def write(self, filename = None):
        """Emit the Document HTML to a file or standard output.
        
        Will not overwrite file is it exists and is textually the same.
        In Unix you can use environment variables in filenames.
        Will print to stdout if no argument given.
        """
        if filename:
            filename = mpath(filename)
            if os.path.exists(filename):
                s = str(self)
                if compare_s2f(s, filename):
                    f = open(filename, 'w')
                    f.write(str(self))
                    f.close()
                    if PRINTECHO: print 'wrote: "'+filename+'"'
                else:
                    if PRINTECHO: print 'file unchanged: "'+filename+'"'
            else:
                f = open(filename, 'w')
                f.write(str(self))
                f.close()
                if PRINTECHO: print 'wrote: "'+filename+'"'
        else:
            import sys
            sys.stdout.write(str(self))

class TemplateDocument(StringTemplate):
    """Generate documents based on a template and a substitution mapping.

    Must use Python 1.5 or newer. Uses re and the get method on dictionaries.

    Usage:
       T = TemplateDocument('Xfile')
       T.substitutions = {'month': ObjectY, 'town': 'Scarborough'}
       T.write('Maine.html')

    A dictionary, or object that behaves like a dictionary, is assigned to the
    *substitutions* attribute which has symbols as keys to objects. Upon every
    occurance of these symbols surrounded by braces {} in the source template,
    the corresponding value is converted to a string and substituted in the output.

    For example, source text which looks like:
     I lost my heart at {town} Fair.
    becomes:
     I lost my heart at Scarborough Fair.

    Symbols in braces which do not correspond to a key in the dictionary remain
    unchanged.

    An optional third argument to the class is a list or two strings to be
    used as the delimiters instead of { } braces. They must be of the same
    length; for example ['##+', '##'] is invalid.
    """
    def set_template(self, template):
        f = open(mpath(template))
        self.source = f.read()
        f.close()

class AutoStringTemplate(StringTemplate):
    marker_begin = '<!--{%s}Begin-->'
    marker_end   = '<!--{%s}End-->'
    R = re.compile(r"<!--{(?P<key>[\w_]+)}Begin-->(?P<text>.*?)<!--{\1}End-->", re.S)
    
    def set_template(self, template):
        """Set template string and normalize by extracting comment tokens.
        """
        self.source = template
        self.extract_template()
        
    def extract_template(self, source=None):
        """Convert comment-marked regions to a regular {tokens}.
        
        Updates the substitution dictionary with the text from the region.
        """
        source = source or self.source
        a = 0
        newsubs = {}
        newtemplate = []
        d1, d2 = self.delimiters
        while 1:
            m = self.R.search(source, a)
            if m:
                start, end = m.span()
                newtemplate.append(source[a:start])
                a = end
                newsubs[m.group('key')] = m.group('text')
                newtemplate.append(d1+m.group('key')+d2)
            else:
                newtemplate.append(source[a:])
                break
        self.source = string.join(newtemplate, '')
        self.substitutions.update(newsubs)

    def _sub(self, source, subs=None):
        """Perform source text substitutions.

        *source* string containing template source text
        *subs* mapping of symbols to replacement values
        """
        substitutions = subs or self.substitutions
        dw = self.delimiter_width
        i = 0
        output = []
        matched = self.subpat.search(source[i:])
        while matched:
            a, b = matched.span()
            output.append(source[i:i+a])
            #implant comments to mark the location of the tokens
            output.append(self.marker_begin % source[i+a+dw:i+b-dw])
            # using the new get method for dicts in 1.5
            output.append(str(substitutions.get(
                   source[i+a+dw:i+b-dw], source[i+a:i+b])))
            output.append(self.marker_end % source[i+a+dw:i+b-dw])
            i = i + b
            matched = self.subpat.search(source[i:])
        else:
            output.append(source[i:])
        return string.join(output, '')

class AutoTemplateDocument(AutoStringTemplate):
    """Generate documents based on a template and a substitution mapping.
    
    The primary difference between AutoTemplateDocument and TemplateDocument
    is that the Auto version can read through an HTML file previously
    generated with this class and identify the regions of text that were
    substituted. It then extracts that text into the substitution dictionary
    and can then be updated. The intent is to eliminate the need to 
    maintain separate content files for insertion into templates. The HTML
    output file can double as a template for future use.
    Output from this class have their filled regions marked by comments:
        ...gets <!--{wz}Begin-->glued,<!--{wz}End--> in place...
    Which came from ...gets {wz} in place... in old style template syntax.
    
    AutoTemplateDocument is a functional superset of TemplateDocument and should
    be compatible.

    Usage:
       T = AutoTemplateDocument('Maine.html')
       T.substitutions = {'month': ObjectY, 'town': 'Scarborough'}
       or
       T['month'] = ObjectY ; T['town'] = 'Scarborough'
       T.write('Maine.html')
    
    A dictionary, or object that behaves like a dictionary, is assigned to the
    *substitutions* attribute which has symbols as keys to objects. Upon every
    occurance of these symbols surrounded by braces {} in the source template,
    the corresponding value is converted to a string and substituted in the output.

    For example, source text which looks like:
     I lost my heart at {town} Fair.
    becomes:
     I lost my heart at Scarborough Fair.

    Symbols in braces which do not correspond to a key in the dictionary remain
    unchanged.

    An optional third argument to the class is a list or two strings to be
    used as the delimiters instead of { } braces. They must be of the same
    length; for example ['##+', '##'] is invalid.
    """
    def set_template(self, template):
        f = open(mpath(template))
        self.source = f.read()
        f.close()


class Container:
    """A holder for a list of HTMLgen objects.    
    """

    def __init__(self, *args, **kw):
        self.contents = list(args)
        for name, value in kw.items():
            setattr(self, name, value)

    def __str__(self):
        bodystring = '%s\n' * len(self.contents)
        return bodystring % tuple(self.contents)

    def __add__(self, other):
        new = self.__class__()
        new.contents = self.contents + other.contents
        return new
        
    def append_file(self, filename, marker_function = None):
        """Add the contents of a file to the document.

        filename -- the filename of the file to be read [string]
        marker_function -- a callable object which the text read from
          the file will be passed through before being added to the
          document.
        """
        f = open(mpath(filename), 'r')
        if marker_function:
            self.append(marker_function(f.read()))
        else:
            self.append(f.read())
        f.close()
        
    def append(self, *items):
        """Add content to the Document object.
        
        Arg *items* can be plain text or objects; multiple arguments supported.
        """
        for item in items:
            self.contents.append(item)

    def prepend(self, *items):
        """Add content to the beginning of the Document object.
        
        Arg *items* can be plain text or objects; multiple arguments supported.
        """
        li = len(items)
        for i in range(li-1, 0, -1):
            self.contents.insert(0, items[i])

    def copy(self):
        """Return a complete copy of the current Container object.
        """
        return copy.deepcopy(self)

#===================
        
class Meta:
    """Set document Meta-information.

    The META element is used within the HEAD element to embed
    document meta-information not defined by other HTML elements.
    
    Keywords supported
 
        name  -- NAME element attribute (default: 'keywords')
        equiv  -- will map to the HTTP-EQUIV attribute
        content -- mandatory attribute (default: 'python,HTMLgen')
        url -- URL naturally
    
    Example:

        Meta( name='keywords', content='eggs,spam,beans' )
    """
    def __init__(self, **kw):
        self.equiv = 'keywords'
        self.name  = ''
        self.content = 'python,HTMLgen'
        self.url = ''
        for item in kw.keys():
            self.__dict__[item] = kw[item]

    def __str__(self):
        s = ['<META']
        if self.equiv: s.append(' HTTP-EQUIV="%s"' % self.equiv)
        if self.name:  s.append(' NAME="%s"' % self.name)
        if self.content: s.append(' CONTENT="%s"' % self.content)
        if self.url: s.append(' URL="%s"' % self.url)
        s.append('>\n')
        return string.join(s, '')

##### Client-side Imagemap Support #####

class Map:
    """Used to name and describe a client-side image map.

    The *areas* argument is a list of Area objects.
    Keyword arg is supported for *name*, which defines the map name
    to be used with the usemap attribute of an Image class instance.
    """
    def __init__(self, areas = None, **kw):
        self.areas = areas or []
        self.name = ''
        for item in kw.keys():
            self.__dict__[item] = kw[item]

    def __str__(self):
        s = ['\n<MAP NAME="%s">\n' % self.name]
        for area in self.areas:
            s.append(str(area))
        s.append('</MAP>\n')
        return string.join(s, '')


class Href:
    """Generate a hyperlink.

    Argument 1 is the URL and argument 2 is the hyperlink text.

    Keyword arguments

        target -- is an optional target symbol 
        onClick --  is the script-code which is executed when link is clicked.
        onMouseOver -- the script-code which is executed when the mouse
                       moves over the link.
        onMouseOut -- the script-code which is executed when the mouse
                       moves off the link.
    """
    def __init__(self, url='', text='', **kw):
        self.target = None
        self.onClick = None
        self.onMouseOver = None
        self.onMouseOut = None
        self.url = url
        self.text = text
        for item in kw.keys():
            if self.__dict__.has_key(item):
                self.__dict__[item] = kw[item]
            else:
                raise KeyError, `item`+' not a valid parameter for this class.'

    def __str__(self):
        s = ['<A HREF="%s"' % self.url]
        if self.target: s.append(' TARGET="%s"' % self.target)
        if self.onClick: s.append(' onClick="%s"' % self.onClick)
        if self.onMouseOver: s.append(' onMouseOver="%s"' % self.onMouseOver)
        if self.onMouseOut: s.append(' onMouseOut="%s"' % self.onMouseOut)
        s.append('>%s</A>' % self.text)
        return string.join(s, '')

    def append(self, content):
        self.text = self.text + str(content)
        

A = HREF = Href # alias

class Name(Href):
    """Generate a named anchor.

    Arg *url* is a string or URL object,
    Arg *text* is optional string or object to be highlighted as the anchor.
    """
    def __str__(self):
        return '<A NAME="%s">%s</A>' % (self.url, self.text)

NAME = Name # alias

class MailTo:
    """A Mailto href

    First argument is an email address, optional second argument is
    the text shown as the underlined hyperlink. Default is the email
    address. Optional third argument is a Subject: for the email.
    """
    def __init__(self, address='', text=None, subject=None):
        self.address = address
        self.text = text or address
        self.subject = subject

    def __str__(self):
        if self.subject: self.address = "%s?subject=%s" % (self.address, self.subject)
        return '<A HREF="mailto:%s">%s</A>' % (self.antispam(self.address), self.text)

    def antispam(self, address):
        """Process a string with HTML encodings to defeat address spiders.
        """
        from whrandom import choice
        buffer = map(None, address)
        for i in range(0, len(address), choice((2,3,4))):
            buffer[i] = '&#%d;' % ord(buffer[i])
        return string.join(buffer,'')

MAILTO = Mailto = MailTo # aliases

class P:
    """Just echo a <P> tag."""
    def __str__(self):
        return '\n<P>\n'

# List constructs

class List(UserList.UserList):
    """Will generate a bulleted list given a list argument.

    Now supports rendering a list into multiple columns by setting the
    *columns* attribute to a number greater than one. This is
    implemented using tables and you can also set a background color
    for the list itself by using the *bgcolor* attribute.

    Supports nested lists, i.e. lists of lists. Each time a list is
    encountered in a list it will indent those contents w.r.t. the
    prior list entry. This can continue indefinitely through nested
    lists although there are only three different bullets provided by
    the browser (typically).
    
    Optional keyword *indent* can be used to indicate whether you want
    the list to start left justified or indented. *indent=0* will make
    it left justified. The default is to indent.

    Optional keyword *type* can be set to either disk, circle, or
    square to specify what kind of symbol is used for each list item's
    bullet. (Netscape extension)
    
    Since we inherit from the UserList class any normal list
    operations work on instances of this class.  Any list contents
    will do. Each of the items will be emitted in html if they are
    themselves objects from this module.
    Aliases: UL, BulletList
    """

    I_am_a_list = 1
    tagname = 'UL'
    attrs = ('type','align','class','id','style')
    flags = ('compact',)
    columns = 1
    bgcolor = ''
    pad = '    '
    indent = 1
    def __init__(self, list = None, **kw):
        self.data = []
        self.lvl = 0
        if list:
            if type(list) == type(self.data):
                self.data[:] = list
            else:
                self.data[:] = list.data[:]
        for item in kw.keys():
            self.__dict__[string.lower(item)] = kw[item]

    def __getslice__(self, i, j):
        newlist = copy.copy(self)
        newlist.data = self.data[i:j]
        newlist.columns = 1 # don't forget that the copy will carry
                            # the old attribute value. set to 1
        return newlist

    def multi_column_table(self):
        """Return a table containing the list sliced into columns.
        """
        slices = self.column_slices(self.columns)
        table = TableLite(border=0, cellpadding=3)
        if self.bgcolor: table.bgcolor = self.bgcolor
        for begin, end in slices:
            column = TD(self[begin:end], valign='top', html_escape='no')
            table.append(column)
        return table

    def column_slices(self, columns=1):
        """Calculate a list of index pairs bounding column slices.
        """
        list_len = len(self.data)
        col_len, remainder = divmod(list_len, columns)
        if remainder: col_len = col_len + 1
        indexpairs = []
        if columns > 1:
            for column in range(columns):
                col_end = (column+1)*col_len
                if col_end < list_len:
                    indexpairs.append((column*col_len, col_end))
                else:
                    indexpairs.append((column*col_len, list_len))
        else:
            indexpairs.append((0, list_len))
        return indexpairs
        
    def __str__(self):
        if self.columns > 1: # go to the new multicolumn feature
            return str(self.multi_column_table())
        # same as before
        self.firstitem = 1
        self.s = []
        if self.indent:
            self.s.append(self.pad*self.lvl + self.start_element())
        for item in self.data: #start processing main list
            itemtype = type(item)
            if itemtype == InstanceType: 
                try: # in case it's a nested list object
                    if item.I_am_a_list:
                        itemtype = ListType
                except AttributeError:
                    pass
            if itemtype == ListType: #process the sub list
                self.sub_list(item)
            else:
                self.s.append(self.render_list_item(item))

        if self.indent: #close out this level of list
            self.s.append(self.pad*self.lvl + self.end_element())
        self.lvl = 0
        return string.join(self.s, '')

    def sub_list(self, list):
        """Recursive method for generating a subordinate list
        """
        self.lvl = self.lvl + 1
        if type(list) == InstanceType:
            try:
                if list.I_am_a_list: #render the List object
                    list.lvl = self.lvl
                    self.s.append(str(list))
            except AttributeError:
                pass
        else:
            self.s.append(self.pad*self.lvl + self.start_element())
            for item in list:
                itemtype = type(item)
                if itemtype == InstanceType:
                    try: #could be another nested List child object
                        if item.I_am_a_list:
                            itemtype = ListType
                    except AttributeError:
                        pass
                if itemtype == ListType:
                    self.sub_list(item) #recurse for sub lists
                else: # or just render it
                    self.s.append(self.render_list_item(item))
            # close out this list level
            self.s.append(self.pad*self.lvl + self.end_element())
        self.lvl = self.lvl - 1 #decrement indentation level

    def render_list_item(self, item):
        """Renders the individual list items

        Overloaded by child classes to represent other list styles.
        """
        return '%s<LI>%s\n' % (self.pad*self.lvl, item)

    def start_element(self):
        """Generic creator for the HTML element opening tag.

        Reads tagname, attrs and flags to return appropriate tag.
        """
        s = ['<' + self.tagname]
        for attr in self.attrs:
            try:
                s.append(' %s="%s"' % (attr, getattr(self, attr)))
            except AttributeError:
                pass
        for flag in self.flags:
            try:
                x = getattr(self, flag)
                s.append(' %s' % flag)
            except AttributeError:
                pass
        s.append('>\n')
        return string.join(s, '')

    def end_element(self):
        """Closes the HTML element
        """
        return '</%s>\n' % self.tagname

    def append(self, *items):
        """Append entries to the end of the list
        """
        for item in items:
            self.data.append(item)
    
UL = BulletList = List  #Aliases

class OrderedList(List):
    """Will generate a numbered list given a list arg.

    Optional keyword *type* can be used to specify whether you want
    the list items marked with: capital letters (type='A'), small
    letters (type='a'), large Roman numerals (type='I'), small Roman
    numerals (type='i'). The default is arabic numbers. The other
    types are HTML3.2 only and may not be supported by browsers yet.
    Any list contents will do. Each of the items will be emitted
    in HTML if they are themselves objects.
    """
    tagname = 'OL'
    attrs = ('type','class','id','style')

OL = NumberedList = OrderedList

class DefinitionList(List):
    """Show a series of items and item definitions.

    Arg is a list of tuple pairs:
    [(string/object,string/object),(,)...]  1st item in each pair is
    the word to be defined. It will be rendered in bold. 2nd is the
    string which will be indented to it's next-line-right. If the
    *compact* flag is set to non-empty, the definition side will be
    placed on the same line.  Example

        DefinitionList([( 4 , 'Number after 3') , ( 1 , 'Unity')] ) will emit:
        4
            Number after 3
        1
            Unity
    """
    tagname = 'DL'
    attrs = ('class','id','style')
    flags = ('compact',)
    def render_list_item(self, item):
        """Overload method to perform DT/DD markup.
        """
        return '%s<DT><B>%s</B><DD>%s\n' % (self.pad*self.lvl, item[0], item[1])

DL = DefinitionList

class ImageBulletList(List):
    """Show a list of images with adjoining text(or object).

    Arg is a list of tuple pairs: [(Image_obj, string/object),(,)...]
    Generates an inlined image then the text followed by a <BR>
    for each element.
    """
    tagname = 'UL'
    attrs = ()
    flags = ()
    def render_list_item(self, item):
        """Overload method to take first item from an item tuple and
        setting it next to the second item, using BR to separate list items.
        """
        return '%s%s %s<BR>\n' % (self.pad*self.lvl, item[0], item[1])

class NonBulletList(List):
    """Generate a raw indented list without bullet symbols.

    Arg is a list of python objects:
    """
    tagname = 'UL'
    attrs = ()
    flags = ()
    def render_list_item(self, item):
        """Overload method to take first item from an item tuple and
        setting it next to the second item, using BR to separate list items.
        """
        return '%s%s<BR>\n' % (self.pad*self.lvl, item)


####### FORM TAGS ########
class Form:
    """Define a user filled form. Uses POST method.
   
    *cgi* is the URL to the CGI processing program.  Input objects
    (any others as well) are appended to this container widget.
    
    Keywords
    
        name -- name of the form
        submit -- The Input object to be used as the submit button.
                  If none specified a Submit button will automatically
                  be appended to the form. Do not manually append your
                  submit button. HTMLgen will append it for you.
        reset  -- Input object to be used as a reset button.
        target -- set a TARGET attribute
        enctype -- specify an Encoding type.
        onSubmit -- script, which is executed, when the form is submitted
    """
    def __init__(self, cgi = None, **kw):
        self.contents = []
        self.cgi = cgi
        self.submit = Input(type='submit', name='SubmitButton', value='Send')
        self.reset = None
        self.target = None
        self.enctype = None
        self.name = None
        self.onSubmit = ''
        overlay_values(self, kw)

    def append(self, *items):
        """Append any number of items to the form container.
        """
        for item in items:
            self.contents.append(str(item))

    def __str__(self):
        s = ['\n<FORM METHOD="POST"']
        if self.cgi: s.append(' ACTION="%s"' % self.cgi)
        if self.enctype: s.append(' ENCTYPE="%s"' % self.enctype)
        if self.target: s.append(' TARGET="%s"' % self.target)
        if self.name: s.append(' NAME="%s"' % self.name)
        if self.onSubmit: s.append(' onSubmit="%s"' % self.onSubmit)
        s.append('>\n')
        s = s + self.contents
        s.append(str(self.submit))
        if self.reset: s.append(str(self.reset))
        s.append('\n</FORM>\n')
        return string.join(s, '')
        
        
def overlay_values(obj, dict):
    """Adds each item from dict to the given object iff there already
    exists such a key. Raises KeyError if you try to update the value
    of non-existing keys.
    """
    for key in dict.keys():
        if hasattr(obj, key):
            obj.__dict__[key] = dict[key]
        else:
            raise KeyError(`key` + ' not a keyword for ' + obj.__class__.__name__)


class Input:
    """General Form Input tags.

    Keyword Arguments

        type -- 'TEXT' (default) Supported types include password, checkbox,
                      radio, file, submit, reset, hidden.
        name -- provides the datum name
        value -- the initial value of the input item
        checked --  flag indicating if the item is checked initially
        size -- size of the widget (e.g. size=10 for a text widget is it's width)
        maxlength -- maximum number of characters accepted by the textfield.
        border -- border width in pixels for an image type.
        align -- top|middle|bottom align w.r.t. adjoining text for image types.
        llabel  --  an optional string set to the left of the widget
        rlabel  --  an optional string set to the right of the widget
        onBlur -- script, which is executed, when the field loses focus,
                  useful for the text-type 
        onChange -- script, which is executed, when the field value changed,
                    useful for the text-type
        onClick -- script, which is executed, when the field in clicked,
                   useful for the button, checkbox, radio, submit, reset type
        onFocus -- script, which is executed, when the field receives focus,
                   useful for the text-type
        onSelect -- script, which is executed, when part of the field 
                    is selected, useful for the text-type
    """
    re_type = re.compile('text|password|checkbox|radio|image|button|file|submit|reset|hidden',
                            re.IGNORECASE)
    def __init__(self, **kw):
        self.type = 'TEXT'
        self.name = 'Default_Name'
        self.value = None
        self.checked = ''
        self.size = 0
        self.maxlength = 0
        self.llabel = ''
        self.rlabel = ''
        self.onBlur = ''
        self.onChange = ''
        self.onClick = ''
        self.onFocus = ''
        self.onSelect = ''
        self.border = None
        self.align = ''
        for item in kw.keys():
            if self.__dict__.has_key(item):
                self.__dict__[item] = kw[item]
            else:
                raise KeyError, `item`+' not a valid parameter of the Input class.'
        if Input.re_type.search(self.type) is None:
            raise KeyError, `self.type`+' not a valid type of Input class.'

    def __str__(self):
        s = []
        if self.llabel: s.append(str(self.llabel))
        s.append('\n<INPUT')
        if self.type: s.append(' TYPE="%s"' % self.type)
        if self.name: s.append(' NAME="%s"' % self.name)
        if self.value is not None: s.append(' VALUE="%s"' % self.value)
        if self.checked: s.append(' CHECKED')
        if self.size: s.append(' SIZE=%s' % self.size)
        if self.maxlength: s.append(' MAXLENGTH=%s' % self.maxlength)
        if self.onBlur: s.append(' onBlur="%s"' % self.onBlur)
        if self.onChange: s.append(' onChange="%s"' % self.onChange)
        if self.onClick: s.append(' onClick="%s"' % self.onClick)
        if self.onFocus: s.append(' onFocus="%s"' % self.onFocus)
        if self.onSelect: s.append(' onSelect="%s"' % self.onSelect)
        if self.border is not None: s.append(' BORDER="%s"' % self.border)
        if self.align: s.append(' ALIGN="%s"' % self.align)
        s.append('>')
        if self.rlabel: s.append(str(self.rlabel))
        return string.join(s, '')


class Select(UserList.UserList):
    """Used to define a list widget or option widget.
    
    Pass a list of strings to show a list with those values. Alternatively
    can pass a list of tuple pairs. Each pair contains the displayed string
    and it's associatated value mapping. If no value mapping is needed just
    use something that evaluates to None.

    Keyword Arguments:
    
        name -- provides the datum name
        size -- the visual size. 1 means use an option popup widget. 
                               >=2 means use a list widget with that many lines.
        multiple -- flag to indicate whether multiple selections are supported.
        selected -- list of values to be shown as pre-selected.
        onBlur -- script, which is executed, when the field loses focus
        onChange -- script, which is executed, when the field value changed
        onFocus -- script, which is executed, when the field receives focus
    """
    def __init__(self, data=None, **kw):
        UserList.UserList.__init__(self, data)
        self.name = ''
        self.size = 1
        self.multiple = None
        self.selected = []
        self.onBlur = ''
        self.onChange = ''
        self.onFocus = ''
        for item in kw.keys():
            if self.__dict__.has_key(item):
                self.__dict__[item] = kw[item]
            else:
                raise KeyError, `item`+' not a valid parameter of the Select class.'

    def __str__(self):
        s = ['<SELECT NAME="%s"' % self.name]
        if self.size: s.append(' SIZE=%s' % self.size)
        if self.multiple: s.append(' MULTIPLE')
        if self.onBlur: s.append(' onBlur="%s"' % self.onBlur)
        if self.onChange: s.append(' onChange="%s"' % self.onChange)
        if self.onFocus: s.append(' onFocus="%s"' % self.onFocus)
        s.append('>\n')
        if type(self.data[0]) is TupleType:
            for item, value in self.data:
                s.append('<OPTION')
                if value is not None:
                    s.append(' Value="%s"' % value)
                if value in self.selected:
                    s.append(' SELECTED')
                else:
                    if item in self.selected:
                        s.append(' SELECTED')
                s.append('>%s\n' % item)
        else:
            for item in self.data:
                if item not in self.selected:
                    s.append('<OPTION>%s\n' % item)
                else:
                    s.append('<OPTION SELECTED>%s\n' % item)
        s.append('</SELECT>\n')
        return string.join(s, '')

class Textarea:
    """Used for an entry widget to type multi-line text (for forms).

    Keyword Arguments:

        rows -- sets the number of text rows. (default=4)
        cols -- sets the number of text columns. (default=40)
        onBlur -- script, which is executed, when the field loses focus
        onChange -- script, which is executed, when the field value changed
        onFocus -- script, which is executed, when the field receives focus
        onSelect -- script, which is executed, when part of the field 
                    is selected
    """
    def __init__(self, text='', **kw):
        self.text = text
        self.name = 'text_area'
        self.rows = 4
        self.cols = 40
        self.onBlur = ''
        self.onChange = ''
        self.onFocus = ''
        self.onSelect = ''
        for item in kw.keys():
            if self.__dict__.has_key(item):
                self.__dict__[item] = kw[item]
            else:
                raise KeyError, `item`+' not a valid parameter of the Textarea class.'

    def __str__(self):
        s = ['<TEXTAREA NAME="%s" ROWS=%s COLS=%s' % (self.name, self.rows, self.cols)]
        if self.onBlur: s.append(' onBlur="%s"' % self.onBlur)
        if self.onChange: s.append(' onChange="%s"' % self.onChange)
        if self.onFocus: s.append(' onFocus="%s"' % self.onFocus)
        if self.onSelect: s.append(' onSelect="%s"' % self.onSelect)
        s.append('>')
        s.append(str(self.text))
        s.append('</TEXTAREA>')
        return string.join(s, '')

class Script:
    """Construct a Script

    Keyword Arguments

        Defaults in (parenthesis).  Keyword parameters may be set as attributes of 
        the instantiated script object as well.

        language -- specifies the language ('JavaScript')
        src -- specifies the location
        code -- script code, which is printed in comments, to hide it from non
                java-script browsers
    """
    def __init__(self, **kw):
        # Specify the default values
        self.language = 'JavaScript'
        self.src = ''
        self.code = ''
        # Now overlay the keyword arguments from caller
        for k in kw.keys():
            if self.__dict__.has_key(k):
                self.__dict__[k] = kw[k]
            else:
                print `k`, "isn't a valid parameter for this class."

    def __str__(self):
        s = ['<SCRIPT LANGUAGE="%s" ' % self.language]
        if self.src: s.append('SRC="%s" ' % self.src)
        s.append('>')
        if self.code: s.append('<!--\n%s\n//-->\n' % self.code)
        s.append('</SCRIPT>')
        return string.join(s, '')
    
    def append(self, s):
        self.code = self.code + s
  

####################


class Table:
    """Construct a Table with Python lists.

    Instantiate with a string argument for the table's name (caption).
    Set object.heading to a list of strings representing the column headings.
    Set object.body to a list of lists representing rows. **WARNING:** the body
    attribute will be edited to conform to html. If you don't want your
    data changed make a copy of this list and use that with the table object.

    Keyword Parameters

        Defaults in (parenthesis).  Keyword parameters may be set as attributes of the
        instantiated table object as well.

        caption_align -- 'top'|'bottom'  specifies the location of the table title ('top')
        border -- the width in pixels of the bevel effect around the table (2)
        cell_padding -- the distance between cell text and the cell boundary (4)
        cell_spacing -- the width of the cell borders themselves (1)
        width -- the width of the entire table wrt the current window width ('100%')
        colspan -- a list specifying the number of columns spanned by that heading
               index. e.g. t.colspan = [2,2] will place 2 headings spanning
               2 columns each (assuming the body has 4 columns).
        heading --  list of strings, the length of which determine the number of
                   columns.  ( ['&nbsp']*3 )
        heading_align -- 'center'|'left'|'right'
                        horizontally align text in the header row ('center')
        heading_valign --  'middle' |'top'|'bottom'
                        vertically align text in the header row ('middle')
        body_color -- a list of colors, for each column (None)
        heading_color -- a list of color for each column heading (None)
             For both these the index used is i%len(..._color) so
             the color cycles through the columns
        body -- a list of lists in row major order containing strings or objects
               to populate the body of the table. ( [['&nbsp']*3] )
        column1_align -- 'left'|'right'|'center'  text alignment of the first column
        cell_align --    'left'|'right'|'center'  text alignment for all other cells
        cell_line_breaks -- 1|0  flag to determine if newline char in body text will be
                  converted to <br> symbols; 1 they will, 0 they won't. (1)
        
    """
    def __init__(self, tabletitle='', **kw):
        """Arg1 is a string title for the table caption, optional keyword
        arguments follow.
        """
        # Specify the default values
        self.tabletitle = tabletitle
        self.caption_align = 'top'
        self.border = 2
        self.cell_padding = 4
        self.cell_spacing = 1
        self.width = '100%'
        self.heading = None
        self.heading_align = 'center'
        self.heading_valign = 'middle'
        self.body = [['&nbsp;']*3]
        self.column1_align = 'left'
        self.cell_align = 'left'
        self.cell_line_breaks = 1
        self.colspan = None
        self.body_color= None
        self.heading_color=None
        # Now overlay the keyword arguments from caller
        for k in kw.keys():
            if self.__dict__.has_key(k):
                self.__dict__[k] = kw[k]
            else:
                print `k`, "isn't a valid parameter for this class."

    def __str__(self):
        """Generates the html for the entire table.
        """
        if self.tabletitle:
           s = [str(Name(self.tabletitle)) + '\n<P>']
        else:
           s = []

        s.append('<TABLE border=%s cellpadding=%s cellspacing=%s width="%s">\n' % \
                (self.border, self.cell_padding, self.cell_spacing, self.width))
        if self.tabletitle:
            s.append('<CAPTION align=%s><STRONG>%s</STRONG></CAPTION>\n' % \
                    (self.caption_align, self.tabletitle))

        for i in range(len(self.body)):
            for j in range(len(self.body[i])):
                if type(self.body[i][j]) == StringType:
                    #process cell contents to insert breaks for \n char.
                    if self.cell_line_breaks:
                        self.body[i][j] = string.replace(self.body[i][j], '\n','<br>')
                    else:
                        self.body[i][j] = Text(self.body[i][j])

        # Initialize colspan property to 1 for each
        # heading column if user doesn't provide it.
        if self.heading:
            if not self.colspan:
                if type(self.heading[0]) == ListType:
                    self.colspan = [1]*len(self.heading[0])
                else:
                    self.colspan = [1]*len(self.heading)
        # Construct heading spec
        #  can handle multi-row headings. colspan is a list specifying how many
        #  columns the i-th element should span. Spanning only applies to the first
        #  or only heading line.
        if self.heading:
            prefix = '<TR Align=' + self.heading_align + '> '
            postfix = '</TR>\n'
            middle = ''
            if type(self.heading[0]) == ListType:
                for i in range(len(self.heading[0])):
                    middle = middle + '<TH ColSpan=%s%s>' % \
                             (self.colspan[i], \
                              self.get_body_color(self.heading_color,i)) \
                              + str(self.heading[0][i]) +'</TH>'
                s.append(prefix + middle + postfix)
                for i in range(len(self.heading[1])):
                    middle = middle + '<TH>' + str(self.heading[i]) +'</TH>'
                for heading_row in self.heading[1:]:
                    for i in range(len(self.heading[1])):
                        middle = middle + '<TH>' + heading_row[i] +'</TH>'
                    s.append(prefix + middle + postfix)
            else:
                for i in range(len(self.heading)):
                    middle = middle + '<TH ColSpan=%s%s>' % \
                             (self.colspan[i], \
                              self.get_body_color(self.heading_color,i)) \
                              + str(self.heading[i]) +'</TH>'
                s.append(prefix + middle + postfix)
        # construct the rows themselves
        stmp = '<TD Align=%s %s>'
        for row in self.body:
            s.append('<TR>')
            for i in range(len(row)):
                if i == 0 :
                    ss1 = self.column1_align
                else:
                    ss1 = self.cell_align
                s.append(stmp % (ss1, self.get_body_color(self.body_color,i)))
                s.append(str(row[i]))
                s.append('</TD>\n')
            s.append('</TR>\n')
        #close table
        s.append('</TABLE><P>\n')
        return string.join(s, '')
    
    def get_body_color(self, colors, i):
        """Return bgcolor argument for column number i
        """
        if colors is not None: 
            try: 
                index = i % len(colors) 
                return ' bgcolor="%s"' % colors[index] 
            except: 
                pass 
        return ''


#--------------------New stuff--------------------------#

def _make_attr_inits(opts):
    """Construct a format string and dictionary suitable for value
    substitution of tag attributes.
    """
    a = []
    d = {}
    for name in opts:
        a.append('%('+name+')s')
        d[name] = ''
    return string.join(a, ''), d  

class AbstractTagSingle:
    "Abstract base class for all tag markup classes not requiring a closure tag."
    tagname = '' # to be provided by derived classes
    attrs = ()   # to be provided by derived classes
    attr_template = '' # to be provided by derived classes
    attr_dict = {}     # to be provided by derived classes
    
    def __init__(self, *args, **kw):
        self.__dict__['attr_dict'] = copy.copy(self.__class__.attr_dict)
        self.args = args
        for name, value in kw.items():
            name = string.lower(name)
            setattr(self, name, value)

    def __str__(self):
        """Generate an HTML formatted string for this object.
        """
        return  '<%s' % self.tagname + self.attr_template % self.attr_dict + '>'
        
    def __setattr__(self, name, value):
        """Intercept attribute assignments.

        If the attribute is a legal HTML tag attribute add it to the
        dict used for substitution in __str__, otherwise just set it as
        an instance attribute.
        """
        name = string.lower(name)
        if name in self.attrs:
            self.attr_dict[name] = ' %s="%s"' % (name, value)
        self.__dict__[name] = value

class Image(AbstractTagSingle):
    """Inlined Image

    The *filename* argument is a filename, or URL of a graphic image,
    or a triple of ( filename, width, height ) where dimensions are in
    pixels. Where the filename is found to be a valid pathname to an
    existing graphic file that file will be read to determine its width and
    height properties. GIF, JPEG, and PNG files are understood.
    
    Keyword Arguments
    
        width  -- (int) Width in pixels
        height -- (int) Height in pixels
        border -- (int) Border width in pixels
        align  -- (string) 'top'|'middle'|'bottom'|'right'|'left'
        alt    -- (string) Text to substitute for the image in nonGUI browsers
        usemap -- Imagemap name or Map object
        ismap  -- Flag (1|0) indicating if a server side imagemap is available.
        absolute -- Absolute path to the directory containing the image
        prefix -- Relative path or URL to directory containing the image
        hspace -- Number of pixels to be added to the left and right of the image.
        vspace -- Number of pixels to be added to the top and bottom of the image.
        name -- A name for this image to be used by JavaScript
        Class -- A CSS class this tag belongs to.
        style -- A CSS inline style specification.
    """
    tagname = 'IMG'
    attrs = ('src', 'height', 'width', 'alt', 'border', 'align', 'class','id',
             'hspace','vspace', 'lowsrc', 'name', 'style', 'usemap', 'ismap' )
    attr_template , attr_dict = _make_attr_inits(attrs)

    def __init__(self, *args, **kw):
        apply(AbstractTagSingle.__init__, (self,) + args, kw)
        self.prefix = None
        self.absolute = None
        if self.args:
            self.process_arg(self.args[0])

    def process_arg(self, arg):
        # unpack the tuple if needed
        if type(arg) == TupleType:
            self.filename = arg[0]
            self.width = arg[1]
            self.height = arg[2]
        else:
            self.filename = arg
        self.src = self.filename
        # if the file is there test it to get size
        if not self.attr_dict['width']: # assume if the user has set the width property
            # she knows the image size already or wants to resize it.
            try:
                self.width, self.height = imgsize(self.filename)
            except IOError:
                pass

    def calc_rel_path(self, from_dir=None):
        """Calculate the relative path from 'from_dir' to the
        absolute location of the image file.

        Sets self.prefix.
        """
        if not from_dir:
            from_dir = os.getcwd()
        if self.absolute:
            self.prefix = relpath(from_dir, self.absolute)

    def __str__(self):
        if self.prefix:
            self.src = os.path.join(self.prefix, self.filename)
        if not self.attr_dict['alt']:
            self.alt = os.path.basename(self.filename)
        if self.attr_dict['usemap']:
            if type(self.attr_dict['usemap']) == InstanceType:
                # can use a Map instance for this
                try:
                    self.usemap = '#' + self.usemap.name
                except:
                    pass
        return AbstractTagSingle.__str__(self)

IMG = Image # alias

class BR(AbstractTagSingle):
    """Break tag. Argument is an integer integer multiplier. BR(2)=='<BR><BR>'
    """
    tagname = 'BR'
    attrs = ('clear',)
    attr_template , attr_dict = _make_attr_inits(attrs)
    
    def __str__(self):
        s = AbstractTagSingle.__str__(self)
        if self.args and type(self.args[0]) is IntType:
            return s*self.args[0]
        else:
            return s

class Base(AbstractTagSingle):
    """Specify the base URL for all relative URLs in this document.

    One string argument required. It must be a complete file name, and
    is usually the original URL of this document.  If this file is
    moved, having the BASE set to the original URL eliminates the need
    to also move all the documents which are identified by relative
    URL links in this document.
    """
    tagname = 'BASE'
    attrs = ('href', 'target')
    attr_template , attr_dict = _make_attr_inits(attrs)

class BaseFont(AbstractTagSingle):
    """Specify the font size for subsequent text.
    """
    tagname = 'BASEFONT'
    attrs = ('color', 'name', 'size')
    attr_template , attr_dict = _make_attr_inits(attrs)

class Embed(AbstractTagSingle):
    """Embed an application in this document.
    """
    tagname = 'EMBED'
    attrs = ('align', 'border', 'height', 'hidden', 'hspace',
             'name', 'palette', 'pluginspage', 'src', 'type',
             'units', 'vspace', 'width')
    attr_template , attr_dict = _make_attr_inits(attrs)

class HR(AbstractTagSingle):
    """Break the current text flow and insert a horizontal rule.
    """
    tagname = 'HR'
    attrs = ('align', 'class','id', 'color', 'noshade', 'size',
             'style', 'width')
    attr_template , attr_dict = _make_attr_inits(attrs)


class AbstractTag:
    "Abstract base class for all tag markup classes requiring a closure tag."
    tagname = '' # to be provided by derived classes
    attrs = ()   # to be provided by derived classes
    attr_template = '' # to be provided by derived classes
    attr_dict = {}      # to be provided by derived classes
    html_escape = 'ON'
    trailer = '\n'
    
    def __init__(self, *contents, **kw):
        self.__dict__['contents'] = []
        self.__dict__['attr_dict'] = copy.copy(self.__class__.attr_dict)
        for item in contents:
            self.contents.append(item)
        for name, value in kw.items():
            name = string.lower(name)
            setattr(self, name, value)

    def __str__(self):
        """Generate an HTML formatted string for this object.
        """
        s = ['<%s' % self.tagname]  # tag opening
        s.append(self.attr_template % self.attr_dict + '>') # options
        # crunch through the contents
        for item in self.contents:
            if type(item) is StringType and self.html_escape == 'ON':
                s.append(escape(item))
            else:
                s.append(str(item))
        # close out the marked region
        s.append( '</%s>' % self.tagname)
        return string.join(s, '') + self.trailer
        
    def __setattr__(self, name, value):
        """Intercept attribute assignments.

        If the attribute is a legal HTML tag attribute add it to the
        dict used for substitution in __str__, otherwise just set it as
        an instance attribute.
        """
        name = string.lower(name)
        if name in self.attrs:
            self.attr_dict[name] = ' %s="%s"' % (name, value)
        else:
            self.__dict__[name] = value

    def __call__(self, text):
        """Enable instances to be callable as text processing functions.

        For Example:

          >>> S = HTMLgen.Strong()
          >>> print S('Hi!')
          >>> <STRONG>Hi!</STRONG>
        """
        self.__dict__['contents'] = [text]
        return str(self)

    def __add__(self, other):
        """Support self + list
        """
        if type(other) is ListType:
            self.contents = self.contents + other
            return self
        else:
            raise TypeError, 'can only add lists to this object'
            
    def append(self, *items):
        """Append one or more items to the end of the container.
        """
        for item in items:
            self.contents.append(item)

    def prepend(self, *items):
        """Prepend one or more items to the top of the container.
        """
        for item in items:
            self.contents.insert(0, item)

    def empty(self):
        """Empty the contents of the container.
        """
        self.contents = []

    def __len__(self):
        """Return the integer length of the container list.
        """
        return len(self.contents)

    def last(self):
        """Return a reference to the last item in the container.
        """
        return self.contents[-1]

    def copy(self):
        """Return a full copy of the object.
        """
        return copy.deepcopy(self)

    def markup(self, rex=None, marker=None, **kw):
        """Markup the contained text matching a regular expression with
        a tag class instance or function. 

        Arguments

            rex -- a regular expression object or pattern which will be used
                to match all text patterns in the Paragraph body. Must have a single
                group defined. Group 1 is the matching text that will be marked.
                Default to all parenthetical text.
            marker -- an HTMLgen class instance to which the found text will
                be sent for wrapping (using its __call__ method). Default is Emphasis.

        Keywords
        
            collapse -- When set to 1 removes the non-grouped matching text
                from the output. Default 0.

        Returns the number of matching text groups.
        """
        collapse = 0
        if kw.has_key('collapse'): collapse = kw['collapse']
        text = string.join(map(str, self.contents))
        newtext, count = markup_re(text, rex, marker, collapse)
        if count:
            self.contents = [newtext]
            self.html_escape = 'OFF'
        return count


class Area(AbstractTagSingle):
    """Specify a click-sensitive area of an image.

    The area is linked to a HREF specified by the *href* attribute.
    The *coords* attribute is required and describes the position of
    an area (in pixels) of the image in comma-separated x,y
    coordinates where the upper-left corner is "0,0". For shape='rect'
    (the default), it is "left,top,right,bottom". For shape='circle',
    it is "center_x,center_y,radius". For shape='polygon', it is
    successive x,y vertices of the polygon. If the first and last
    coordinates are not the same, then a segment is inferred to close
    the polygon. If no *href* keyword is given a *NOHREF* will be
    generated indicating that this region should generate no links.
    
    Keyword Arguments
    
        href --  Typically a reference to an image
        coords --  string holding a list of coordinates defining
        shape  -- 'rect'|'circle'|'polygon'
    """
    tagname = 'AREA'
    attrs = ('alt','class','coords','href','id','name',
	     'onmouseout','onmouseover','shape','target')
    attr_template, attr_dict = _make_attr_inits(attrs)
    attr_dict['href'] = ' nohref'

###### FRAME SUPPORT ######

class Frameset(AbstractTag):
    """Define a Frameset to contain Frames or more Framesets"""
    tagname = 'FRAMESET'
    attrs = ('border','bordercolor','cols','frameborder','framespacing','onblur',
	     'onfocus','onload','onunload','rows')
    attr_template, attr_dict = _make_attr_inits(attrs)

class NoFrames(AbstractTag):
    """Issue a message on browsers that don't support frames"""
    tagname = 'NOFRAMES'
    attrs = ()
    attr_template, attr_dict = _make_attr_inits(attrs)

    def __init__(self, *contents, **kw):
	AbstractTag.__init__(self)
	for content in contents: self.append(content)
	for name, value in kw.items(): self.__setattr__(name,value)
	if len(contents) == 0:
	    self.append(Heading(2,'Frame ALERT!',align='center'),
			Para("""This document is designed to be viewed using Netscape's
			Frame features.  If you are seeing this message, you are using
			a frame challenged browser."""),
			Para('A ',Strong('Frame-capable'),' browser can be retrieved from',
			     Href('http://home.netscape.com/','Netscape Communications'),
			     ' or ',
			     Href('http://www.microsoft.com/','Microsoft')))

class Frame(AbstractTag):
    """Define the characteristics of an individual frame.

    Keywords Arguments

        src  -- is a HREF which points to the initial contents of the frame.
        name -- is the window name used by others to direct content into this frame.
        marginwidth -- is the number of pixels used to pad the left and right
               sides of the frame.
        marginheight -- is the number of pixels used to pad the top and bottom
               sides of the frame.
        scrolling -- is used to indicate scrolling policy set to 'yes'|'no'|'auto'
        noresize -- is a flag which instructs the browser to disallow frame resizing. 
               set to non zero lock size ( noresize=1 ).
    """
    
    tagname = 'FRAME'
    attrs = ('align','bordercolor','frameborder','marginheight','marginwidth','name',
	     'noresize','scrolling','src')
    attr_template, attr_dict = _make_attr_inits(attrs)



class Paragraph(AbstractTag):
    """Define a Paragraph.

    Takes a single string/object argument and the optional
    keyword argument 'align' which may be one of (left, right,
    center).  As always, Class and style keywords are supported.
    **Not to be confused with class P**. That is
    just for inserting a para break.

    Example:
    
        Paragraph('Some text to center', align='center')
    """
    tagname = 'P'
    attrs = ('class','id', 'style', 'align')
    attr_template , attr_dict = _make_attr_inits(attrs)

Para = Paragraph # Alias

# Headings

class Heading(AbstractTag):
    """Heading markups for H1 - H6

    Heading(level, text, **kw)

    The *level* arg is an integer for the level of the heading.
    Valid levels are 1-6.
    The *text* arg is a string (or any object) for the text of the heading.
    Keyword arguments are align, Class, and style.

    For example:
    h = Heading(2, 'Chapter 3', align='center')
    """
    tagname = ''
    attrs = ('class','id', 'style', 'align')
    attr_template , attr_dict = _make_attr_inits(attrs)
    def __str__(self):
        if not self.tagname:
            if self.contents[0] not in (1,2,3,4,5,6):
                raise AttributeError, "First arg of Heading must be int from 1 to 6."
            self.tagname = 'H%d' % self.contents[0]
            del self.contents[0]
        return AbstractTag.__str__(self)

H = Head = Header = Heading # Aliases

class Caption(AbstractTag):
    """Define a caption for a table.
    """
    tagname = 'CAPTION'
    attrs = ('class','id', 'style', 'align', 'valign')
    attr_template , attr_dict = _make_attr_inits(attrs)

class TH(AbstractTag):
    """Define a table header cell.
    """
    tagname = 'TH'
    attrs = ('class','id', 'style', 'nowrap', 'align','valign','rowspan',
             'colspan', 'height', 'width', 'bgcolor', 'background',
             'bordercolor', 'bordercolordark', 'bordercolorlight')
    attr_template , attr_dict = _make_attr_inits(attrs)
    trailer = ''

class TR(AbstractTag):
    """Define a row of cells within a table.
    """
    tagname = 'TR'
    attrs = ('class','id', 'style', 'align', 'bgcolor', 'bordercolor',
             'bordercolordark', 'bordercolorlight', 'nowrap', 'valign')
    attr_template , attr_dict = _make_attr_inits(attrs)

class TD(AbstractTag):
    """Define a table data cell.
    """
    tagname = 'TD'
    attrs = ('class','id', 'style', 'nowrap', 'align','valign', 'background',
             'bordercolor', 'bordercolordark', 'bordercolorlight',
             'rowspan','colspan','height', 'width','bgcolor')
    attr_template , attr_dict = _make_attr_inits(attrs)
    trailer = ''

class TableLite(AbstractTag):
    """Container class for TH TD TR and Caption objects.
    """
    tagname = 'TABLE'
    attrs = ('class','id', 'style', 'align', 'background', 'border',
             'bordercolor', 'bordercolordark', 'bordercolorlight',
             'cols', 'frame', 'cellpadding', 'cellspacing',
             'height', 'hspace', 'width', 'bgcolor', 'nowrap',
             'rules', 'valign', 'vspace')
    attr_template , attr_dict = _make_attr_inits(attrs)

class Pre(AbstractTag):
    """Render the text verbatim honoring line breaks and spacing.

    Does not escape special characters. To override this set html_escape
    to 'ON'.
    """
    tagname = 'PRE'
    attrs = ('width',)
    attr_template , attr_dict = _make_attr_inits(attrs)
    html_escape = 'OFF'

class Strike(AbstractTag):
    """The text is struck trough with a horizontal line.
    """
    tagname = 'STRIKE'
    attrs = ('class','id', 'style')
    attr_template , attr_dict = _make_attr_inits(attrs)
    trailer = ''

class Blockquote(AbstractTag):
    """Indent text as a block quotation.
    """
    tagname = 'BLOCKQUOTE'
    attrs = ('class','id', 'style')
    attr_template , attr_dict = _make_attr_inits(attrs)

Indent = Blockquote

class Big(AbstractTag):
    """Format text in a bigger font.
    """
    tagname = 'BIG'
    attrs = ('class','id', 'style')
    attr_template , attr_dict = _make_attr_inits(attrs)
    trailer = ''

class Font(AbstractTag):
    """Set the size or color of the text.
    """
    tagname = 'FONT'
    attrs = ('color', 'face', 'size')
    attr_template , attr_dict = _make_attr_inits(attrs)
    trailer = ''

class Address(AbstractTag):
    """A mailing address. Not a URL.
    """
    tagname = 'ADDRESS'
    attrs = ('class','id', 'style')
    attr_template , attr_dict = _make_attr_inits(attrs)
    trailer = ''

class Emphasis(AbstractTag):
    """Format with additional emphasis. (usually italics)
    """
    tagname = 'EM'
    attrs = ('class','id', 'style')
    attr_template , attr_dict = _make_attr_inits(attrs)
    trailer = ''

class Center(AbstractTag):
    """Center the text.
    """
    tagname = 'center'
    attrs = ()
    attr_template , attr_dict = _make_attr_inits(attrs)
    
class Cite(AbstractTag):
    """A citation.
    """
    tagname = 'CITE'
    attrs = ('class','id', 'style')
    attr_template , attr_dict = _make_attr_inits(attrs)
    trailer = ''

class KBD(AbstractTag):
    """Keyboard-like input.
    """
    tagname = 'KBD'
    attrs = ('class','id', 'style')
    attr_template , attr_dict = _make_attr_inits(attrs)
    html_escape = 'OFF'

class Sample(AbstractTag):
    """Sample text. Escaping of special characters is not performed.

    To enable escaping set html_escape='ON'.
    """
    tagname = 'SAMP'
    attrs = ('class','id', 'style')
    attr_template , attr_dict = _make_attr_inits(attrs)
    html_escape = 'OFF'
    
class Strong(AbstractTag):
    """Strongly emphasize the text.
    """
    tagname = 'STRONG'
    attrs = ('class','id', 'style')
    attr_template , attr_dict = _make_attr_inits(attrs)
    trailer = ''

class Code(AbstractTag):
    """Code sample. Escaping of special characters is not performed.

    To enable escaping set html_escape='ON'.
    """
    tagname = 'CODE'
    attrs = ('class','id', 'style')
    attr_template , attr_dict = _make_attr_inits(attrs)
    html_escape = 'OFF'

class Define(AbstractTag):
    """Format as definition text.
    """
    tagname = 'DFN'
    attrs = ('class','id', 'style')
    attr_template , attr_dict = _make_attr_inits(attrs)

class Var(AbstractTag):
    """Used for variable names.
    """
    tagname = 'VAR'
    attrs = ('class','id', 'style')
    attr_template , attr_dict = _make_attr_inits(attrs)
    trailer = ''

class Div(AbstractTag):
    """Specify a division within a document.
    """
    tagname = 'DIV'
    attrs = ('class','id', 'style', 'align', 'lang', 'nowrap')
    attr_template , attr_dict = _make_attr_inits(attrs)

class TT(AbstractTag):
    """Format teletype style.
    """
    tagname = 'TT'
    attrs = ('class','id', 'style')
    attr_template , attr_dict = _make_attr_inits(attrs)
    trailer = ''

class U(AbstractTag):
    """Underlined text.
    """
    tagname = 'U'
    attrs = ('class','id', 'style')
    attr_template , attr_dict = _make_attr_inits(attrs)
    trailer = ''

class Nobr(AbstractTag):
    """Specify non-breaking text.
    """
    tagname = 'NOBR'
    attrs = ()
    attr_template , attr_dict = _make_attr_inits(attrs)
    trailer = ''
    
class Small(AbstractTag):
    """Render in a smaller font.
    """
    tagname = 'SMALL'
    attrs = ('class','id', 'style')
    attr_template , attr_dict = _make_attr_inits(attrs)
    trailer = ''
    
class Sub(AbstractTag):
    """Render as subscript.
    """
    tagname = 'SUB'
    attrs = ('class','id', 'style')
    attr_template , attr_dict = _make_attr_inits(attrs)
    trailer = ''

class Sup(AbstractTag):
    """Render as subscript.
    """
    tagname = 'SUP'
    attrs = ('class','id', 'style')
    attr_template , attr_dict = _make_attr_inits(attrs)
    trailer = ''
    
class Span(AbstractTag):
    """Generic tag to mark text for a style application.
    """
    tagname = 'SPAN'
    attrs = ('class','id', 'style')
    attr_template , attr_dict = _make_attr_inits(attrs)



# Text Formatting Classes
class InitialCaps:
    """Utility class to process text into Initial Upper Case style
    using Font specifications. All text is converted to upper case
    and the initial characters are altered by the size given by
    the optional second argument. The rest of the characters are
    altered by the size given in the optional third argument.

    For example:
    
       InitialCaps('We the people', '+3', '+1')
    """
    def __init__(self, text='', upsize = '+2', downsize = '+1'):
        self.hi = Font(size=upsize)
        self.lo = Font(size=downsize)
        self.text = text
        self.upsize = upsize
        self.downsize = downsize

    def __str__(self):
        list = string.split(self.text)
        wordlist = []
        for word in list:
            word = self.hi(string.upper(word[0])) + self.lo(string.upper(word[1:]))
            wordlist.append(word)
        return string.join(wordlist)

    def __call__(self, text):
        self.text = text
        return self.__str__()

class Text:
    """Class to encapsulate text. Escape special characters for HTML.
    """
    def __init__(self, text=''):
        if type(text) == StringType:
            text = escape(text)
        self.text = str(text)

    def append(self, text=''):
        """Concatenate text characters onto the end.
        
        Will escape special characters.
        """
        if type(text) == StringType:
            text = escape(text)
        self.text = self.text + ' ' + str(text)

    def __str__(self):
        return self.text

class RawText:
    """Class to encapsulate raw text. Does **NOT** escape special characters.
    """
    def __init__(self, text=''):
        self.text = text
        
    def append(self, text):
        self.text = self.text + str(text)

    def __str__(self):
        return self.text

# ALIASES
PRE = Pre
Bold = STRONG = Strong
Italic = EM = Emphasis
Typewriter = TT

class Comment:
    """Place a comment internal to the HTML document.

    Will not be visible from the browser.
    """
    def __init__(self, text=''):
        self.text = text

    def __str__(self):
        return '\n<!-- %s -->\n' % self.text

    def __call__(self, text):
        self.text = text
        return self.__str__()

###### UTILITIES USED INTERNALLY ########

def escape(text, replace=string.replace):
    """Converts the special characters '<', '>', and '&'.

    RFC 1866 specifies that these characters be represented
    in HTML as &lt; &gt; and &amp; respectively. In Python
    1.5 we use the new string.replace() function for speed.
    """
    text = replace(text, '&', '&amp;') # must be done 1st
    text = replace(text, '<', '&lt;')
    text = replace(text, '>', '&gt;')
    return text

def markup_re(text, rex=None, marker=None, collapse=0):
    """Markup the contained text with a given re pattern/object with
    a given tag class instance. Uses re module.

    Arguments

        text -- string to act on
        rex -- a regular expression object or pattern from the re module which will be used
            to match all text patterns in the Paragraph body. Must have a single
            group defined. Group 1 is the matching text that will be marked.
            Defaults to all parenthetical text.
        marker -- an HTMLgen class instance to which the found text will
            be sent for wrapping (using its __call__ method). Default is Emphasis.
            Can be your function as well.
        collapse -- Optional flag. When set to 1 removes the non-
            grouped matching text from the output. Default 0.

    Returns tuple pair of the marked text and the number of matching text groups.
    """
    if rex is None: rex = re.compile('\(([^)]*)\)')
    if marker is None: marker = Emphasis()
    if type(rex) == StringType: rex = re.compile(rex)
    endpoints = []
    output = []
    i = 0
    count = 0
    while 1:
        # build up a list of tuples: ( 'u'|'m', begin, end ) 
        # 'u' indicates unmarked text and 'm' marked text
        # begin and end is the range of characters
        match = rex.search(text, i)
        if match:
            if collapse: #skip chars outside group1
                endpoints.append( ('u', i, match.start(0)) )
                i = match.end(0)
            else: #incl chars outside group1
                endpoints.append( ('u', i, match.start(1)) )
                i = match.end(1)
            endpoints.append( ('m', match.start(1), match.end(1)) ) #text2Bmarked
            count = count + 1
        else:
            endpoints.append( ('u', i, len(text) ) ) # tack on an ending slice
            break

    if count == 0: return text, 0  # didn't find any matches
    for (style, begin, end) in endpoints:
        if style == 'm':
            output.append(marker(text[begin:end]))
        else:
            output.append(text[begin:end])
    return string.join(output, ''), count


class URL:
    """Represent a Universal Resource Locator.
    
    Assumed to be of the form: **http://www.node.edu/directory/file.html**
    with *http* being an example protocol, *www.node.edu* being an example
    network node, *directory* being the directory path on that node, and
    *file.html* being the target filename. The argument string is parsed
    into attributes .proto , .node , .dir , .file respectively and may
    be altered individually after instantiation. The __str__ method
    simply reassembles the components into a full URL string.
    """
    def __init__(self, url):
        self.url = url
        self.parse(url)
    def parse(self, url):
        import urlparse
        self.unparse = urlparse.urlunparse
        self.proto, self.node, self.path, self.params, self.query, self.fragment = \
                    urlparse(url)
        self.dir, self.file = self.split(self.path)

    def split(self, p):
        """Same as posixpath.split()

        Copied here for availability on the Mac.
        """
        i = string.rfind(p, '/') + 1
        head, tail = p[:i], p[i:]
        if head and head != '/'*len(head):
                while head[-1] == '/':
                        head = head[:-1]
        return head, tail

    def __str__(self):
        return self.unparse( (self.proto,
                              self.node,
                              self.dir+self.file,
                              self.params,
                              self.query, self.fragment) )

    def copy(self):
        """No argument. Return a copy of this object.
        """
        return copy.deepcopy(self)

def mpath(path):
    """Converts a POSIX path to an equivalent Macintosh path.

    Works for ./x ../x /x and bare pathnames.
    Won't work for '../../style/paths'.

    Also will expand environment variables and Cshell tilde
    notation if running on a POSIX platform.
    """
    import os
    if os.name == 'mac' : #I'm on a Mac
        if path[:3] == '../': #parent
            mp = '::'
            path = path[3:]
        elif path[:2] == './': #relative
            mp = ':'
            path = path[2:]
        elif path[0] == '/': #absolute
            mp = ''
            path = path[1:]
        else: # bare relative
            mp = ''
        pl = string.split(path, '/')
        mp = mp + string.join(pl, ':')
        return mp
    elif os.name == 'posix': # Expand Unix variables
        if path[0] == '~' :
            path = os.path.expanduser( path )
        if '$' in path:
            path = os.path.expandvars( path )
        return path
    else: # needs to take care of dos & nt someday
        return path

#_realopen = open  #If I do a lot of mpath I can overload 'open'
#def open(filename, mode = 'r', bufsize = -1):
#    return _realopen( mpath(filename), mode, bufsize )

def relpath(path1, path2):
    """Return the relative path from directory 'path1' to directory 'path2'

    Both arguments are assumed to be directory names as there is no
    way to really distinguish a file from a directory by names
    alone. To loosen this restriction one can either assume that both
    arguments represent files or directories currently extant so that
    they can be tested, or add extra arguments to flag the path types
    (file or directory).

    I chose to impose this restriction because I will use this function
    in places where the pathnames are for files yet to be created.
    """
    common = os.path.commonprefix([path1, path2])
    sliceoff = len(common)
    path1 = path1[sliceoff:]
    path2 = path2[sliceoff:]

    dirs1 = string.split(path1, os.sep) # list of directory components below
                                        # the common path
    dirs1 = filter(lambda x: x, dirs1)  # filter out empty elements
    rel = (os.pardir+os.sep)*len(dirs1) # construct the relative path to the
                                        # common point
    return rel+path2


def compare_f2f(f1, f2):
    """Helper to compare two files, return 0 if they are equal."""

    BUFSIZE = 8192
    fp1 = open(f1)
    try:
        fp2 = open(f2)
        try:
            while 1:
                b1 = fp1.read(BUFSIZE)
                b2 = fp2.read(BUFSIZE)
                if not b1 and not b2: return 0
                c = cmp(b1, b2)
                if c:
                    return c
        finally:
            fp2.close()
    finally:
        fp1.close()

def compare_s2f(s, f2):
    """Helper to compare a string to a file, return 0 if they are equal."""

    BUFSIZE = 8192
    i = 0
    fp2 = open(f2)
    try:
        while 1:
            try:
                b1 = s[i: i + BUFSIZE]
                i = i + BUFSIZE
            except IndexError:
                b1 = ''
            b2 = fp2.read(BUFSIZE)
            if not b1 and not b2: return 0
            c = cmp(b1, b2)
            if c: return c
    finally:
        fp2.close()

#!/usr/bin/env python
#'$Id$'
# COPYRIGHT (C) 1999  ROBIN FRIEDRICH  email:Robin.Friedrich@pdq.net
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
"""Classes to generate and insert hyperlinks to navigate among a series
of HTML documents.
"""
import HTMLgen
import string, os

class NavLinks:
    """Provide function to insert a hyperlink navigation component in a document.

    Usage:  A = NavLinks()
            A(list_of_docs)    where list_of_docs is a list of tuple pairs
                               (HTMLdoc, docfilename). HTMLdoc can be either
                               an instance of SimpleDocument (or child),
                               an instance of StringTemplate (or child),
                               or a string. These documents must contain a
                               marker string "{NAVLINKS}" where you wish the
                               nav aid to be placed. The docfilename is the
                               filename for the HTML document when it is
                               written to disk.
                               Calling the NavLinks instance as a function
                               given this list of tuples mutates the individual
                               documents. Then it is up to the user to write
                               the objects out as files. See test() for examples.
                               An optional second argument to the instance call
                               can specify a different tag, e.g.,
                                   A(doclist, 'INDEX') would look for {INDEX}
                               instead of {NAVLINKS}.
    """

    def __init__(self, **kw):
        self.uplink = None
        self.upimage  = 'UP'
        self.style = 'font-size: 75%'
        self.__dict__.update(kw)
    
    def __call__(self, docs, marker='NAVLINKS'):
        self.docs = docs # docs must be a list of tuple pairs (doc_obj, filename)
        self.last = len(docs) - 1
        i = 0
        for doc, filename in docs:
            if isinstance(doc, HTMLgen.StringTemplate):
                doc[marker] = self.generate_navbar(i)
            elif isinstance(doc, HTMLgen.SimpleDocument):
                bodystring = '%s\n' * len(doc.contents)
                T = HTMLgen.StringTemplate(bodystring % tuple(doc.contents))
                T[marker] = self.generate_navbar(i)
                docs[i].contents = [str(T)]
            else:
                T = HTMLgen.StringTemplate(str(doc))
                T[marker] = self.generate_navbar(i)
                docs[i] = str(T), filename
            i = i + 1

    def generate_navbar(self, index):
        """Generate and return navbar.
        """
        links = []
        for i in range(self.last+1):
            if i < index - 1:
                link = self.link_past_page(i)
            elif i == index - 1:
                link = self.link_previous_page(i)
            elif i == index:
                link = self.link_this_page(i)
            elif i == index + 1:
                link = self.link_next_page(i)
            else:
                link = self.link_future_page(i)
            if link is not None: links.append(link)

        navbar = self.link_up() + ' ' + self.get_left_terminus() + \
                 string.join(links, self.get_sep()) + self.get_right_terminus()
        if self.style:
            navbar = HTMLgen.Span(navbar, style=self.style, html_escape='OFF')
        return navbar

    def link_this_page(self, index):
        """Return symbol used for this page.
        """
        return str(index+1)

    def link_past_page(self, index):
        """Return symbol used for non-adjacent past pages.
        """
        doc, filename = self.docs[index]
        return str(HTMLgen.Href(filename, index+1))
        
    def link_previous_page(self, index):
        """Return symbol used for previous page.
        """
        doc, filename = self.docs[index]
        return str(HTMLgen.Href(filename, index+1))

    def link_next_page(self, index):
        """Return symbol used for next page.
        """
        doc, filename = self.docs[index]
        return str(HTMLgen.Href(filename, index+1))

    def link_future_page(self, index):
        """Return symbol used for non-adjacent following pages.
        """
        doc, filename = self.docs[index]
        return str(HTMLgen.Href(filename, index+1))

    def link_up(self):
        """Return symbol used to traverse upward.
        """
        if self.uplink is not None:
            return str(HTMLgen.Href(self.uplink, self.upimage))
        else:
            return ''

    def get_left_terminus(self):
        """Return symbol used as left delimiter of navlinks.
        """
        return '[ '
    def get_right_terminus(self):
        """Return symbol used as right delimiter of navlinks.
        """
        return ' ]'
    def get_sep(self):
        """Return symbol used to separate navlinks.
        """
        return ' |\n'
    def null(self, *arg):
        """Return empty string. Used to nullify one of these feature methods.
        """
        return ''

class AdjacentArrows(NavLinks):
    """Just put forward and back arrow images.

    Mandatory attributes:
        prev_image, next_image
    Optional Attribute:
        uplink, upimage
    set either as keywords to the constructor or as
    attribute assignments to instance."""
    
    def link_previous_page(self, index):
        doc, filename = self.docs[index]
        return str(HTMLgen.Href(filename, self.prev_image))

    def link_next_page(self, index):
        doc, filename = self.docs[index]
        return str(HTMLgen.Href(filename, self.next_image))
        
    def link_future_page(self, index):
        return ''
    link_past_page = link_this_page = link_future_page = NavLinks.null
    get_left_terminus = get_right_terminus = get_sep = NavLinks.null


class AdjacentArrows2(AdjacentArrows):
    """Put forward and back arrow images plus a current doc icon.

    Mandatory attributes:
        prev_image, next_image, this_page_image
    Optional Attribute:
        uplink, upimage
    set either as keywords to the constructor or as
    attribute assignments to instance."""
    def link_this_page(self, index):
        return str(self.this_page_image)


class AllIcons(NavLinks):
    """Display iconic links for all pages in series.

    Mandatory attributes:
        prev_image, this_page_image, next_image, past_image, future_image
    Optional Attribute:
        uplink, upimage
    set either as keywords to the constructor or
    as attribute assignments to instance.
    """
    def link_this_page(self, index):
        return str(self.this_page_image)

    def link_past_page(self, index):
        doc, filename = self.docs[index]
        return str(HTMLgen.Href(filename, self.past_image))
        
    def link_previous_page(self, index):
        doc, filename = self.docs[index]
        return str(HTMLgen.Href(filename, self.prev_image))

    def link_next_page(self, index):
        doc, filename = self.docs[index]
        return str(HTMLgen.Href(filename, self.next_image))

    def link_future_page(self, index):
        doc, filename = self.docs[index]
        return str(HTMLgen.Href(filename, self.future_image))

    get_left_terminus = get_right_terminus = get_sep = NavLinks.null

class AllFilenames(NavLinks):
    """Display iconic links for all pages in series.

    Optional Attribute:
        uplink, upimage
    set either as keywords to the constructor or
    as attribute assignments to instance.
    """
    def link_this_page(self, index):
        doc, filename = self.docs[index]
        name = os.path.splitext(os.path.basename(filename))[0]
        return name
        
    def link_previous_page(self, index):
        doc, filename = self.docs[index]
        name = os.path.splitext(os.path.basename(filename))[0]
        return str(HTMLgen.Href(filename, name))

    link_past_page = link_next_page = link_future_page = link_previous_page




def test():
    prev_image      = HTMLgen.Image('image/prev_image.gif', border=0)
    this_page_image = HTMLgen.Image('image/this_image.gif', border=0)
    next_image      = HTMLgen.Image('image/next_image.gif', border=0)
    past_image      = HTMLgen.Image('image/past_image.gif', border=0)
    future_image    = HTMLgen.Image('image/future_image.gif', border=0)
    up_image        = HTMLgen.Image('image/up_image.gif', border=0)

    docs1 = []
    docs2 = []
    docs3 = []
    docs4 = []
    docs5 = []
    for name in ('one','two','three','four','five','six'):
        doc = HTMLgen.SimpleDocument(bgcolor='#FFFFFF', title=name)
        doc.append('{NAVLINKS}')
        doc.append('<P>SOME GOOD STUFF<P>')
        docs1.append( (doc.copy(), name+'.html') )
        docs2.append( (doc.copy(), '2'+name+'.html') )
        docs3.append( (doc.copy(), '3'+name+'.html') )
        docs4.append( (doc.copy(), '4'+name+'.html') )
        docs5.append( (doc.copy(), '5'+name+'.html') )

    A = AllIcons(prev_image=prev_image, this_page_image=this_page_image,
                 next_image=next_image, past_image=past_image, future_image=future_image)
    A.uplink = '../index.html'
    A.upimage = up_image
    A(docs1)
    for doc, fn in docs1:
        f = open(fn, 'w')
        f.write(doc)
        f.close()

    B = AllFilenames()
    B.uplink = '../index.html'
    B(docs2)
    for doc, fn in docs2:
        f = open(fn, 'w')
        f.write(doc)
        f.close()

    C = AdjacentArrows()
    C.prev_image = prev_image
    C.next_image = next_image
    C(docs3)
    for doc, fn in docs3:
        f = open(fn, 'w')
        f.write(doc)
        f.close()

    D = AdjacentArrows2()
    D.prev_image = prev_image
    D.next_image = next_image
    D.this_page_image = this_page_image
    D(docs4)
    for doc, fn in docs4:
        f = open(fn, 'w')
        f.write(doc)
        f.close()
    
    E = NavLinks(uplink='../')
    E(docs5)
    for doc, fn in docs5:
        f = open(fn, 'w')
        f.write(doc)
        f.close()
    
if __name__ == '__main__': test()

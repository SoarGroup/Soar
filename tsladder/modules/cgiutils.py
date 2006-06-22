# Version 0.3.4
# 2005/10/29

# Copyright Michael Foord 2004 & 2005
# cgiutils.py
# Functions and constants useful for working with CGIs

# http://www.voidspace.org.uk/python/modules.shtml

# Released subject to the BSD License
# Please see http://www.voidspace.org.uk/documents/BSD-LICENSE.txt

# For information about bugfixes, updates and support, please join the Pythonutils mailing list.
# http://voidspace.org.uk/mailman/listinfo/pythonutils_voidspace.org.uk
# Comments, suggestions and bug reports welcome.
# Scripts maintained at http://www.voidspace.org.uk/python/index.shtml
# E-mail fuzzyman@voidspace.org.uk

import os
import sys

__all__ = [
    'serverline',
    'SENDMAIL',
    'validchars',
    'alphanums',
    'getrequest',
    'getform',
    'getall',
    'isblank',
    'formencode',
    'formdecode',
    'mailme',
    'sendmailme',
    'createhtmlmail',
    'environdata',
    'validemail',
    'cgiprint',
    'ucgiprint',
    'replace',
    'error',
    'makeindexline',
    'istrue',
    'randomstring',
    ]

serverline = "Content-Type: text/html"

# A common location of sendmail on servers
SENDMAIL = "/usr/sbin/sendmail"
validchars = 'abcdefghijklmnopqrstuvwxyz0123456789!-_*'
alphanums = 'abcdefghijklmnopqrstuvwxyz0123456789'

#######################################################
# Some functions for dealing with CGI forms (instances of FieldStorage)

def getrequest(valuelist=None, nolist=False):
    """
    Initialise the ``FieldStorage`` and return the specified list of values as 
    a dictionary.
    
    If you don't specify a list of values, then *all* values will be returned.
    
    If you set ``nolist`` to ``True`` then any parameters supplied as lists 
    will only have their first entry returned.
    """
    import cgi
    form = cgi.FieldStorage()
    if valuelist is not None:
        return getform(valuelist, form, nolist=nolist)
    else:
        return getall(form, nolist=nolist)

def getform(valuelist, theform, notpresent='', nolist=False):
    """
    This function, given a CGI form, extracts the data from it, based on
    valuelist passed in. Any non-present values are set to '' - although this 
    can be changed.
    
    It also takes a keyword argument 'nolist'. If this is True list values only 
    return their first value.
    
    Returns a dictionary.
    """
    data = {}
    for field in valuelist:
        if not theform.has_key(field):
            data[field] = notpresent
        else:
            if not isinstance(theform[field], list):
                data[field] = theform[field].value
            else:
                if not nolist:
                    # allows for list type values
                    data[field] = [x.value for x in theform[field]]
                else:
                    # just fetch the first item
                    data[field] = theform[field][0].value
    return data

def getall(theform, nolist=False):
    """
    Passed a form (FieldStorage instance) return all the values.
    This doesn't take into account file uploads.
    
    Also accepts the 'nolist' keyword argument as ``getform``.
    
    Returns a dictionary.
    """
    data = {}
    for field in theform.keys():
        if not isinstance(theform[field], list):
            data[field] = theform[field].value
        else:
            if not nolist:
                # allows for list type values
                data[field] = [x.value for x in theform[field]]
            else:
                # just fetch the first item
                data[field] = theform[field][0].value
    return data

def isblank(indict):
    """
    Passed an indict of values it checks if any of the values are set.
    
    Returns ``True`` if every member of the indict is empty (evaluates as False).
    
    I use it on a form processed with getform to tell if my CGI has been 
    activated without any values.
    """
    return not [val for val in indict.values() if val]

def formencode(theform):
    """
    A version that turns a cgi form into a single string.
    It only handles single and list values, not multipart.
    This allows the contents of a form requested to be encoded into a single value as part of another request.
    """
    from urllib import urlencode, quote_plus
    return quote_plus(urlencode(getall(theform)))

def formdecode(thestring):
    """Decode a single string back into a form like dictionary."""
    from cgi import parse_qs
    from urllib import unquote_plus 
    return parse_qs(unquote_plus(thestring), True)


#############################################################
# Functions for handling emails
#
# Use mailme for sending email - specify a path to sendmail *or* a host, port etc (optionally username)


def mailme(to_email, msg, email_subject=None, from_email=None,
            host='localhost', port=25, username=None, password=None,
            html=True, sendmail=None):
    """
    This function will send an email using ``sendmail`` or ``smtplib``, depending 
    on what parameters you pass it.
    
    If you want to use ``sendmail`` to send the email then set 
    ``sendmail='/path/to/sendmail'``. (The ``SENDMAIL`` value from Constants_ often 
    works).
    
    If you aren't using sendmail then you will need to set ``host`` and ``port`` to 
    the correct values. If your server requires authentication then you'll need to 
    supply the correct ``username`` and ``password``. 
    
    ``to_email`` can be a single email address, *or* a list of addresses.
    
    ``mailme`` *assumes* you are sending an html email created by 
    ``createhtmlmail``. If this isn't the case then set ``html=False``.
    
    Some servers won't let you send a message without supplying a ``from_email``.
    """
    if sendmail is not None:
        # use sendmailme if specified
        return sendmailme(to_email, msg, email_subject, from_email, 
                            html, sendmail)
    if not isinstance(to_email, list):
        # if we have a single email then change it into a list
        to_email = [to_email]
    #
    import smtplib
    #
    head = "To: %s\r\n" % ','.join(to_email) 
    if from_email is not None:
        head += ('From: %s\r\n' % from_email)
    # subject is in the body of an html email
    if not html and email_subject is not None:
        head += ("Subject: %s\r\n\r\n" % email_subject)
    msg = head + msg
    #
    server = smtplib.SMTP(host, port)
    if username:
        server.login(username, password)
    server.sendmail(from_email, to_email, msg)
    server.quit()

 
def sendmailme(to_email, msg, email_subject=None, from_email=None, 
                html=True, sendmail=SENDMAIL):
    """
    Quick and dirty, pipe a message to sendmail. Can only work on UNIX type systems 
    with sendmail.
    
    Will need the path to sendmail - defaults to the 'SENDMAIL' constant.
    
    ``to_email`` can be a single email address, *or* a list of addresses.
    
    *Assumes* you are sending an html email created by ``createhtmlmail``. If this 
    isn't the case then set ``html=False``.
    """
    if not isinstance(to_email, list):
        to_email = [to_email]
    o = os.popen("%(a)s -t -f %(b)s" % {'a':sendmail, 'b':from_email},"w")
    o.write("To: %s\r\n" %  ','.join(to_email))
    if from_email:
        o.write("From: %s\r\n" %  from_email)
    if not html and email_subject:
        o.write("Subject: %s\r\n" %  email_subject)
    o.write("\r\n")
    o.write("%s\r\n" % msg)
    o.close()

def createhtmlmail(subject, html, text=None):
    """
    Create a mime-message that will render as HTML or text as appropriate.
    If no text is supplied we use htmllib to guess a text rendering. 
    (so html needs to be well formed) 
    
    Adapted from recipe 13.5 from Python Cookbook 2
    """
    import MimeWriter, mimetools, StringIO
    if text is None:
        # produce an approximate text from the HTML input
        import htmllib
        import formatter
        textout = StringIO.StringIO()
        formtext = formatter.AbstractFormatter(formatter.DumbWriter(textout))
        parser = htmllib.HTMLParser(formtext)
        parser.feed(html)
        parser.close()
        text = textout.getvalue()
        del textout, formtext, parser
    out = StringIO.StringIO()       # output buffer for our message
    htmlin = StringIO.StringIO(html)  # input buffer for the HTML
    txtin = StringIO.StringIO(text)   # input buffer for the plain text
    writer = MimeWriter.MimeWriter(out)
    # Set up some basic headers. Place subject here because smtplib.sendmail
    # expects it to be in the message, as relevant RFCs prescribe.
    writer.addheader("Subject", subject)
    writer.addheader("MIME-Version", "1.0")
    # Start the multipart section of the message. Multipart/alternative seems
    # to work better on some MUAs than multipart/mixed.
    writer.startmultipartbody("alternative")
    writer.flushheaders()
    # the plain-text section: just copied through, assuming iso-8859-1  # XXXX always true ?
    subpart = writer.nextpart()
    pout = subpart.startbody("text/plain", [("charset", 'iso-8859-l')]) 
    pout.write(txtin.read())
    txtin.close()
    # the HTML subpart of the message: quoted-printable, just in case
    subpart = writer.nextpart()
    subpart.addheader("Content-Transfer-Encoding", "quoted-printable")
    pout = subpart.startbody("text/html", [("charset", 'us-ascii')])
    mimetools.encode(htmlin, pout, 'quoted-printable')
    htmlin.close()
    # You're done; close your writer and return the message as a string
    writer.lastpart()
    msg = out.getvalue()
    out.close()
    return msg    

def environdata():
    """Returns some data about the CGI environment, in a way that can be mailed."""
    ENVIRONLIST = [ 'REQUEST_URI','HTTP_USER_AGENT','REMOTE_ADDR','HTTP_FROM','REMOTE_HOST','REMOTE_PORT','SERVER_SOFTWARE','HTTP_REFERER','REMOTE_IDENT','REMOTE_USER','QUERY_STRING','DATE_LOCAL' ]  # XXX put this in template ??
    environs = []
    environs.append("\n\n---------------------------------------\n")
    for x in ENVIRONLIST:
        if os.environ.has_key(x):
            environs.append("%s: %s\n" % (x, os.environ[x]))
    environs.append("---------------------------------------\n")
    return ''.join(environs)

def validemail(email):
    """
    A quick function to do a basic email validation.
    Returns False or the email address.
    """
    if ' ' in email:
        return False
    dot = email.rfind('.')
    at = email.find('@')
    if dot == -1 or at < 1 or at > dot:
        return False
    return email

##########################################################

def error(errorval=''):
    """The generic error function."""
    print serverline
    print
    print '''<html><head><title>An Error Has Occurred</title>
    <body><center>
    <h1>Very Sorry</h1>
    <h2>An Error Has Occurred</h2>'''
    if errorval:
        print '<h3>%s</h3>' % errorval
    print '</center></body></html>'
    sys.exit()
    
#########################################################

def makeindexline(url, startpage, total, numonpage=10, pagesonscreen=5):
    """
    Make a menu line for a given number of inputs, with a certain number per page.
    Will look something like : ::
    
        First  Previous  22 23 24 25 26 27 28 29 30 31 32  Next  Last
    
    Each number or word will be a link to the relevant page.
    
    url should be in the format : ``'<a href="script.py?startpage=%s">%s</a>'`` - 
    it will have the two ``%s`` values filled in by the function.
    
    The url will automatically be put between ``<strong></strong>`` tags. Your 
    script needs to accepts a parameter ``start`` telling it which page to display.
    
    ``startpage`` is the page actually being viewed - which won't be a link.
    
    ``total`` is the number of total inputs.
    
    ``numonpage`` is the number of inputs per page - this tells makeindexline how 
    many pages to divide the total into.
    
    The links shown will be some before startpage and some after. The amount of 
    pages links are shown for is ``pagesonscreen``. (The actual total number shown 
    will be *2 \* pagesonscreen + 1*).
    
    The indexes generated are *a bit* like the ones created by google. Unlike 
    google however, next and previous jump you into the *middle* of the next set of 
    links. i.e. If you are on page 27 next will take you to 33 and previous to 21. 
    (assuming pagesonscreen is 5). This makes it possible to jump more quickly 
    through a lot of links. Also - the current page will always be in the center of 
    the index. (So you never *need* Next just to get to the next page).
    """
    b = '<strong>%s</strong>'
    url = b % url
    outlist = []
    last = ''
    next = ''
    numpages = total//numonpage
    if total%numonpage:
        numpages += 1
    if startpage - pagesonscreen > 1:
        outlist.append(url % (1, 'First'))
        outlist.append('&nbsp;')
        outlist.append(url % (startpage-pagesonscreen-1, 'Previous'))
        outlist.append('&nbsp;')
    index = max(startpage - pagesonscreen, 1)
    end = min(startpage+pagesonscreen, numpages)
    while index <= end:
        if index == startpage:
            outlist.append(b % startpage)
        else:
            outlist.append(url % (index, index))
        index += 1
    outlist.append('&nbsp;')
    if (startpage+pagesonscreen) < numpages:
        outlist.append(url % (startpage+pagesonscreen+1, 'Next'))
        outlist.append('&nbsp;')
        outlist.append(url % (numpages, 'Last'))

    return '&nbsp;'.join(outlist)    

######################################

def istrue(value):
    """
    Accepts a string as input.
    
    If the string is one of  ``True``, ``On``, ``Yes``, or ``1`` it returns 
    ``True``.
    
    If the string is one of  ``False``, ``Off``, ``No``, or ``0`` it returns 
    ``False``.
    
    ``istrue`` is not case sensitive.
    
    Any other input will raise a ``KeyError``. 
    """
    return {
        'yes': True, 'no': False,
        'on': True, 'off': False, 
        '1': True, '0': False,
        'true': True, 'false': False,
        }[value.lower()]

def randomstring(length):
    """
    Return a random string of length 'length'.
    
    The string is comprised only of numbers and lowercase letters.
    """ 
    import random
    outstring = []
    while length > 0:
        length -=1
        outstring.append(alphanums[int(random.random()*36)])
    return ''.join(outstring)

##################################

def cgiprint(inline='', unbuff=False, line_end='\r\n'):
    """
    Print to the ``stdout``.
    
    Set ``unbuff=True`` to flush the buffer after every write.
    
    It prints the inline you send it, followed by the ``line_end``. By default this 
    is ``\r\n`` - which is the standard specified by the RFC for http headers.
    """
    sys.stdout.write(inline)
    sys.stdout.write(line_end)
    if unbuff:
        sys.stdout.flush()

def ucgiprint(inline='', unbuff=False, encoding='UTF-8', line_end='\r\n'):
    """
    A unicode version of ``cgiprint``. It allows you to store everything in your 
    script as unicode and just do your encoding in one place.
    
    Print to the ``stdout``.
    
    Set ``unbuff=True`` to flush the buffer after every write.
    
    It prints the inline you send it, followed by the ``line_end``. By default this 
    is ``\r\n`` - which is the standard specified by the RFC for http headers.
    
    ``inline`` should be a unicode string.
    
    ``encoding`` is the encoding used to encode ``inline`` to a byte-string. It 
    defaults to ``UTF-8``, set it to ``None`` if you pass in ``inline`` as a byte 
    string rather than a unicode string.
    """
    if encoding:
        inline = inline.encode(encoding)
        # don't need to encode the line endings
    sys.stdout.write(inline)
    sys.stdout.write(line_end)
    if unbuff:
        sys.stdout.flush()

def replace(instring, indict):
    """
    This function provides a simple but effective template system for your html 
    pages. Effectively it is a convenient way of doing multiple replaces in a 
    single string.
    
    Takes a string and a dictionary of replacements. 
    
    This function goes through the string and replaces every occurrence of every 
    dicitionary key with it's value.
    
    ``indict`` can also be a list of tuples instead of a dictionary (or anything 
    accepted by the dict function).
    """
    indict = dict(indict)
    if len(indict) > 40:
        regex = re.compile("(%s)" % "|".join(map(re.escape, indict.keys())))
        # For each match, look-up corresponding value in dictionary
        return regex.sub(lambda mo: indict[mo.string[mo.start():mo.end()]],
                                                                    instring)
    for key in indict:
        instring = instring.replace(key, indict[key])
    return instring

############################

if __name__ == '__main__':
    print 'No tests yet - sorry'

"""
TODO/ISSUES
===========

The indexes generated by makeindexline use next to jump 10 pages. This is
different to what people will expect if they are used to the 'Google' type
index lines.

createhtmlmail assumes iso-8859-1 input encoding for the html

email functions to support 'cc' and 'bcc'

Need doctests

Changelog
==========
2005/10/29      Version 0.3.4
Shortened ``isblank``.

2005/09/21      Version 0.3.3
Fixed bug in ``getall`.
Fixed bug in ``getrequest``.

2005/08/27      Version 0.3.2
Large dictionary replaces use a regex approach.

2005/08/20      Version 0.3.1
Improved istrue function.
Added __all__.
Various other code/doc improvements.

2005/04/07      Version 0.3.0
Changed the email functions, this may break things (but it's better this way)
Added createhtmlemail, removed loginmailme
mailme is now a wrapper for sendmailme, mailme, *and* the old loginmailme

2005/03/20      Version 0.2.0
Added ucgiprint and replace.

2005/02/18      Version 0.1.0
The first numbered version.
"""

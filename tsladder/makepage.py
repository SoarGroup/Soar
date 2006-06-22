#! /usr/bin/python

# 2005/09/20
# v 0.6.0
# Copyright Michael Foord 2004
# Part of jalopy
# http://www.voidspace.org.uk/python/jalopy.html

# Released subject to the BSD License
# Please see http://www.voidspace.org.uk/documents/BSD-LICENSE.txt

# For information about bugfixes, updates and support, please join the Pythonutils mailing list.
# http://voidspace.org.uk/mailman/listinfo/pythonutils_voidspace.org.uk
# Comments, suggestions and bug reports welcome.
# Scripts maintained at http://www.voidspace.org.uk/python/index.shtml
# E-mail fuzzyman@voidspace.org.uk

"""
This script is the body of the Jalopy project.

It generates the index pages, saves edited pages, sends pages to be edited and
sends the editor.

It requires pythonutils 0.2.0 or more recent to be installed
(or the modules from it placed in the 'modules directory)
"""

import sys
import cgi
import os
try:
    import cgitb
    cgitb.enable()
except ImportError:
    sys.stderr = sys.stdout

sys.path.append(os.path.abspath('../modules'))
from configobj import ConfigObj
from pathutils import *
from cgiutils import *

# from cgiutils
alphanums += '_-.'
username = 'default'

#############
# Some defaults

# NOTE: When not run in a CGI environment this will fail
thisscript = os.environ['SCRIPT_NAME']
defaultsite = 'jalopy'
configfile = 'jalopy.ini'

# The DEFAULT minimum admin level a user must have to be able to delete pages
# the real one will normally be loaded from the config file
ADMINDEL = 2

#####################
# HTML template stuff
# Used for generating tables and links

tablehead = '''<div id="indextable"><table border="1" width="80%"><caption>Index of Pages</caption>'''
public_head='''<colgroup><col width="25%" /><col width="75%" /></colgroup>
<thead valign="bottom"><tr><th>View Page</th><th>Description</th></tr>
</thead><tbody valign="top">'''
private_head ='''<colgroup><col width="20%" /><col width="15%" /><col width="50%" /><col width="15%" /></colgroup>
<thead valign="bottom"><tr><th>View Page</th><th>Edit Page</th><th>Description</th><th>Edit Details</th></tr>
</thead><tbody valign="top">'''
public_entry = '<tr><td>%s</td><td>%s</td></tr>'
private_entry = '<tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>'
tablefoot = '</tbody></table></div>'
link = '<a href="%s">%s</a>'

backlink = '<div id="backlink"><center><strong><a href="%s">Back to Main Index</a></strong></center><br /></div>'
empty_table = '<h3>None Yet - Sorry</h3>'
welcome_mes = '<h2>Welcome %s.</h2>'

###################################################
# Load the config files

def readconfig(thissite, thescript):
    """Read the config file and make the variables global.
    Not the best way to do it, but it allows this script to be treated as a module.
    In which case the following variables will need to be passed in as globals - or this function called.
    """
    global deftitle, defdescript, defname, deftemplate, templatedir, newtemplatedir
    global editortitle, editorhead, editorfoot
    global weblocmod, tempstyleloc, stylesheet, finalstyleloc
    global thisscript, webpath, webloc, thistemplatedir
    global tablefile, publicindex, privateindex, createform
    global userdir, thesite, admindel, editdetpage
    global editdetform
    #
    # FIXME: absence of this file should raise nicely formatted errror message
    config = ConfigObj(thissite+'.ini', file_error=True)
    thesite = thissite
    #
    # main template directory
    templatedir = tslash(config['templatedir'])
    #
    # default title for new pages
    deftitle = config['deftitle']
    # default description for new pages
    defdescript = config['defdescript']
    # default name for new pages
    defname = config['defname']
    #
    # title for the editor page
    editortitle = config['editortitle']
    # editor page head file
    editorhead = templatedir + 'form_head.txt'
    # editor page footer file
    editorfoot = templatedir + 'form_foot.txt'
    # file in the template directory, contains the form to create new pages.
    createform = templatedir + 'create_form.txt'
    #
    # web location of this site
    webloc = tslash(config['webloc'])
    # web location of kupu directory
    weblocmod = '/kupu/'
    # file path from script to the html files
    webpath = tslash(config['webpath'])
    # filepath from script to saved template directory (the kupu directory)
    newtemplatedir = tslash(config['kupupath'])
    # filename of the stylesheet used for new files
    stylesheet = config['stylesheet']
    #
    # location for template files for this website
    thistemplatedir = thesite + '/'
    # template filename - stored in templates folder and the kupu directory
    deftemplate = config['templatefile']
    #
    # final location of stylesheet relative to web pages
    finalstyleloc = tslash(config['finalstyleloc'])
    # web path from template file to the stylesheet directory
    tempstyleloc = tslash(relpath(newtemplatedir, os.path.join(webpath,
                                                            finalstyleloc)))
    # relpath from kupudir to join(webpath, finalstyleloc)
    # this is because the template is stored in the kupu directory during
    # editing
    #
    # file that has the list of pages in - for building the link tables in the indexes from
    tablefile = thistemplatedir + 'index_table.txt'
    # file in the template directory that has the template for the public index
    # page - used to create index.html in the webloc folder
    publicindex = config['publicindex']
    # file in the template directory that has the template for the private
    # index page - used by makepage
    privateindex = config['privateindex']
    #
    thisscript = thescript
    #
    # the directory containing the user login files.
    # Set to '' to not require a login
    userdir = tslash(config['userdir'])
    # the minimum admin level a user must have before they are able to
    # delete pages. Set to 0 to let anyone delete
    try:
        admindel = int(config['admindel'])
    except (KeyError, ValueError):
        # FIXME: this is set if the config file has a bad value 
        # should I do this or should I let it crash ?
        admindel = ADMINDEL
    #
    editdetpage = thistemplatedir + 'editdetails_page.html'
    editdetform = templatedir + 'editdetails_form.txt'

###############################################################
def main(theform):
    """From the form contents deduce what action is happening, and call the correct function."""
    global username, userconfig
# if userdir is set - do the login stuff.
# This could include new login, checking the cookie or even admin stuff - all transparent
    if userdir:
        from logintools import login
        action, userconfig = login(theform, userdir, thisscript)       # this function displays login and exits if it fails, otherwise it prints the new/updated cookie
        username = userconfig['realname']
#        loginname = userconfig['username']

    else:                                   # we aren't using userdirs - but we must supply an empty username and userconfig file
        username = ''       # XXX - have a default ?
        userconfig = {'admin' : admindel}
            
    if theform.has_key('thisfile'):     # save an edited page
        createpage(theform)
    elif theform.has_key('editor'):    # edit a page
        sendeditor(theform)
    elif theform.has_key('detail'):     # do the edit page details
        editdetail(theform)
    elif theform.has_key('editdetail'):     # display edit page details page
        displayeditdetail(theform)
    elif theform.has_key('delete'):     # called if delete is chosen from the edit details page
        deletepage(theform)
    elif theform.has_key('sendpage'):     # called when we need to send a page to edit
        sendpage(theform)
        
    else:                       # called without parameters - just display the index (create if necessary)
        print serverline
        print
        print getindex().replace('**username**', username.capitalize()).encode('UTF8')
    sys.exit()
    
############################################################################

def sendeditor(theform):
    """Fill in the variables from the editor templates and send it as a webpage."""
    from urllib import urlencode
    editorpage = unicode(readfile(editorhead), 'UTF8') 
    if theform.has_key('pagename'):
        editorpage = editorpage.replace('**pagename**', unicode(theform['pagename'].value, 'UTF8') or defname)
    editorpage = editorpage.replace('**page title**', editortitle)
    editorpage = editorpage.replace('**makepage**', thisscript)
    editorpage = editorpage.replace('href="', 'href="'+weblocmod)
    editorpage = editorpage.replace('src="', 'src="'+weblocmod)
    editorpage = editorpage.replace('**menu link**', backlink % thisscript)
    editorpage = editorpage.replace('**path**', weblocmod)
    editorpage = editorpage.replace('**the site**', thesite)
    editorpage = editorpage + unicode(readfile(editorfoot), 'UTF8')              # the 'src="' in the editorfoot doesn't get changed

    if theform.has_key('edit'):            # we are editing an existing file and should use that
        editfile = unicode(theform['edit'].value, 'UTF8')
        if not os.path.isfile(webpath+editfile):
            error('File to be edited "%s" not found.') % editfile
        doctitle = ''
        description = ''
    else:
        editfile = 'new'
        if theform.has_key('doctitle'):
            doctitle = unicode(theform['doctitle'].value, 'UTF8') or deftitle
        else:
            doctitle = deftitle
        if theform.has_key('description'):
            description = unicode(theform['description'].value, 'UTF8') or defdescript
        else:
            description = defdescript
    getdict = {'sendpage' : editfile.encode('UTF8'), 'site' : thesite.encode('UTF8')}
    if doctitle:
        getdict['title'] = doctitle.encode('UTF8')
        getdict['description'] = description.encode('UTF8')
    getstring = '?' + urlencode(getdict)
    editorpage = editorpage.replace('**filename**', editfile)
    editorpage = editorpage.replace('**edit file**', thisscript + getstring)

# we probably ought to be encoding back into UTF-8 here first.
# What if the receiving end can't receive UTF-8 ?
# Need to check the http headers. (and also test with a few browsers)
    print serverline
    print
    print editorpage.encode('UTF8')

def sendpage(theform):
    """Create the right template file for the editor script to use.
    If we are passed an existing file we should use that as the template.
    If we are making a new page we should use the default template.
    We must return which file is being edited and whether it is a new page or not.
    If it is new there will need to be extra actions taken when the file is saved.
    """
    filename = unicode(theform['sendpage'].value, 'UTF8')
                       
    if filename != 'new':            # we are editing an existing file and should use that
        if os.path.isfile(webpath+filename):
            templatepage = removelinks(unicode(readfile(webpath+filename), 'UTF8'))
        else:
            error('File to be edited "%s" not found.') % filename                       # XXXX vaguely possible the page will be deleted before it is sent - *very* unlikely
        templatepage = templatepage.replace(u'"%s"' % (finalstyleloc+stylesheet), u'"%s"' % (tempstyleloc+stylesheet))
    else:
        action = 'new'
        if theform.has_key('title'):
            doctitle = unicode(theform['title'].value, 'UTF8') or deftitle
        else:
            doctitle = deftitle
        if theform.has_key('description'):
            description = unicode(theform['description'].value, 'UTF8') or defdescript
        else:
            description = defdescript
        filename = description
        templatepage = unicode(readfile(thistemplatedir + deftemplate), 'UTF8')
        templatepage = templatepage.replace('**Document Title**', doctitle)
        templatepage = templatepage.replace('**stylesheet**', tempstyleloc+stylesheet)
        templatepage = templatepage.replace('**description**', description)

    print serverline
    print
    print templatepage.encode('UTF8') 
 
#####################################

def createpage(theform):
    """The function called when the user hits save."""
    import re
    if not theform.has_key('kupu'):
        error('Sorry No Page Was Received')
    thepage = unicode(theform['kupu'].value, 'UTF8')        # XXXX need to check the integrity of this value, did we receive a whole page ??
    thepage = thepage.replace('\r\n', '\n')
    thepage = thepage.replace('"%s"' % (tempstyleloc+stylesheet), '"%s"' % (finalstyleloc+stylesheet))
    thepage = thepage.replace(u'\xa0', u'&nbsp;')
    thepage = thepage.replace(u'<td> </td>', u'<td>&nbsp;</td>')
#    writefile('lastpage.txt', thepage)                       # log of lastpage saved - useful for debugging. XXXX fails on unicode :-)

    thepage = thepage.replace('<meta content="no-cache" />', '<meta http-equiv="Pragma" content="no-cache" />')
    thepage = thepage.replace('<meta content="no-cache, must-revalidate" />', '<meta http-equiv="Cache-Control" content="no-cache, must-revalidate" />')
    thepage = thepage.replace('<meta content="text/html; charset=UTF-8" />', '<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />')

    filename = unicode(theform['filename'].value, 'UTF8')
    thisfile = unicode(theform['thisfile'].value, 'UTF8')
    if filename == 'new':                       # we are creating a new file
        pagename = unicode(theform['pagename'].value, 'UTF8') or defname
        filename = makefilename(pagename)       # makefilename returns a filename from the description
        private_table = changeindex(filename, pagename, thepage)      # add this file to the index.
    else:
        private_table = updateindex(filename, thepage)                      # update description
# now we need to sort out any mangled local links      XXXX prob. need to sort out images as well
    thepage = re.sub(u'"http://[^"]*'+thisscript+'[^"]*#', u'"#', thepage)  # XXXX this regex needs work now
    thepage = addlinks(thepage)                 # this needs to edit the stylesheet location as well 
# XXXX Still need to do the meta values - like original author, last edited by etc.....
    try:
        writefile(webpath + filename, thepage.encode('UTF8'))
    except IOError:
        error('An Error Ocurred whilst saving "%s".' % filename)
    print serverline
    print
    print getindex(private_table).replace('**username**', username.capitalize()).encode('UTF8') 
     
def makefilename(pagename):
    """Given the document title of a page to be created, it returns a filename to use.
    Called when a new page is saved.
    """
    filename = ''
    for char in pagename[:10].lower().replace(' ', '_'):
        if char in alphanums:
            filename += char
    modifier = ''
    while os.path.exists(webpath+filename+modifier+'.html'):
        if not modifier:
            modifier = '1'
        else:
            modifier = str(int(modifier)+1)
    return filename+modifier+'.html'

##############################################

def changeindex(filename, pagename, thepage):
    """
    Updates the index_table and then regenerates the two index pages.
    Link table is a CSV with 3 entries per line :
    filename, pagename, description
    Each entry is quoted using elem_quote.
    
    Public index is template with link table inserted.
    Privated index is template with links and create page form inserted.
    public table has 2 columns - Name(link), Description
    private table has 4 columns - Name(link), Edit Page, Description, Edit Details
    
    public index is saved in webloc as index.html
    private index is saved in templatedir as index.html
    """
    from listquote import csvread, elem_quote
    test = thepage.lower().find('<meta name="description" content="')
    if test == -1:                          # no meta description tag
        description = ''
    else:
        startindex = test + 34
        endindex = thepage.find('"', startindex)
        if endindex == -1:                          # the page is screwed
            description = ''
        else:
            description = thepage[startindex:endindex]

    thefile = ''
    if os.path.isfile(tablefile):
        thefile = unicode(readfile(tablefile), 'UTF8')
    if not thefile.endswith('\n'):
        thefile += '\n'
    thefile += elem_quote(filename) + ', ' + elem_quote(pagename) + ', ' + elem_quote(description) + '\n'
    thetable = [line.strip() for line in thefile.split('\n') if line.strip() and not line[0]=='#']
    items = csvread(thetable)
    writefile(tablefile, thefile.encode('UTF8'))
    return makeindexes(items)
    
def makeindexes(items):
    """
    From the index_table generate the indexes.
    The 'items' parameter is the index_table file read in and already split into items using csvread.
    See the changeindex and updateindex functions.
    """
    public_table = private_table = tablehead
    public_table += public_head
    private_table += private_head
    
    for filename, pagename, description in items:
        description = cgi.escape(description)
        viewlink = link % (webloc+filename, pagename)
        editlink = link % (thisscript+'?editor=on&edit='+cgi.escape(filename), 'Edit Page')
        detaillink = link % (thisscript+'?editdetail='+cgi.escape(filename), 'Edit Details')
        public_table += public_entry % (viewlink, description)
        private_table += private_entry % (viewlink, editlink, description, detaillink)
    public_table += tablefoot
    private_table += tablefoot

# read in the publiindex template, replace into it the new table and save it out again ! (all in one breath)    
    writefile(webpath+'index.html', unicode(readfile(thistemplatedir+publicindex), 'UTF8').replace('**table of links**', public_table).encode('UTF8'))
    return private_table

def gettable():
    """Build and return just the private_table for the index."""
    from listquote import csvread, elem_quote   
    private_table = tablehead + private_head
    if os.path.isfile(tablefile):
        thefile = unicode(readfile(tablefile), 'UTF8')
    else:
        return '<h2>None Yet, Sorry.</h2>'
    thetable = [line.strip() for line in thefile.split('\n') if line.strip() and not line[0]=='#']
    items = csvread(thetable)    
    for filename, pagename, description in items:
        description = cgi.escape(description)
        viewlink = link % (webloc+filename, pagename)
        editlink = link % (thisscript+'?editor=on&edit='+cgi.escape(filename), 'Edit Page')
        detaillink = link % (thisscript+'?editdetail='+cgi.escape(filename), 'Edit Details')
        private_table += private_entry % (viewlink, editlink, description, detaillink)
    private_table += tablefoot    
    return private_table  
        
def getindex(private_table=None):
    """Build and return the private index page."""
    if not private_table:
        private_table = gettable()
    privatepage = unicode(readfile(thistemplatedir+privateindex), 'UTF8')
    privatepage = privatepage.replace('**table of links**', private_table)   
    
    thecreateform = readfile(createform)
    thecreateform = thecreateform.replace('**makepage**', thisscript)
    thecreateform = thecreateform.replace('**page title**', deftitle)
    thecreateform = thecreateform.replace('**description**', defdescript)
    thecreateform = thecreateform.replace('**page name**', defname)
    thecreateform = thecreateform.replace('**the site**', thesite)
    
    privatepage = privatepage.replace('**createpage**', thecreateform)
    if userdir:
        privatepage = privatepage.replace('**logout**', thisscript+'?login=logout')
        privatepage = privatepage.replace('**edit account**', thisscript+'?login=editaccount')
        privatepage = privatepage.replace('<!-- **commstart** ', '')
        privatepage = privatepage.replace(' **commend** -->', '')
        if int(userconfig['admin']):
            privatepage = privatepage.replace('<!-- **admincommstart** ', '')
            privatepage = privatepage.replace(' **admincommend** -->', '')
            privatepage = privatepage.replace('**admin menu**', thisscript+'?login=admin')
    return privatepage
# tablefile, publicindex, privateindex, createform - all stored in templatedir.

def updateindex(filename, thepage):
    """
    When a file is saved after editing (not a new page) this checks if the description has changed.
    If it has it will need to update the indexes.

    Link table is a CSV with 3 entries per line :
    filename, pagename, description
    Each entry is quoted using elem_quote. (csvread automatically undoes this for us)
    """
    from listquote import csvread, elem_quote, csvwrite
    thefile = u''
    if os.path.isfile(tablefile):
        thefile = unicode(readfile(tablefile), 'UTF8')
    thetable = [line.strip() for line in thefile.split('\n') if line.strip() and not line[0]=='#']
    items = csvread(thetable)

    test = thepage.lower().find('<meta name="description" content="')
    if test == -1:                          # no meta description tag
        return
    startindex = test + 34
    endindex = thepage.find('"', startindex)
    if endindex == -1:                          # the page is screwed
        return
    description = thepage[startindex:endindex]
    
    outitems = []
    changed = False
    for pageline in items:
        if pageline[0] != filename or description == pageline[2]:
            outitems.append(pageline)
        else:
            changed = True
            pageline[2] = description
            outitems.append(pageline)
    if changed:
        writefile(tablefile, ('\n'.join(csvwrite(outitems))+'\n').encode('UTF8'))

    return makeindexes(outitems)

###################################################################

def displayeditdetail(theform, filename=None, msg=None):
    """This is called when the user hits 'edit detail' on a page.
    It displays the edit detail page.
    If the user has admin rights this includes the option of deleting a page.
    
    It can also be called if an invalid change is attempted.
    """
    from listquote import csvread, elem_quote
    admin = int(userconfig['admin']) >= admindel
    filename = filename or unicode(theform['editdetail'].value, 'UTF8')
    
# we need the title, description and pagename of the page referred to by the filename
# That means we need to load the index_table *and* the page
    thefile = ''
    if os.path.isfile(tablefile):
        thefile = unicode(readfile(tablefile), 'UTF8')
    thetable = [line.strip() for line in thefile.split('\n') if line.strip() and not line[0]=='#']
    items = csvread(thetable)
    #
    ourpage = None
    for pageline in items:
        if pageline[0] == filename:             # XXXX can we *always* be certain of casing and unescaped characters ?
            ourpage = pageline
            break
    if not ourpage:
        error("Sorry, I can't find that page.<br />Very Odd.")
    description = ourpage[2]
    pagename = ourpage[1]

    thepage = unicode(readfile(webpath+filename), 'UTF8')
    tstartindex = thepage.lower().find('<title>') + 7                   # XXXX what if this is -1
    tendindex = thepage.lower().find('</title>')       # XXXX check no whitespace
    title = thepage[tstartindex:tendindex]

    editform = unicode(readfile(editdetform), 'UTF8')
    editform = editform.replace('**makepage**', thisscript)
    editform = editform.replace('**page title**', title)
    editform = editform.replace('**description**', description)
    editform = editform.replace('**page name**', pagename)
    editform = editform.replace('**the site**', thesite)
    editform = editform.replace('**filename**', filename)
    if admin:
        editform = editform.replace('<!-- **del start**', '')
        editform = editform.replace('**del end** -->', '')

    editpage = unicode(readfile(editdetpage), 'UTF8')
    editpage = editpage.replace('**username**', username)
    editpage = editpage.replace('**edit details table **', editform)
    if msg:
        editpage = editpage.replace('<!-- **message** -->', '<h2>'+msg+'</h2>')

    print serverline
    print '\r'
    print editpage.encode('UTF8')
    
def editdetail(theform):
    """This is called when the user submits an edit detail form.
    It implements the change made.
    If the user has admin rights this can include deleting a page.
    After most changes the indexes will need to be updated.
    """
    from listquote import csvread, elem_quote, csvwrite
    msg = None
    filename = theform['detail'].value  
    if not theform.has_key('doctitle') or not theform['doctitle'].value.strip():
        msg = 'No Title Was Provided'
    elif not theform.has_key('description') or not theform['description'].value.strip():
        msg = 'No Description Was Provided'
    elif not theform.has_key('pagename')or not theform['pagename'].value.strip():
        msg = 'No Pagename Was Provided'

    if msg:
        displayeditdetail(theform, filename, msg)
        sys.exit()
    else:
        doctitle = unicode(theform['doctitle'].value, 'UTF8')             # XXXX should I be using cgi.escape on these values ?
        description = unicode(theform['description'].value, 'UTF8')
        pagename = unicode(theform['pagename'].value, 'UTF8')       
    
    thefile = ''
    if os.path.isfile(tablefile):
        thefile = unicode(readfile(tablefile), 'UTF8')
    thetable = [line.strip() for line in thefile.split('\n') if line.strip() and not line[0]=='#']
    items = csvread(thetable)
    ourpage = None
    titlechange = None
    namechange = None
    deschange = None
    for pageline in items:
        if pageline[0] == filename:
            ourpage = pageline
            if pagename != ourpage[1]:
                namechange = True
                ourpage[1] = pagename
            if description != ourpage[2]:
                deschange = True
                ourpage[2] = description
            break
    if not ourpage:
        error("Sorry, I can't find that page.<br />Very Odd.")
    
    # If the description or title have been modified, they need to be put back into the page
    thepage = unicode(readfile(webpath+filename), 'UTF8')
    tstartindex = thepage.lower().find('<title>') + 7                   # XXXX what if this is -1
    tendindex = thepage.lower().find('</title>')       # XXXX check no whitespace
    title = thepage[tstartindex:tendindex]
    if title != doctitle:
        titlechange = True
        thepage = thepage[:tstartindex] + doctitle + thepage[tendindex:]
    if deschange:
        dstartindex = thepage.lower().find('<meta name="description" content="')
        if dstartindex  != -1:                          # no meta description tag
            dstartindex += 34
            dendindex = thepage.find('"', dstartindex)
        thepage = thepage[:dstartindex] + description + thepage[dendindex:]
    if titlechange or deschange:
        # write the changed page
        writefile(webpath+filename, thepage.encode('UTF8'))
    
    if deschange or titlechange or namechange:
        writefile(tablefile, ('\n'.join(csvwrite(items))+'\n').encode('UTF8'))
        makeindexes(items)
    print serverline
    print
    print getindex().replace('**username**', username.capitalize()) .encode('UTF8')

def deletepage(theform):
    """Called when delete page is selected from the edit detail page.
    It checks the confirm checkbox and admin rights first - then deletes page (or goes back to the edit detail page)
    Then it sorts out the index pages."""
    from listquote import csvread, elem_quote, csvwrite
    if int(userconfig['admin']) < admindel:
        error("Sorry, You're Not Authorised to do That")

    filename = unicode(theform['delete'].value, 'UTF8')
    if not theform.has_key('confirm'):
        displayeditdetail(theform, filename, 'You Must Select the Confirm Box to Delete a Page')
    thefile = ''
    if os.path.isfile(tablefile):
        thefile = unicode(readfile(tablefile), 'UTF8')
    thetable = [line.strip() for line in thefile.split('\n') if line.strip() and not line[0]=='#']
    items = csvread(thetable)
    
    found = None
    i=-1
    while i < len(items)-1:
        i += 1
        pageline = items[i]
        #FIXME: can we *always* be certain of casing and unescaped characters ?
        if pageline[0] == filename:
            del items[i]
            break
    if i == len(items)-1 or not os.path.isfile(webpath+filename):
        error("Sorry, I can't find that page.<br />Very Odd.")
    try:
        os.remove(webpath+filename)
    except IOError:
        error("An error occurred whilst trying to delete.")
    writefile(tablefile, ('\n'.join(csvwrite(items))+'\n').encode('UTF8'))
    makeindexes(items)
    print serverline
    print
    print getindex().replace('**username**', username.capitalize()).encode('UTF8') 

################################################################

def addlinks(thepage):
    """Add the links section to the top of each page."""
    linkstart = '\n<!-- **links** -->\n'
    linkend = '\n<!-- **/links** -->\n'
    thelinks = backlink % (webloc + 'index.html')
    thelinks = '<body>'+linkstart + thelinks + linkend
    bodyindex = thepage.find('<body')
    if bodyindex == -1:
        return thepage.replace('</body>', thelinks)
    while bodyindex < len(thepage) -1:
        if thepage[bodyindex] == '>':
            break
        bodyindex  += 1
    if bodyindex == len(thepage) -1:
        return thepage.replace('</body>', thelinks)
    bodyindex += 1
    return thepage[:bodyindex] + thelinks + thepage[bodyindex:]

def removelinks(thepage):
    """Strip out the link section from loaded pages."""
    startindex = thepage.find('\n<!-- **links** -->\n')
    if startindex == -1: return thepage 
    endindex = thepage.find('\n<!-- **/links** -->\n')
    return thepage[:startindex]+thepage[endindex+21:]

#####################################################################

if __name__ == '__main__':
    # If makepage is called as __main__ a 'site' parameter should be specified
    # or the ``defaultsite`` will be used
    theform = cgi.FieldStorage()
    if not theform.has_key('site'):
        thissite = defaultsite
    else:
        thissite = theform['site'].value
    readconfig(thissite, thisscript)
    main(theform)

"""
ISSUES
======

Globals in the config section are horrible :-)


CHANGELOG
=========

2005/09/08
----------

Changes to bring compatibility with ConfigObj 4.0.0, and the other changes from
pythonutils 0.2.0

Code and comments cleanup.

"""
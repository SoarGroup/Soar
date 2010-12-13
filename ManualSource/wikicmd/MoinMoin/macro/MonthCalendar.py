"""
    MoinMoin - MonthCalendar Macro

    You can use this macro to put a month's calendar page on a Wiki page.

    The days are links to Wiki pages following this naming convention:
    BasePageName/year-month-day

    @copyright: 2002-2009 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.

    Revisions:
    * first revision without a number (=1.0):
        * was only online for a few hours and then replaced by 1.1
    * 1.1:
        * changed name to MonthCalendar to avoid conflict with "calendar" under case-insensitive OSes like Win32
        * days as subpages
        * basepage argument
        * change order of year/month argument
        * browsing links to prev/next month/year
            * current limitation: you can only browse one calendar on the same
              page/url, if you try to browse another calendar on the same page,
              the first one jumps back to its original display
    * show basepage in calendar header if basepage<>currentpage
    * 1.2:
        * minor fixes in argument parsing
        * cosmetic fix for netscape, other cosmetic changes, changed css
        * i18n support (weekday short names)
    * 1.3:
        * fixes to run with MoinMoin 0.11, thanks to JuergenHermann
        * fix: withspace before "," allowed in argument list
        * BasePage in calendar header (if present) is a link now
        * more i18n
        * HTML cleanup, generating code avoids bracketing errors
        * colour cosmetics
    * 1.4:
        * new parameter for enabling fixed height of 6 "calendar weeks",
          if you want to show a whole year's calendar, this just looks
          better than having some months with 4, some with 5 and some with 6.
        * group calendaring functions:
          * you can give mutliple BasePages UserName1*UserName2*UserName3
          * first BasePage is considered "your" Basepage,
            used days are bright red
          * 2nd and all other BasePages are considered "others" BasePages
            and lead to an increasing green component the more "used" days
            the others have. So white gets greener and red gets more yellowish.
          * in the head part of the calendar, you can click on each name
            to get to the Page of the same name
          * colouring of my and others BasePage is done in a way to show
            the colouring used in the calendar:
          * others use green colouring (increasingly green if multiply used)
          * I use red colouring, which gets more and more yellowish as
            the day is used by more and more others, too
    * 1.5:
        * fixed username colouring when using a BasePage
        * fixed navigation header of MonthCalendar not to get broken into
          multiple lines
        * fixed SubPage handling (please do not use relative SubPages like
          /SubPage yet. Use MyName/SubPage.)
    * 1.6:
        * syntactic cleanup
        * removed i18n compatibility for moin<1.1 or cvs<2003-06-10
        * integrated Scott Chapman's changes:
            * Made it configurable for Sunday or Monday as the first day of the week.
              Search for "change here".
            * Made it so that today is not only set to a seperate css style, but also boldfaced.
              Some browsers don't show the other css style (Netscape).
            * Made it so weekend dates have different color.
    * 1.7:
        * added request parameter where needed
    * 1.8:
        * some fixes for moin 1.2 (use this version ONLY if you run moin 1.2, too):
            * .value backtrace fixed when selecting next/prev month/year
            * request param added to macro function
    * 1.9:
        * adapted to moin 1.3
    * 2.0:
        * integrated into moin 1.3
        * added some nice JS (thanks to Klaus Knopper) to show nice mouseovers
          showing a preview of the day page linked (use first level headlines
          to make entries)
        * merged "common navigation" change of OliverGraf
        * merged AnnualMonthlyCalendar change of JonathanDietrich
    * 2.1:
        * fixed CSS for IE users
        * fix javascript for IE4
        * do a correct calculation of "today" using user's timezone
    * 2.2:
        * added template argument for specifying an edit template for new pages
    * 2.3:
        * adapted to moin 1.7 new macro parameter parsing

    Usage:
        <<MonthCalendar(BasePage,year,month,monthoffset,monthoffset2,height6,anniversary,template)>>

        each parameter can be empty and then defaults to currentpage or currentdate or monthoffset=0

    Samples (paste that to one of your pages for a first try):

Calendar of current month for current page:
<<MonthCalendar>>

Calendar of last month:
<<MonthCalendar(,,,-1)>>

Calendar of next month:
<<MonthCalendar(,,,+1)>>

Calendar of Page SampleUser, this years december:
<<MonthCalendar(SampleUser,,12)>>

Calendar of current Page, this years december:
<<MonthCalendar(,,12)>>

Calendar of December, 2001:
<<MonthCalendar(,2001,12)>>

Calendar of the month two months after December, 2001
(maybe doesn't make much sense, but is possible)
<<MonthCalendar(,2001,12,+2)>>

Calendar of year 2002 (every month padded to height of 6):
||||||Year 2002||
||<<MonthCalendar(,2002,1,,,1)>>||<<MonthCalendar(,2002,2,,,1)>>||<<MonthCalendar(,2002,3,,,1)>>||
||<<MonthCalendar(,2002,4,,,1)>>||<<MonthCalendar(,2002,5,,,1)>>||<<MonthCalendar(,2002,6,,,1)>>||
||<<MonthCalendar(,2002,7,,,1)>>||<<MonthCalendar(,2002,8,,,1)>>||<<MonthCalendar(,2002,9,,,1)>>||
||<<MonthCalendar(,2002,10,,,1)>>||<<MonthCalendar(,2002,11,,,1)>>||<<MonthCalendar(,2002,12,,,1)>>||

Current calendar of me, also showing entries of A and B:
<<MonthCalendar(MyPage*TestUserA*TestUserB)>>

SubPage calendars:
<<MonthCalendar(MyName/CalPrivate)>>
<<MonthCalendar(MyName/CalBusiness)>>
<<MonthCalendar(MyName/CalBusiness*MyName/CalPrivate)>>


Anniversary Calendars: (no year data)
<<MonthCalendar(Yearly,,,+1,,6,1)>>

This creates calendars of the format Yearly/MM-DD
By leaving out the year, you can set birthdays, and anniversaries in this
calendar and not have to re-enter each year.

This creates a calendar which uses MonthCalendarTemplate for directly editing
nonexisting day pages:
<<MonthCalendar(,,,,,,,MonthCalendarTemplate)>>
"""

Dependencies = ['namespace', 'time', ]

import re, calendar, time

from MoinMoin import wikiutil
from MoinMoin.Page import Page

# The following line sets the calendar to have either Sunday or Monday as
# the first day of the week. Only SUNDAY or MONDAY (case sensitive) are
# valid here.  All other values will not make good calendars.
# If set to Sunday, the calendar is displayed at "March 2003" vs. "2003 / 3" also.
# XXX change here ----------------vvvvvv
calendar.setfirstweekday(calendar.MONDAY)

def cliprgb(r, g, b):
    """ clip r,g,b values into range 0..254 """
    def clip(x):
        """ clip x value into range 0..254 """
        if x < 0:
            x = 0
        elif x > 254:
            x = 254
        return x
    return clip(r), clip(g), clip(b)

def yearmonthplusoffset(year, month, offset):
    """ calculate new year/month from year/month and offset """
    month += offset
    # handle offset and under/overflows - quick and dirty, yes!
    while month < 1:
        month += 12
        year -= 1
    while month > 12:
        month -= 12
        year += 1
    return year, month

def parseargs(request, args, defpagename, defyear, defmonth, defoffset, defoffset2, defheight6, defanniversary, deftemplate):
    """ parse macro arguments """
    args = wikiutil.parse_quoted_separated(args, name_value=False)
    args += [None] * 8 # fill up with None to trigger defaults
    parmpagename, parmyear, parmmonth, parmoffset, parmoffset2, parmheight6, parmanniversary, parmtemplate = args[:8]
    parmpagename = wikiutil.get_unicode(request, parmpagename, 'pagename', defpagename)
    parmyear = wikiutil.get_int(request, parmyear, 'year', defyear)
    parmmonth = wikiutil.get_int(request, parmmonth, 'month', defmonth)
    parmoffset = wikiutil.get_int(request, parmoffset, 'offset', defoffset)
    parmoffset2 = wikiutil.get_int(request, parmoffset2, 'offset2', defoffset2)
    parmheight6 = wikiutil.get_bool(request, parmheight6, 'height6', defheight6)
    parmanniversary = wikiutil.get_bool(request, parmanniversary, 'anniversary', defanniversary)
    parmtemplate = wikiutil.get_unicode(request, parmtemplate, 'template', deftemplate)

    # multiple pagenames separated by "*" - split into list of pagenames
    parmpagename = re.split(r'\*', parmpagename)

    return parmpagename, parmyear, parmmonth, parmoffset, parmoffset2, parmheight6, parmanniversary, parmtemplate

def execute(macro, text):
    request = macro.request
    formatter = macro.formatter
    _ = request.getText

    # return immediately if getting links for the current page
    if request.mode_getpagelinks:
        return ''

    currentyear, currentmonth, currentday, h, m, s, wd, yd, ds = request.user.getTime(time.time())
    thispage = formatter.page.page_name
    # does the url have calendar params (= somebody has clicked on prev/next links in calendar) ?
    if 'calparms' in macro.request.args:
        has_calparms = 1 # yes!
        text2 = macro.request.args['calparms']
        cparmpagename, cparmyear, cparmmonth, cparmoffset, cparmoffset2, cparmheight6, cparmanniversary, cparmtemplate = \
            parseargs(request, text2, thispage, currentyear, currentmonth, 0, 0, False, False, u'')
        # Note: cparmheight6 and cparmanniversary are not used, they are just there
        # to have a consistent parameter string in calparms and macro args
    else:
        has_calparms = 0

    if text is None: # macro call without parameters
        text = u''

    # parse and check arguments
    parmpagename, parmyear, parmmonth, parmoffset, parmoffset2, parmheight6, anniversary, parmtemplate = \
        parseargs(request, text, thispage, currentyear, currentmonth, 0, 0, False, False, u'')

    # does url have calendar params and is THIS the right calendar to modify (we can have multiple
    # calendars on the same page)?
    #if has_calparms and (cparmpagename,cparmyear,cparmmonth,cparmoffset) == (parmpagename,parmyear,parmmonth,parmoffset):

    # move all calendars when using the navigation:
    if has_calparms and cparmpagename == parmpagename:
        year, month = yearmonthplusoffset(parmyear, parmmonth, parmoffset + cparmoffset2)
        parmoffset2 = cparmoffset2
        parmtemplate = cparmtemplate
    else:
        year, month = yearmonthplusoffset(parmyear, parmmonth, parmoffset)

    if request.isSpiderAgent and abs(currentyear - year) > 1:
        return '' # this is a bot and it didn't follow the rules (see below)
    if currentyear == year:
        attrs = {}
    else:
        attrs = {'rel': 'nofollow' } # otherwise even well-behaved bots will index forever

    # get the calendar
    monthcal = calendar.monthcalendar(year, month)

    # european / US differences
    months = ('January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December')
    # Set things up for Monday or Sunday as the first day of the week
    if calendar.firstweekday() == calendar.MONDAY:
        wkend = (5, 6)
        wkdays = ('Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun')
    if calendar.firstweekday() == calendar.SUNDAY:
        wkend = (0, 6)
        wkdays = ('Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat')

    colorstep = 85
    p = Page(request, thispage)
    qpagenames = '*'.join([wikiutil.quoteWikinameURL(pn) for pn in parmpagename])
    qtemplate = wikiutil.quoteWikinameURL(parmtemplate)
    querystr = "calparms=%%s,%d,%d,%d,%%d,,,%%s" % (parmyear, parmmonth, parmoffset)
    prevlink = p.url(request, querystr % (qpagenames, parmoffset2 - 1, qtemplate))
    nextlink = p.url(request, querystr % (qpagenames, parmoffset2 + 1, qtemplate))
    prevylink = p.url(request, querystr % (qpagenames, parmoffset2 - 12, qtemplate))
    nextylink = p.url(request, querystr % (qpagenames, parmoffset2 + 12, qtemplate))

    prevmonth = formatter.url(1, prevlink, 'cal-link', **attrs) + '&lt;' + formatter.url(0)
    nextmonth = formatter.url(1, nextlink, 'cal-link', **attrs) + '&gt;' + formatter.url(0)
    prevyear = formatter.url(1, prevylink, 'cal-link', **attrs) + '&lt;&lt;' + formatter.url(0)
    nextyear = formatter.url(1, nextylink, 'cal-link', **attrs) + '&gt;&gt;' + formatter.url(0)

    if parmpagename != [thispage]:
        pagelinks = ''
        r, g, b = (255, 0, 0)
        l = len(parmpagename[0])
        steps = len(parmpagename)
        maxsteps = (255 / colorstep)
        if steps > maxsteps:
            steps = maxsteps
        chstep = int(l / steps)
        st = 0
        while st < l:
            ch = parmpagename[0][st:st+chstep]
            r, g, b = cliprgb(r, g, b)
            link = Page(request, parmpagename[0]).link_to(request, ch,
                        rel='nofollow',
                        style='background-color:#%02x%02x%02x;color:#000000;text-decoration:none' % (r, g, b))
            pagelinks = pagelinks + link
            r, g, b = (r, g+colorstep, b)
            st = st + chstep
        r, g, b = (255-colorstep, 255, 255-colorstep)
        for page in parmpagename[1:]:
            link = Page(request, page).link_to(request, page,
                        rel='nofollow',
                        style='background-color:#%02x%02x%02x;color:#000000;text-decoration:none' % (r, g, b))
            pagelinks = pagelinks + '*' + link
        showpagename = '   %s<BR>\n' % pagelinks
    else:
        showpagename = ''
    if calendar.firstweekday() == calendar.SUNDAY:
        resth1 = '  <th colspan="7" class="cal-header">\n' \
                 '%s' \
                 '   %s&nbsp;%s&nbsp;<b>&nbsp;%s&nbsp;%s</b>&nbsp;%s\n&nbsp;%s\n' \
                 '  </th>\n' % (showpagename, prevyear, prevmonth, months[month-1], str(year), nextmonth, nextyear)
    if calendar.firstweekday() == calendar.MONDAY:
        resth1 = '  <th colspan="7" class="cal-header">\n' \
                 '%s' \
                 '   %s&nbsp;%s&nbsp;<b>&nbsp;%s&nbsp;/&nbsp;%s</b>&nbsp;%s\n&nbsp;%s\n' \
                 '  </th>\n' % (showpagename, prevyear, prevmonth, str(year), month, nextmonth, nextyear)
    restr1 = ' <tr>\n%s </tr>\n' % resth1

    r7 = range(7)
    restd2 = []
    for wkday in r7:
        wday = _(wkdays[wkday])
        if wkday in wkend:
            cssday = "cal-weekend"
        else:
            cssday = "cal-workday"
        restd2.append('  <td class="%s">%s</td>\n' % (cssday, wday))
    restr2 = ' <tr>\n%s </tr>\n' % "".join(restd2)

    if parmheight6:
        while len(monthcal) < 6:
            monthcal = monthcal + [[0, 0, 0, 0, 0, 0, 0]]

    maketip_js = []
    restrn = []
    for week in monthcal:
        restdn = []
        for wkday in r7:
            day = week[wkday]
            if not day:
                restdn.append('  <td class="cal-invalidday">&nbsp;</td>\n')
            else:
                page = parmpagename[0]
                if anniversary:
                    link = "%s/%02d-%02d" % (page, month, day)
                else:
                    link = "%s/%4d-%02d-%02d" % (page, year, month, day)
                daypage = Page(request, link)
                if daypage.exists() and request.user.may.read(link):
                    csslink = "cal-usedday"
                    query = {}
                    r, g, b, u = (255, 0, 0, 1)
                    daycontent = daypage.get_raw_body()
                    header1_re = re.compile(r'^\s*=\s(.*)\s=$', re.MULTILINE) # re.UNICODE
                    titletext = []
                    for match in header1_re.finditer(daycontent):
                        if match:
                            title = match.group(1)
                            title = wikiutil.escape(title).replace("'", "\\'")
                            titletext.append(title)
                    link = wikiutil.escape(link).replace("'", "\\'")
                    tipname = link
                    tiptitle = link
                    tiptext = '<br>'.join(titletext)
                    maketip_js.append("maketip('%s','%s','%s');" % (tipname, tiptitle, tiptext))
                    attrs = {'onMouseOver': "tip('%s')" % tipname,
                             'onMouseOut': "untip()"}
                else:
                    csslink = "cal-emptyday"
                    if parmtemplate:
                        query = {'action': 'edit', 'template': parmtemplate}
                    else:
                        query = {}
                    r, g, b, u = (255, 255, 255, 0)
                    if wkday in wkend:
                        csslink = "cal-weekend"
                    attrs = {'rel': 'nofollow'}
                for otherpage in parmpagename[1:]:
                    otherlink = "%s/%4d-%02d-%02d" % (otherpage, year, month, day)
                    otherdaypage = Page(request, otherlink)
                    if otherdaypage.exists():
                        csslink = "cal-usedday"
                        if u == 0:
                            r, g, b = (r-colorstep, g, b-colorstep)
                        else:
                            r, g, b = (r, g+colorstep, b)
                r, g, b = cliprgb(r, g, b)
                style = 'background-color:#%02x%02x%02x' % (r, g, b)
                fmtlink = formatter.url(1, daypage.url(request, query), csslink, **attrs) + str(day) + formatter.url(0)
                if day == currentday and month == currentmonth and year == currentyear:
                    cssday = "cal-today"
                    fmtlink = "<b>%s</b>" % fmtlink # for browser with CSS probs
                else:
                    cssday = "cal-nottoday"
                restdn.append('  <td style="%s" class="%s">%s</td>\n' % (style, cssday, fmtlink))
        restrn.append(' <tr>\n%s </tr>\n' % "".join(restdn))

    restable = '<table border="2" cellspacing="2" cellpadding="2">\n<col width="14%%" span="7">%s%s%s</table>\n'
    restable = restable % (restr1, restr2, "".join(restrn))

    if maketip_js:
        tip_js = '''<script language="JavaScript" type="text/javascript">
<!--
%s
// -->
</script>
''' % '\n'.join(maketip_js)
    else:
        tip_js = ''

    result = """\
<script type="text/javascript" src="%s/common/js/infobox.js"></script>
<div id="%s" style="position:absolute; visibility:hidden; z-index:20; top:-999em; left:0px;"></div>
%s%s
""" % (request.cfg.url_prefix_static, formatter.make_id_unique('infodiv'), tip_js, restable)
    return formatter.rawHTML(result)

# EOF


# 01-02-04
#v1.0.2

#
# Date Utils
# By Fuzzyman see www.voidspace.org.uk/atlantibots/pythonutils.html
# Written for the Victory Day program for Jesus Fellowship Church 
# www.jesus.org.uk

# These are various functions for dealing with dates (including leap years and so on)
# Useful especially for situations where you have to arrange appointments.
# (e.g. second Tuesday of the month etc...)

# None of these functions are designed to handle BC dates.........
# They will also only work with dates from the Gregorian (modern) calender.
# They usually assume that given dates are *possible* dates.
# (Although there is a function to explicitly check a date).

# Help and inspiration was taken from :
# http://users.aol.com/s6sj7gt/mikecal.htm and 
# http://mathforum.org/library/drmath/view/62338.html

# If you have any bug reports or suggestions please contact me.
# If you would like to be notified of bug fixes / updates then please contact me.

# E-mail fuzzyman AT atlantibots DOT org DOT uk (or michael AT foord DOT me DOT uk )
# Code maintained at http://www.voidspace.org.uk/atlantibots/pythonutils.html

# Copyright Michael Foord
# Not for use in commercial projects without permission.
# If you use them in a non-commercial project then please credit me and include a link back.
# If you release the project non-commercially then let me know (and include this message with my code !)

# No warranty express or implied for the accuracy, fitness to purpose or otherwise for this code....
# Use at your own risk !!!

from time import localtime

##############################

# First set up some useful values

monthslower = [ 'january', 'february', 'march', 'april', 'may', 'june', 'july',
          'august', 'september', 'october', 'november', 'december' ]

dayslower =[ 'sunday', 'monday', 'tuesday', 'wednesday', 'thursday', 'friday',
        'saturday' ]

monthdict = { 'january' : 31, 'february' : 28, 'march' : 31, 'april' : 30, 'may' : 31,
              'june' : 30, 'july' : 31, 'august' : 31, 'september' : 30, 'october' : 31,
              'november' : 30, 'december' : 31 }

monthdictleap = { 'january' : 31, 'february' : 29, 'march' : 31, 'april' : 30, 'may' : 31,
              'june' : 30, 'july' : 31, 'august' : 31, 'september' : 30, 'october' : 31,
              'november' : 30, 'december' : 31 }

monthlist = [ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 ]

monthlistleap = [ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 ]

days =[ 'Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday',
        'Saturday' ]

months = [ 'January', 'February', 'March', 'April', 'May', 'June', 'July',
          'August', 'September', 'October', 'November', 'December' ]

#############################

# Next the functions

"""
There are various useful 'constants' defined in dateutils :

monthslower, dayslower = lowercase lists of the days and months
monthdict, monthdictleap = dictionaries keyed by month - value is the number of days in the month (monthdictleap is for a leap year)
monthlist, monthlistleap = a list of the number of days in the month (monthlistleap is for a leapyear)
days, months = capitalised lists of the days and months
dateformcon = a dictionary with the standard config settings for the formatted date function.

The Following functions are defined in dateutils :

(Some of the functions depend on each other - so it's better to import the ones you want rather than cut and paste :-)

realdate(day, month, year):
    Returns true if the supplied date is a possible date
    and False if it isn't :-) (Note - it only tests that the *year* is greater than zero).

isleapyear(year):
    Given a year as an integer (e.g. 2004) it returns True if the year is a leap year,
    and False if it isn't.

daysinmonth(year, month):
    Given a year and a month it returns how many days are in that month.
    
datetoday(day, month, year):
    Passed in a date, in integers, it returns the day of the week.
    Output is expressed as an integer from 0-6.
    0 is Sunday, 1 is Monday.......
    
datestringtoints(datestring):
    Passed in a datestring - in the form 'yyyymmdd'
    (e.g. 20040122 being 22nd January 2004) -
    it returns an integer tuple ( yyyy, mm, dd ).
    If the datestring is of the wrong length it returns None.
    (It assumes a four figure year).

intstodatestring(day, month, year):
    Given three integers for day, month and year
    it returns a datestring 'yyyymmdd' (for easy storage).
    
returndate():
    Returns the local date using the localtime function
    from the time module.
    Returns integers - ( yyyy, mm, dd ).

nearestday(day, month, year, dayofweek = 2, afteronly = 0):
    Given a date as three integers (year, month and day) it returns the nearest
    date that is 'dayofweek'. (dayofweek should be an integer from 0 - 6. 0 is Sunday, 1 Monday etc..)
    If afteronly is set to 1 then it finds the nearest date of that day, on or *after* the specified.
    Returns integers - ( yyyy, mm, dd ).
    dayofweek defaults to Tuesday (2) and afteronly defaults to 0 as they are the defaults I'm using  for the Victory Day program this is written for.
    This is used for : e.g find the nearest Tuesday to a given date, or find the nearest Tuesday *after* a given date !

addnumdays(day, month, year, modifier):
    Given a date as three integers (year, month and day) and a number of days to add or subtract
    to that date (the integer modifier, positive or negative value) - it returns the correct date
    as a tuple of integers - ( yyyy, mm, dd ).   

incdate(day, month, year):
    Given a date it adds one day to the date and returns the new date.

decdate(day, month, year):
    Given a date it subtracts one day from the date and returns the new date.

adddate(day1, month1, year1, day2, month2, year2):
    Given a date as three integers (year1, month1 and day1) and another number of days (day2), months (month2)
    and years (year2) to add to that date (or subtract from it) - it returns the new date as a tuple of integers - ( yyyy, mm, dd ).
    Note :
    Feb 28th + 1 month = March 31st
    Feb 29th  + 1 month = March 31st
    January 29th to 31st + 1 month = feb 28th/29th
    August 31st + 1 month = September 30th
    We add the years together, then the months, then correct for the 'end of month' (e.g. we change Sep 31st to Sep 30th)
    Finally we add any extra days on.    

daycount(year, month, day)
    This is an implementation of the Julian Day system. This 
    is a continuous count of days from January 1, 4713 B.C.
    Given a date in in integers it returns an integer value for the date
    This represents it's Julian Day number as above.
    This only works for dates represented using the the Gregorian
    calendar which was adopted in the US/UK on Oct. 15, 1582 - but
    at different times elsewhere (so historical dates may not be in this system....).

counttodate(daycount)
    Given the number for a date using the Julian Day System,
    it returns that date as integer tuple (year, month, day).

daysbetween(day1, month1, year1, day2, month2, year2)
    Given two dates it returns the number of days between them.
    If date1 is earlier than date2 then the result will be positive.

def dayfinish(day)
    Takes an integer day and returns the correct finish for it
    1 = 'st', 2 = 'nd', 3 = 'rd', 4-10 = 'th' etc....

def formatteddate(day, month, year, configdict = {}, **configs)
    Given a date in in integers, it returns the date as a nicely formatted string :
    e.g. 24th January 1997 or 2nd February 1948
    configs accepts the following keywords :
    dayofweek, addzero, addcom, fullstop, monthfirst
    e.g. print formatteddate(12, 8, 1974, dayofweek=1, addzero=0, addcom=1, fullstop=1, monthfirst=0)
         Monday 12th August, 1974.
    If dayofweek is set to 1 then the day of the week will also be printed :
    e.g. Monday 24th January 1997
    If addzero is set to 1 then days 1-9 will have an additional zero :
    e.g. 02nd February 1948
    If addcom is set to 1 then there will be a comma between the month and the year :
    e.g. 24th January, 1997
    If fullstop is set to 1 then there will be a fullstop after the year :
    e.g. 24th January 1997.
    If monthfirst is set to 1 then then the month will be put before the day :
    e.g. January 24th 1997
    If the year is set to zero then it will be missed off.
    (and the dayofweek will be treated as 0 in this case as well).
    There is a dictionary called dateformcon defined in the dateutils module with all the config values
    defined and some good  standard settings :-)
    This dictionary can be passed in instead of the individual settings.
    """

#############################


def realdate(day, month, year):
    """Returns true if the supplied date is a possible date
    and False if it isn't :-) (Note - it *only* tests that the year is greater than zero)."""
    if  month > 12 or year < 1 or day < 1 or month < 1:
        return False
    elif  month == 2:      # if it's february we need to know if it's a leap year
        if isleapyear(year):
            numdays = 29
        else:
            numdays = 28
    else:
        numdays = monthlist[ month-1 ]        # -1 because in the list January is 0
    if day > numdays:
        return False
    else:
        return True


def isleapyear(year):
    """Given a year as an integer (e.g. 2004) it returns True if the year is a leap year,
    and False if it isn't."""
    if year%4 != 0:
        return False
    elif year%100 !=0:
        return True
    elif year%400 == 0:
        return True
    else:
        return False


def daysinmonth(year, month):
    """Given a year and a month it returns how many days are in that month."""
    if month == 2:      # if it's february we need to know if it's a leap year
        if isleapyear(year):
            numdays = 29
        else:
            numdays = 28
    else:
        numdays = monthlist[ month-1 ]        # -1 because in the list January is 0
    return numdays


def datetoday(day, month, year):
    """Passed in a date, in integers, it returns the day of the week.
    Output is expressed as an integer from 0-6.
    0 is Sunday, 1 is Monday....... """
    # dayofweek would have been a better name for this function :-(
    d = day
    m = month
    y = year
    if m < 3:
        z = y-1
    else:
        z = y
    dayofweek = ( 23*m//9 + d + 4 + y + z//4 - z//100 + z//400 )
    if m >= 3:
        dayofweek -= 2
    dayofweek = dayofweek%7
    return dayofweek

def datestringtoints(datestring):
    """Passed in a datestring - in the form 'yyyymmdd'
    (e.g. 20040122 being 22nd January 2004) -
    it returns an integer tuple ( yyyy, mm, dd ).
    If the datestring is of the wrong length it returns None.
    (It assumes a four figure year)."""
    if len(datestring) != 8:        # badly formed datestring
        return None
    return (int(datestring[:4]), int(datestring[4:6]), int(datestring[6:8]))

def intstodatestring(day, month, year):
    """Given three integers for day, month and year
    it returns a datestring 'yyyymmdd' (for easy storage)."""
    y = str(year)
    while len(y) < 4:
        y = '0' + y
    m = str(month)
    d = str(day)
    if len(m) < 2:
        m = '0' + m
    if len(d) < 2:
        d = '0' + d
    return y+m+d


def returndate():
    """Returns the local date using the localtime function
    from the time module.
    Returns integers - ( yyyy, mm, dd )."""
    try:      # because this function doesn't work on some platforms
        datetuple = localtime()
    except:
        return (2004, 1, 31)
    return ( datetuple[0], datetuple[1], datetuple[2] )

def nearestday(day, month, year, dayofweek = 2, afteronly = 0):
    """Given a date as three integers (year, month and day) it returns the nearest
    date that is 'dayofweek'. (dayofweek should be an integer from 0 - 6. 0 is Sunday, 1 Monday etc..)
    If afteronly is set to 1 then it finds the nearest date of that day, on or *after* the specified.
    Returns integers - ( yyyy, mm, dd ).
    dayofweek defaults to Tuesday (2) and afteronly defaults to 0 as they are the defaults I'm using  for the Victory Day program this is written for.
    This is used for : e.g find the nearest Tuesday to a given date, or find the nearest Tuesday *after* a given date !"""
    thisday = datetoday(day, month, year)
    if thisday == dayofweek:
        return (year, month, day)
    
    if thisday < dayofweek:     # this 'if else test' tells us the number of days between the two days of the week
        forward = dayofweek - thisday
        backward = 7 - forward
    else:
        backward = thisday - dayofweek
        forward = 7 - backward
    if afteronly or forward < backward:
        difference = forward
    else:
        difference = -backward
        
    return addnumdays(day, month, year, difference)
        

def addnumdays(day, month, year, modifier):
    """Given a date as three integers (year, month and day) and a number of days to add or subtract
    to that date (the integer modifier, positive or negative value) - it returns the correct date
    as a tuple of integers - ( yyyy, mm, dd )."""
    if modifier > 0:    # damn - different rules for negative modifiers and hard to make generic
        if month == 2 and isleapyear(year) and day == 29:     # special case
            modifier -= 1
            month = 3
            day = 1
        while modifier >= 365:      # add any years on
            if month <= 2 and isleapyear(year) or month > 2  and isleapyear(year+1):
                numdays = 366
            else:
                numdays = 365            
            if modifier >= numdays:
                year += 1
                modifier -= numdays
            else:
                break
                
        while modifier >= 28:   #add any full months on     
            if month == 2:      # if it's february we need to know if it's a leap year
                if isleapyear(year):
                    numdays = 29
                else:
                    numdays = 28
            else:
                numdays = monthlist[ month-1 ]        # -1 because in the list January is 0
            if modifier >= numdays:
                modifier -= numdays
                if month != 12:
                    month += 1
                else:
                    month = 1
                    year += 1
            else:
                break
# now we need to correct if the new 'day' value is greater than the number of days in the new month...... 
            if month == 2:      # if it's february we need to know if it's a leap year
                if isleapyear(year):
                    numdays = 29
                else:
                    numdays = 28
            else:
                numdays = monthlist[ month-1 ]        # -1 because in the list January is 0
            if day > numdays:
                if month != 12:
                    month += 1
                else:
                    month = 1
                    year += 1
                day = day - numdays

        while modifier > 0:
            year, month, day = incdate(day, month, year)
            modifier -= 1

    elif modifier < 0:   # we have to subtract days
        modifier = -modifier        # easier to deal with positive numbers :-)
        if month == 2 and isleapyear(year) and day == 29:     # special case
            modifier -= 1
            day = 28
        while modifier >= 365:      # take any years off
            if month > 2 and isleapyear(year) or month <= 2 and isleapyear(year-1):
                numdays = 366
            else:
                numdays = 365            
            if modifier >= numdays:
                year -= 1
                modifier -= numdays
            else:
                break
            
        while modifier >= 28:   # subtract any full months on
            if month == 2:
                if isleapyear(year):
                    numdays = 29
                else:
                    numdays = 28
            else:
                numdays = monthlist[month-1]
            adjuster = numdays - day        # how many days before the end of the month is it
            if day > numdays:
                modifier -= numdays
                if month != 1:
                    month -=1
                else:
                    month = 12
                    year -= 1
                if month == 2:
                    if isleapyear(year):
                        numdays = 29
                    else:
                        numdays = 28
                else:
                    numdays = monthlist[month-1]
                day = numdays - adjuster        # if we've gone back a whole month it's now the smae numebr of days before the end of the month
            else:
                break
            
        while modifier > 0:
            year, month, day = decdate(day, month, year)
            modifier -= 1         

    return ( year, month, day )
    

def incdate(day, month, year):
    """Given a date it adds one day to the date and returns the new date."""
    if month == 2:      # if it's february we need to know if it's a leap year
        if isleapyear(year):
            numdays = 29
        else:
            numdays = 28
    else:
        numdays = monthlist[ month-1 ]        # -1 because in the list January is 0
    if day < numdays:
        day += 1
    else:       # of course, here day should equal numdays or the date is invalid :-)
        if month == 12:
            month = 1
            year +=1
            day = 1
        else:
            month += 1
            day = 1
    return ( year, month, day )

def decdate(day, month, year):
    """Given a date it subtracts one day from the date and returns the new date."""
    if day > 1:
        day -= 1
    elif month == 1:        # 1st January
        year -=1
        day = 31
        month = 12
    elif month == 3:        # 1st March
        if isleapyear(year):
            day = 29
        else:
            day = 28
        month = 2
    else:
        day = monthlist[ month-2 ]
        month -= 1
    return ( year, month, day )

def adddate(day1, month1, year1, day2, month2, year2):
    """Given a date as three integers (year1, month1 and day1) and another number of days (day2), months (month2)
    and years (year2) to add to that date (or subtract from it) - it returns the new date as a tuple of integers - ( yyyy, mm, dd ).
    Note :
    Feb 28th + 1 month = March 31st
    Feb 29th  + 1 month = March 31st
    January 29th to 31st + 1 month = feb 28th/29th
    August 31st + 1 month = September 30th
    We add the years together, then the months, then correct for the 'end of month' (e.g. we change Sep 31st to Sep 30th)
    Finally we add any extra days on."""
    year = year1 + year2
    month = month1 + month2
    while month < 1:
        year -= 1
        month += 12
    while month > 12:
        year += 1
        month -=12
    numdays = daysinmonth(year, month)
    if day1 > numdays:
        day1 = numdays
    if day2 < 0:
        day2 = -day2
        thisfunc = decdate
    else:
        thisfunc = incdate
    while day2 > 0:
        year, month, day1 = thisfunc(day1, month, year)
        day2 -= 1
    return year, month, day1

def daycount(year, month, day):
    """"This is an implementation of the Julian Day system. This 
    is a continuous count of days from January 1, 4713 B.C.
    Given a date in in integers it returns an integer value for the date
    This represents it's Julian Day number as above.
    This only works for dates represented using the the Gregorian
    calendar which was adopted in the US/UK on Oct. 15, 1582 - but
    at different times elsewhere (so historical dates may not be in this system....)."""
    if month < 3:
        year = year - 1
        month = month + 13
    else:
        month = month + 1
    A = int(year/100)
    B = 2 - A + int(A/4)
    return int(365.25*year) + int(30.6001*month) + B + day + 1720995

def counttodate(daycount):
    """Given the number for a date using the Julian Day System,
    it returns that date as integer tuple (year, month, day)."""
# note - slow and badly implemented... but fast enough :-)
    daycount = daycount - 2453030
    return addnumdays(25, 1, 2004, daycount)

def daysbetween(day1, month1, year1, day2, month2, year2):
    """Given two dates it returns the number of days between them.
    If date1 is earlier than date2 then the result will be positive."""
    return daycount(year2, month2, day2) - daycount(year1, month1, day1)

def dayfinish(day):
    """Takes an integer day and returns the correct finish for it
    1 = 'st', 2 = 'nd', 3 = 'rd', 4-10 = 'th' etc...."""
    if day > 3 and day < 21:
        return 'th'     # special cases
    daystr = str(day)
    if len(daystr) > 1:
        daystr = daystr[-1]
    if daystr == '1':
        return 'st'
    elif daystr == '2':
        return 'nd'
    elif daystr == '3':
        return 'rd'
    else:
        return 'th'

def formatteddate(day, month, year, configdict = {}, **configs):
    """Given a date in in integers, it returns the date as a nicely formatted string :
    e.g. 24th January 1997 or 2nd February 1948
    configs accepts the following keywords :
    dayofweek, addzero, addcom, fullstop, monthfirst
    e.g. print formatteddate(12, 8, 1974, dayofweek=1, addzero=0, addcom=1, fullstop=1, monthfirst=0)
         Monday 12th August, 1974.
    If dayofweek is set to 1 then the day of the week will also be printed :
    e.g. Monday 24th January 1997
    If addzero is set to 1 then days 1-9 will have an additional zero :
    e.g. 02nd February 1948
    If addcom is set to 1 then there will be a comma between the month and the year :
    e.g. 24th January, 1997
    If fullstop is set to 1 then there will be a fullstop after the year :
    e.g. 24th January 1997.
    If monthfirst is set to 1 then then the month will be put before the day :
    e.g. January 24th 1997
    If the year is set to zero then it will be missed off.
    (and the dayofweek will be treated as 0 in this case as well).
    There is a dictionary called dateformcon defined in the dateutils module with all the config values
    defined and some good  standard settings :-)
    This dictionary can be passed in instead of the individual settings.
    """
    keywordlist = ['dayofweek', 'addzero', 'addcom', 'fullstop', 'monthfirst']
    if configdict != {} and isinstance(configdict, dict):
        configs = configdict
    for member in keywordlist:
        if not configs.has_key(member):
            configs[member] = 0
    outstring = ''

    if configs['dayofweek'] and year:
        outstring = days[datetoday(day, month, year)] +' '
    if day < 10 and configs['addzero']:
        daystr = '0' + str(day)
    else:
        daystr = str(day)
    if not configs['monthfirst']:
        outstring += daystr + dayfinish(day) + ' ' + months[month-1]
    else:
        outstring += months[month-1] + ' ' + daystr + dayfinish(day) 
    if configs['addcom'] and year:
        outstring += ','
    if year:
        outstring += ' ' + str(year)
    if configs['fullstop']:
        outstring += '.'
    return outstring
    

dateformcon = { 'dayofweek' : 1, 'addzero' : 0, 'addcom' : 1, 'fullstop' : 1, 'monthfirst' : 0 }

############################################################

if __name__ == "__main__":
    print returndate()
    year, month, day = returndate()
    test = daycount(year, month, day)
    print test
    print counttodate(test)
    while True:
        x = raw_input("Enter Year of date (Enter to quit) >> ")
        if x=='':
            break
        y = raw_input("Enter Month >> ")
        z = raw_input("Enter Day >> ")
        test = daycount(int(x), int(y), int(z))
        print test
        print counttodate(test)


    
    print realdate(32, 1, 2004)
    while True:
        x = raw_input("Enter Modifier (0 to quit) >> ")
        if x=='0':
            break
        print addnumdays(31, 3, 2004, -int(x) )

    while True:
        x = raw_input("Enter Day of Week 0-6 (7 to quit) >> ")
        if x=='7':
            break
        print nearestday(24, 1, 2004, int(x))

    while True:
        x = raw_input("Enter Years to Add (Enter to quit) >> ")
        if x=='':
            break
        y = raw_input("Enter Months to Add >> ")
        z = raw_input("Enter Days To Add >> ")
        print adddate(24, 1, 2004, int(z), int(y), int(x))
        year, month , day = adddate(24, 1, 2004, int(z), int(y), int(x))
        print "The nearest Tuesday after that date is ", nearestday(day, month, year)


"""

Versionlog

01-02-04        Version 1.0.2
Corrected bug in intstodatestring.
Created lowercase day list and capitalised month list.
Added formatteddate and dayfinish function.
Put a try: except: catch in returndate - mainly so I can test on the pocketpc.

"""

#13-09-04
# v1.1.5

# **build 3**

# dataenc.py
# password encoding and comparing module
# The strength of the SHA hashing module - in an ascii safe, timestamped format.

# uses a binary to ascii encoding
# and will timestamp the encodings as well.
# (Binary watermarking of data).

# used in a CGI called 'custbase' - to check logins are correct
# (For the CGI to function it needs the users password (or it's SH5 hash) to be encoded into
# each page as a hidden form field. This exposes the encrypted password in the HTML source of each page.
# This module provides functions to interleave a timestamp *into* the hash.
# Even if the encoded 'timestamped hash' is extracted from the HTML source, the CGI can tell
# that the password has expired).


# Contains functions to :

# do binary to ascii encoding using a TABLE mapping (and ascii to binary)
# binary interleave - to disperse one set of binary data into another (e.g. as a 'watermark' or date/time stamp)
# to extract the watermark again 
# convert a decimal value to base 64 digits, and base 64 digits back to 8 bit digits..
# Creating and retrieving a timestamp from the current time/date
# functions for testing and setting bits in a byte (or larger value)
# (including a bitwise operator object that comes from the python cookbook
# and is no longer used here, but included for reference).

# Wrapping all that together to return an ascii encoded, date stamped SHA hash of a string


# Copyright Michael Foord 2004
# dataenc.py
# Functions for encoding and interleaving data.

# http://www.voidspace.org.uk/python/modules.shtml

# Released subject to the BSD License
# Please see http://www.voidspace.org.uk/documents/BSD-LICENSE.txt

# For information about bugfixes, updates and support, please join the Pythonutils mailing list.
# http://voidspace.org.uk/mailman/listinfo/pythonutils_voidspace.org.uk
# Comments, suggestions and bug reports welcome.
# Scripts maintained at http://www.voidspace.org.uk/python/index.shtml
# E-mail fuzzyman@voidspace.org.uk


"""
DOCS for dataenc as a module

When run it should go through a few basic tests - see the function test()

This module provides low-level functions to interleave two bits of data into each other and separate them.
It will also encode this binary data to and from ascii - for inclusion in HTML, cookies or email transmission.

It also provides high level functions to use these functions for time stamping passwords and password hashes,
and also to check that a 'time-stamped hash' is both valid and unexpired.

The check_pass function is interesting. Given an encoded and timestamped hash it compares it with the hash (using SD5) of a password.
If it matches *and* is unexpired (you set the time limit) it returns a new encoded time stamp of the hash with the current time.
I use this for secure, time limited, logins over CGI. (Could be stored in a cookie as well).
(On the first login you will need to compare the password with the stored hash and use that to generate a time stamped hash to include in the page returned.
Thereafter you can just use the check_pass function and include the time-stamped hash in a hidden form field for every action.)

The binary data is interleaved on a 'bitwise' basis - every byte is mangled.

--

CONSTANTS 

The main constant defined in dataenc.py is :

TABLE = '_-0123456789' + \
         'abcdefghijklmnopqrstuvwxyz'+ \
         'NOPQRSTUVWXYZABCDEFGHIJKLM'
TABLE should be exactly 64 printable characters long... or we'll all die horribly
Obviously the same TABLE should be used for decoding as for encoding....
note - changing the order of the TABLE here can be used to change the mapping.
Versions 1.1.2+ of TABLE uses only characters that are safe to pass in URLs
(e.g. using the GET method for passing FORM data)

OLD_TABLE is the previous encoding map used for versions of dataenc.py previous to 1.1.2
See the table_dec function for how to decode data encoded with that map.

PSYCOIN = 1
This decides if we attempt to import psyco or not (the specialising compiler). Set to 0 to not import.
If we attempt but fail to import psyco then this value will be set to 0.

DATEIN = 1
As above but for the dateutils and time module.
We need to import dateutils for the expired and pass_enc functions (amongst others) to work fully.


FUNCTIONS

Following are the docstrings extracted from the public functions :

pass_enc(instring, indict = {}, **keywargs)
    Returns an ascii version of an SHA hash or a string, with the date/time stamped into it.
    e.g. For ascii safe storing of password hashes.

    It also accepts the following keyword args (or a dictionary conatining the following keys).
    (Keywords shown - with default values).
    
    lower = False, sha_hash = False, daynumber = None, timestamp = None, endleave = False

    Setting lower to True makes instring lowercase before hashing/encoding.

    If sha_hash is set to True then instead of the actual string passed in being encoded, it's SHA hash
    is encoded. (In either case the string can contain any binary data).

    If a daynumber is passed in then the daynumber will be encoded into the returned string.
    (daynumber is an integer representing the 'Julian day number' of a date - see the dateutils module).
    This can be used as a 'datestamp' for the generated code and you can detect anyone reusing old codes this way.
    If 'daynumber' is set to True then today's daynumber will automatically be used.
    (dateutils module required - otherwise it will be ignored).

    Max allowed value for daynumber is 16777215 (9th May 41222)
    (so daynumber can be any integer from 1 to 16777215 that you want to 'watermark' the hash with
    could be used as a session ID for a CGI for example).

    If a timestamp is passed in it should either be timestamp = True meanining use 'now'.
    Or it should be a tuple (HOUR, MINUTES).
    HOUR should be an integer 0-23
    MINUTES should be an integer 0-59

    The time and date stamp is *binary* interleaved, before encoding, into the data.

    If endleave is set to True then the timestamp is interleaved more securely. Shouldn't be necessary in practise
    because the stamp is so short and we subsequently encode using table_enc.
    If the string is long this will slow down the process - because we interleave twice.

    
pass_dec(incode)
    Given a string encoded by pass_enc - it returns it decoded.
    It also extracts the datestamp and returns that.
    The return is :
    (instring, daynumber, timestamp)


expired(daynumber, timestamp, validity)
    Given the length of time a password is valid for, it checks if a daynumber/timestamp tuple is
    still valid.
    validity should be an integer tuple (DAYS, HOURS, MINUTES).
    Returns True for valid or False for invalid. 
    Needs the dateutils module to get the current daynumber.

unexpired is an alias for expired - because it makes for better tests.
(The return results from the expired function are logically the wrong way round, expired returns True if the timestamp is *not* expired..)
 
 
check_pass(inhash, pswdhash, EXPIRE)
    Given the hash (possibly from a webpage or cookie) it checks that it is still valid and matches the password it is supposed to have.
    If so it returns a new hash - with the current time stamped into it.
    EXPIRE is a validity tuple to test for (see expired function)
    e.g. (0, 1, 0) means the supplied hash should be no older than 1 hour
    
    If the hash is expired it returns -1.
    If the pass is invalid or doesn't match the supplied pswdhash it returns False.
    This is a high level function that can do all your password checking and 'time-stamped hash' generation after initial login.


makestamp(daynumber, timestamp)
    Receives a Julian daynumber (integer 1 to 16777215) and an (HOUR, MINUTES) tuple timestamp.
    Returns a 5 digit string of binary characters that represent that date/time.
    Can receive None for either or both of these arguments.

    The function 'daycount' in dateutils will turn a date into a daynumber.


dec_datestamp(datestamp)
    Given a 5 character datestamp made by makestamp, it returns it as the tuple :
    (daynumber, timestamp).
    daynumber and timestamp can either be None *or*
    daynumber is an integer between 1 and 16777215
    timestamp is  (HOUR, MINUTES)

    The function 'counttodate' in dateutils will turn a daynumber back into a date.
    

sixbit(invalue)
    Given a value in it returns a list representing the base 64 version of that number.
    Each value in the list is an integer from 0-63...
    The first member of the list is the most significant figure... down to the remainder.
    Should only be used for positive values.


sixtoeight(intuple)
    Given four base 64 (6-bit) digits... it returns three 8 bit digits that represent
    the same value.
    If length of intuple != 4, or any digits are > 63, it returns None.

    **NOTE**
    Not quite the reverse of the sixbit function.
    

table_enc(instring, table=TABLE)
    The actual function that performs TABLE encoding.
    It takes instring in three character chunks (three 8 bit values)
    and turns it into 4 6 bit characters.
    Each of these 6 bit characters maps to a character in TABLE.
    If the length of instring is not divisible by three it is padded with Null bytes.
    The number of Null bytes to remove is then encoded as a semi-random character at the start of the string.
    You can pass in an alternative 64 character string to do the encoding with if you want.


table_dec(instring, table=TABLE)
    The function that performs TABLE decoding.
    Given a TABLE encoded string it returns the original binary data - as a string.
    If the data it's given is invalid (not data encoded by table_enc) it returns None
    (definition of invalid : not consisting of characters in the TABLE or length not len(instring) % 4 = 1).
    You can pass in an alternative 64 character string to do the decoding with if you want.
    

return_now()
    Returns the time now.
    As (HOUR, MINUTES).
    

binleave(data1, data2, endleave = False)
    Given two strings of binary data it interleaves data1 into data2 on a bitwise basis
    and returns a single string combining both. (not just the bytes interleaved).
    The returned string will be 4 bytes or so longer than the two strings passed in.
    Use bin_unleave to return the two strings again.
    Even if both strings passed in are ascii - the result will contain non-ascii characters.
    To keep ascii-safe you must subsequently encode with table_enc.

    Max length for the smallest data string (one string can be of unlimited size) is about 16meg
    (increasing this would be easy if anyone needed it - but would be very slow anyway).

    If either string is empty (or the smallest string greater than 16meg) - we return None.
    The first 4 characters of the string returned 'define' the interleave. (actually the size of the watermark)
    For added safety you could remove this and send seperately.

    Version 1.0.0 used a bf (bitfield) object from the python cookbook. Version 1.1.0 uses the binary and & and or |
    operations and is about 2.5 times faster. On my AMD 3000, leaving and unleaving two 20k files took 1.8 seconds.
    (instead of 4.5 previously - with Psyco enabled this improved to 0.4 seconds.....)

    Interleaving a file with a watermark of pretty much any size makes it unreadable - this is because *every* byte is changed.
    (Except perhaps a few at the end - see the endleave keyword). However it shouldn't be relied on if you need
    a really secure method of encryption. For many purposes it will be sufficient however.
    
    In practise any file not an exact multiple of the size of the watermark will have a chunk at the end that is untouched.
    To get round this you can set endleave = True.. which then releaves the end data back into itself.
    (and therefore takes twice as long - it shouldn't be necessary where you have a short watermark.)

    data2 ought to be the smaller string - or they will be swapped round internally.
    This could cause you to get them back in an unexpected order from binunleave.    


binunleave(data)
    Given a chunk of data woven by binleave - it returns the two seperate pieces of data.
    

For the binary operations of binleave and binunleave, version 1.0.0 used a bf (bitfield) object from
the python cookbook.

class bf(object)
    the bf(object) from activestate python cookbook - by Sebastien Keim - Many Thanks
    http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/113799

Version 1.1.0 replaced these with specific binary AND & and OR | operations that are about 2.5 times faster.
They are 'inline' in the functions for speed (avoiding function calls) but are available separately as well.

def bittest(value, bitindex)
    This function returns the setting of any bit from a value.
    bitindex starts at 0.

def bitset(value, bitindex, bit)
    Sets a bit, specified by bitindex, in in 'value' to 'bit'.
    bit should be 1 or 0



There are also the 'private functions' which actually contain the substance of binleave and binunleave,
You are welcome to 'browse' them - but you shouldn't need to use them directly.


Any comments, suggestions and bug reports welcome.

Regards,

Fuzzy

michael AT foord DOT me DOT uk


"""

import sha
from random import random

DATEIN = 1
if DATEIN:
    try:                # try to import the dateutils and time module
        from time import strftime
        from dateutils import daycount, returndate          # ,counttodate          # counttodate returns a daynumber as a date
    except:
        DATEIN = 0

# If PSYCOON is set to 0 then we won't try and import the psyco module
# IF importing fails, PSYCOON is set to 0
PSYCOON = 1
if PSYCOON:
    try:
        import psyco
        psyco.full()
        from psyco.classes import *
        try:
            psyco.cannotcompile(re.compile)         # psyco hinders rather than helps regular expression compilation
        except NameError:
            pass
    except:
        PSYCOON = 0

# note - changing the order of the TABLE here can be used to change the mapping.
TABLE = '_-0123456789' + \
         'abcdefghijklmnopqrstuvwxyz'+ \
         'NOPQRSTUVWXYZABCDEFGHIJKLM'
# table should be exactly 64 printable characters long... or we'll all die horribly
# Obviously the same TABLE should be used for decoding as for encoding....
# This version of TABLE (v1.1.2) uses only characters that are safe to pass in URLs
# (e.g. using the GET method for passing FORM data)


OLD_TABLE = '!$%^&*()_-+=' + \
         'abcdefghijklmnopqrstuvwxyz'+ \
         'NOPQRSTUVWXYZABCDEFGHIJKLM'
# OLD_TABLE is the old encoding. If anyone has stuff encoded with this then it can be decoded using :
# data = table_dec(encodedstring, OLD_TABLE)


def pass_enc(instring, indict=None, **keywargs):
    """Returns an ascii version of an SHA hash or a string, with the date/time stamped into it.
    e.g. For ascii safe storing of password hashes.

    It also accepts the following keyword args (or a dictionary conatining the following keys).
    (Keywords shown - with default values).
    
    lower = False, sha_hash = False, daynumber = None, timestamp = None, endleave = False

    Setting lower to True makes instring lowercase before hashing/encoding.

    If sha_hash is set to True then instead of the actual string passed in being encoded, it's SHA hash
    is encoded. (In either case the string can contain any binary data).

    If a daynumber is passed in then the daynumber will be encoded into the returned string.
    (daynumber is an integer representing the 'Julian day number' of a date - see the dateutils module).
    This can be used as a 'datestamp' for the generated code and you can detect anyone reusing old codes this way.
    If 'daynumber' is set to True then today's daynumber will automatically be used.
    (dateutils module required - otherwise it will be ignored).

    Max allowed value for daynumber is 16777215 (9th May 41222)
    (so daynumber can be any integer from 1 to 16777215 that you want to 'watermark' the hash with
    could be used as a session ID for a CGI for example).

    If a timestamp is passed in it should either be timestamp = True meanining use 'now'.
    Or it should be a tuple (HOUR, MINUTES).
    HOUR should be an integer 0-23
    MINUTES should be an integer 0-59

    The time and date stamp is *binary* interleaved, before encoding, into the data.

    If endleave is set to True then the timestamp is interleaved more securely. Shouldn't be necessary in practise
    because the stamp is so short and we subsequently encode using table_enc.
    If the string is long this will slow down the process - because we interleave twice.
    """
    if indict == None: indict = {}
    arglist = {'lower' : False, 'sha_hash' : False, 'daynumber' : None, 'timestamp' : None, 'endleave' : False}

    if not indict and keywargs:         # if keyword passed in instead of a dictionary - we use that
        indict = keywargs
    for keyword in arglist:             # any keywords not specified we use the default
        if not indict.has_key(keyword):
            indict[keyword] = arglist[keyword]
            
    if indict['lower']:     # keyword lower :-)
        instring = instring.lower()
    if indict['sha_hash']:
        instring = sha.new(instring).digest()

    if indict['daynumber'] == True:
        if not DATEIN:
            indict['daynumber'] = None
        else:
            a,b,c = returndate()
            indict['daynumber'] = daycount(a,b,c)       # set the daycount to today
    if indict['timestamp']== True:
        if not DATEIN:
            indict['timestamp'] = None
        else:
            indict['timestamp'] = return_now()          # set the time to now.

    datestamp = makestamp(indict['daynumber'], indict['timestamp'])
    if len(instring) == len(datestamp): instring = instring + '&mjf-end;'               # otherwise we can't tell which is which when we unleave them later :-)
    outdata = binleave(instring, datestamp, indict['endleave'])
    return table_enc(outdata)         # do the encoding of the actual string



def pass_dec(incode):
    """Given a string encoded by pass_enc - it returns it decoded.
    It also extracts the datestamp and returns that.
    The return is :
    (instring, daynumber, timestamp)
    """
    binary = table_dec(incode)
    out1, out2 = binunleave(binary)
    if len(out1) == 5:
        datestamp = out1
        if out2.endswith('&mjf-end;'):
            out2 = out2[:-9]
        instring = out2
    else:
        datestamp = out2
        if out1.endswith('&mjf-end;'):
            out1 = out1[:-9]
        instring = out1
    daynumber, timestamp = dec_datestamp(datestamp)
    return instring, daynumber, timestamp



def expired(daynumber, timestamp, validity):
    """Given the length of time a password is valid for, it checks if a daynumber/timestamp tuple is
    still valid.
    validity should be an integer tuple (DAYS, HOURS, MINUTES).
    Returns True for valid or False for invalid. 
    Needs the dateutils module to get the current daynumber.

>>> a, b, c = returndate()
>>> today = daycount(a, b, c)
>>> h, m = return_now()
>>> expired(today, (h, m-2), (0,0,1))
False
>>> expired(today, (h, m-2), (0,0,10))
True
>>> expired(today, (h-2, m-2), (0,1,10))
False
>>> expired(today-1, (h-2, m-2), (1,1,10))
False
>>> expired(today-1, (h-2, m-2), (2,1,10))
True
>>> 
    """
    if not DATEIN:
        raise ImportError("Need the dateutils module to use the 'expired' function.")

    h1, m1 = timestamp
# h1, m1 are the hours and minutes of the timestamp
    d2, h2, m2 = validity
# validity is how long the timestamp is valid for

    a, b, c = returndate()
    today = daycount(a, b, c)
# today is number representing the julian day number of today
    h, m = return_now()
# h, m are the hours and minutes of time now

    h1 = h1 + h2
    m1 = m1 + m2
    daynumber = daynumber + d2
# so we need to test if today, h, m are greater than daynumber, h1, m1
# But first we need to adjust because we might currently have hours above 23 and minutes above 59
    while m1 > 59:
        h1 += 1
        m1 -= 60
    while h1 > 23:
        daynumber += 1
        h1 -= 24
    daynumber += d2
    if today > daynumber:
        return False
    if today < daynumber:
        return True
    if h > h1:                  # same day
        return False
    if h < h1:
        return True
    if m > m1:                  # same hour
        return False
    else:
        return True

unexpired = expired             # Technically unexpired is a better name since this function returns True if the timestamp is unexpired.

def makestamp(daynumber, timestamp):
    """Receives a Julian daynumber (integer 1 to 16777215) and an (HOUR, MINUTES) tuple timestamp.
    Returns a 5 digit string of binary characters that represent that date/time.
    Can receive None for either or both of these arguments.

    The function 'daycount' in dateutils will turn a date into a daynumber.
    """
    if not daynumber:
        datestamp = chr(0)*3
    else:
        day1 = daynumber//65536
        daynumber = daynumber % 65536
        day2 = daynumber//256
        daynumber = daynumber%256
        datestamp = chr(day1) + chr(day2) + chr(daynumber)
    if not timestamp:
        datestamp = datestamp + chr(255)*2
    else:
        datestamp = datestamp + chr(timestamp[0]) + chr(timestamp[1])
    return datestamp


def dec_datestamp(datestamp):
    """Given a 5 character datestamp made by makestamp, it returns it as the tuple :
    (daynumber, timestamp).
    daynumber and timestamp can either be None *or*
    daynumber is an integer between 1 and 16777215
    timestamp is  (HOUR, MINUTES)

    The function 'counttodate' in dateutils will turn a daynumber back into a date."""
    daynumber = datestamp[:3]
    timechars = datestamp[3:]
    daynumber = ord(daynumber[0])*65536 + ord(daynumber[1])*256 + ord(daynumber[2])
    if daynumber == 0: daynumber = None
    if ord(timechars[0]) == 255:
        timestamp = None
    else:
        timestamp = (ord(timechars[0]), ord(timechars[1]))
    return daynumber, timestamp



def sixbit(invalue):
    """Given a value in it returns a list representing the base 64 version of that number.
    Each value in the list is an integer from 0-63...
    The first member of the list is the most significant figure... down to the remainder.
    Should only be used for positive values.
    """
    if invalue < 1:     # special case !
        return [0]
    power = -1
    outlist = []
    test = 0 
    while test <= invalue:
        power += 1
        test = pow(64,power)
        
    while power:
        power -= 1
        outlist.append(int(invalue//pow(64,power)))
        invalue = invalue % pow(64,power)
    return outlist

def sixtoeight(intuple):
    """Given four base 64 (6-bit) digits... it returns three 8 bit digits that represent
    the same value.
    If length of intuple != 4, or any digits are > 63, it returns None.

    **NOTE**
    Not quite the reverse of the sixbit function."""
    if len(intuple) != 4: return None
    for entry in intuple:
        if entry > 63:
            return None
    value = intuple[3] + intuple[2]*64 + intuple[1]*4096 + intuple[0]*262144
    val1 = value//65536
    value = value % 65536
    val2 = value//256
    value = value % 256
    return val1, val2, value


def table_enc(instring, table=None):
    """The actual function that performs TABLE encoding.
    It takes instring in three character chunks (three 8 bit values)
    and turns it into 4 6 bit characters.
    Each of these 6 bit characters maps to a character in TABLE.
    If the length of instring is not divisible by three it is padded with Null bytes.
    The number of Null bytes to remove is then encoded as a semi-random character at the start of the string.
    You can pass in an alternative 64 character string to do the encoding with if you want.
    """
    if table == None: table = TABLE
    out = []
    test = len(instring) % 3
    if test: instring = instring + chr(0)*(3-test)          # make sure the length of instring is divisible by 3
#    print test,'  ', len(instring) % 3
    while instring:
        chunk = instring[:3]
        instring = instring[3:]
        value = 65536 * ord(chunk[0]) + 256 * ord(chunk[1]) + ord(chunk[2])
        newdat = sixbit(value)
        while len(newdat) < 4:
            newdat.insert(0, 0)
        for char in newdat:
            out.append(table[char])
    if not test:
        out.insert(0, table[int(random()*21)])                # if we added 0 extra characters we add a character from 0 to 20
    elif test == 1:
        out.insert(0, table[int(random()*21)+21])                # if we added 1 extra characters we add a character from 21 to 41
    elif test == 2:
        out.insert(0, table[int(random()*22)+42])                 # if we added 1 extra characters we add a character from 42 to 63
    return ''.join(out)

def table_dec(instring, table=None):
    """The function that performs TABLE decoding.
    Given a TABLE encoded string it returns the original binary data - as a string.
    If the data it's given is invalid (not data encoded by table_enc) it returns None
    (definition of invalid : not consisting of characters in the TABLE or length not len(instring) % 4 = 1).
    You can pass in an alternative 64 character string to do the decoding with if you want.
    """
    if table == None: table = TABLE
    out = []
    rem_test = table.find(instring[0])      # remove the length data at the end
    if rem_test == -1: return None
    instring = instring[1:]
    if len(instring)%4 != 0: return None        # check the length is now divisible by 4
    while instring:
        chunk = instring[:4]
        instring = instring[4:]
        newchunk = []
        for char in chunk:
            test = table.find(char)
            if test == -1: return None
            newchunk.append(test)
        newchars = sixtoeight(newchunk)
        if not newchars: return None
        for char in newchars:
            out.append(chr(char))
    if rem_test > 41:
        out = out[:-1]
    elif rem_test > 20:
        out = out[:-2]
    return ''.join(out)

def return_now():
    """Returns the time now.
    As (HOUR, MINUTES)."""
    return int(strftime('%I')), int(strftime('%M'))

def check_pass(inhash, pswdhash, EXPIRE):
    """Given the hash (possibly from a webpage) it checks that it is still valid and matches the password it is supposed
    to have.
    If so it returns the new hash.
    If expired it returns -1.
    If the pass is invalid it returns False."""
    try:
        instring, daynumber, timestamp = pass_dec(inhash)         # of course a fake or mangled password will cause an exception here
        if not table_dec(pswdhash) == instring:
            return False
        if not unexpired(daynumber, timestamp, EXPIRE):            # this tests if the hash is still valid and is the password hash the same as the password hash encoded in the page ?
            return -1
        else:
            return pass_enc(instring, daynumber = True, timestamp = True)       # generate a new hash, with the current time
    except:
        return False

def binleave(data1, data2, endleave = False):
    """Given two strings of binary data it interleaves data1 into data2 on a bitwise basis
    and returns a single string combining both. (bits interleaved not just the bytes).
    The returned string will be 4 bytes or so longer than the two strings passed in.
    Use bin_unleave to return the two strings again.
    Even if both strings passed in are ascii - the result will contain non-ascii characters.
    To keep ascii-safe you must subsequently encode with table_enc.

    Max length for the smallest data string (one string can be of unlimited size) is about 16meg
    (increasing this would be easy if anyone needed it - but would be very slow anyway).

    If either string is empty (or the smallest string greater than 16meg) - we return None.
    The first 4 characters of the string returned 'define' the interleave. (actually the size of the watermark)
    For added safety you could remove this and send seperately.

    Version 1.0.0 used a bf (bitfield) object from the python cookbook. Version 1.1.0 uses the binary and & and or |
    operations and is about 2.5 times faster. On my AMD 3000, leaving and unleaving two 20k files took 1.8 seconds.
    (instead of 4.5 previously - with Psyco enabled this improved to 0.4 seconds.....)

    Interleaving a file with a watermark of pretty much any size makes it unreadable - this is because *every* byte is changed.
    (Except perhaps a few at the end - see the endleave keyword). However it shouldn't be relied on if you need
    a really secure method of encryption. For many purposes it will be sufficient however.
    
    In practise any file not an exact multiple of the size of the watermark will have a chunk at the end that is untouched.
    To get round this you can set endleave = True.. which then releaves the end data back into itself.
    (and therefore takes twice as long - it shouldn't be necessary where you have a short watermark.)

    data2 ought to be the smaller string - or they will be swapped round internally.
    This could cause you to get them back in an unexpected order from binunleave.    
    """
    header, out, data1 = internalfunc(data1,data2)
#    print len(data1), len(out), len(header)
    header = chr(int(random()*128)) + header            # making it a 4 byte header
    if endleave and data1 and len(data1) < 65536:
        header, out, data1 = internalfunc(header + out, data1)
        header = chr(int(random()*128)+ 128) + header
    return header + out + data1

def binunleave(data):
    """Given a chunk of data woven by binleave - it returns the two seperate pieces of data."""
    header = data[0]
    data = data[1:]
    data1, data2 = internalfunc2(data)
    if ord(header) > 127:
#        print len(data1)
        data = data2 + data1
        data = data[1:]
        data1, data2 = internalfunc2(data)
    return data1, data2

######################

# binleave and binunleave used to make extensive use of a python objectcalled bf() (bitfield)
# There are still many places this could be useful, but I now use binary operations inline.
# Included for reference is the bf object and the binary operations as functions.

class bf(object):
    """the bf(object) from activestate python cookbook - by Sebastien Keim - Many Thanks
    http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/113799"""
    def __init__(self,value=0):
        self._d = value

    def __getitem__(self, index):
        return (self._d >> index) & 1 

    def __setitem__(self,index,value):
        value    = (value&1L)<<index
        mask     = (1L)<<index
        self._d  = (self._d & ~mask) | value

    def __getslice__(self, start, end):
        mask = 2L**(end - start) -1
        return (self._d >> start) & mask

    def __setslice__(self, start, end, value):
        mask = 2L**(end - start) -1
        value = (value & mask) << start
        mask = mask << start
        self._d = (self._d & ~mask) | value
        return (self._d >> start) & mask

    def __int__(self):
        return self._d


def bittest(value, bitindex):
    """This function returns the setting of any bit from a value.
    bitindex starts at 0.
    """
    return (value&(1<<bitindex))>>bitindex

def bitset(value, bitindex, bit):
    """Sets a bit, specified by bitindex, in 'value' to 'bit'.
    bit should be 1 or 0
    bitindex starts at 0.
    """
    bit    = (bit&1L)<<bitindex
    mask     = (1L)<<bitindex
    return (value & ~mask) | bit                # set that bit of value to 0 with an & operation and then or it with the 'bit'

    

##################################################################

# private functions used by the above public functions

def internalfunc(data1, data2):
    """Used by binleave.
    This function interleaves data2 into data1 a little chunk at a time."""
    if len(data2) > len(data1):         # make sure that data1 always has the longer string, making data2 the watermark
        dummy = data1
        data1 = data2
        data2 = dummy
    if not data2 or not data1: return None      # check for empty data
    length = len(data2)
    if length >= pow(2,24): return None         # if the strings are oversized
    multiple = len(data1)//length       # this is how often we should interleave bits
    if multiple > 65535: multiple = 65535   # in practise we'll set to max 65535
    header1 = length//65536
    header3 = length % 65536
    header2 = header3//256
    header3 = header3 % 256
    header = chr(header1) + chr(header2) + chr(header3)      # these are the 3 bytes we will put at the start of the string
# so - to encode one byte of data2 (the watermark) we need multiple bytes of data1
    data1 = [ord(char) for char in list(data1)]
    startpos = 0
    data2 = [ord(char) for char in list(data2)]
    BINLIST=[1,2,4,8,16,32,64,128]
    out = []
    bitlen = multiple*8 + 8     # the total number of bits we'll have
#    print bitlen, multiple
    while data2:
        chunklist = data1[startpos:startpos + multiple]
        startpos = startpos + multiple
        heapobj = 0
        mainobj = data2.pop(0)
        charobj = chunklist.pop(0)
        bitindex = 0
        mainindex = 0
        heapindex = 0
        charindex = 0
        while mainindex < bitlen:
    #        print  mainindex, heapindex, charindex, bitindex
            if heapindex == 8:       #    if we've got all 8 bit's
                out.append(chr(heapobj))
                heapobj = 0
                heapindex = 0
            if not mainindex%(multiple+1):            # we've got to a point where we should nick another bit from the byte
                if mainobj&BINLIST[bitindex]:       # if the bit at binindex is set
                    heapobj = heapobj|BINLIST[heapindex] # set the bit at heapindex
                heapindex += 1
                bitindex += 1
                mainindex += 1
                continue
            if charindex == 7 and chunklist:                   # we've used up the current character from the chunk
                if charobj&BINLIST[charindex]:
                    heapobj = heapobj|BINLIST[heapindex]
                charobj = chunklist.pop(0)
                charindex = 0
                heapindex += 1
                mainindex += 1 
                continue
            if charobj&BINLIST[charindex]:
                heapobj = heapobj|BINLIST[heapindex]
            heapindex += 1
            charindex += 1
            mainindex += 1
            
        if heapindex == 8:       #    if we've got all 8 bit's.. but the loop has ended...
            out.append(chr(heapobj))

    return header, ''.join(out), ''.join([chr(char) for char in data1[startpos:]])

def internalfunc2(data):
    """Used by binunleave.
    This function extracts data that has been interleaved using binleave."""
    lenstr = data[:3]           # extract the length of the watermark
    data = list(data[3:])
    length2 = ord(lenstr[0])*65536 + ord(lenstr[1])*256 + ord(lenstr[2])       # length of watermark
    length1 = len(data) - length2                                               # overall length
    multiple = length1//length2 + 1
    if multiple > 65536: multiple = 65536   # in practise we'll set to max 65535 + 1
    bitlen = multiple*8
#    print len(data), length1, length2, multiple
    out1 = []
    out = []
    index = 0
    BINLIST=[1,2,4,8,16,32,64,128]
#    print len(chunk)    
    while index < length2:
        index += 1
        chunk = data[:multiple]
        data = data[multiple:]
        chunklist = [ord(char) for char in chunk]       # turn chunk into a list of it's values
        heapobj = 0
        outbyte = 0
        charobj = chunklist.pop(0)
        bitindex = 0
        mainindex = 0
        heapindex = 0
        charindex = 0
        while mainindex < bitlen:
    #        print  mainindex, heapindex, charindex, bitindex
            if heapindex == 8:       #    if we've got all 8 bit's
                out.append(chr(heapobj))
                heapobj = 0
                heapindex = 0    
            if not mainindex%multiple:            # we've got to a point where we should add another bit to the byte
                if charobj&BINLIST[charindex]:
                    outbyte = outbyte|BINLIST[bitindex]
                if not charindex == 7:
                    charindex += 1
                else:
                    charobj = chunklist.pop(0)
                    charindex = 0                
                bitindex += 1
                mainindex += 1
                continue
            if charindex == 7 and chunklist:                   # we've used up the current character from the chunk
                if charobj&BINLIST[charindex]:
                    heapobj = heapobj|BINLIST[heapindex]
                charobj = chunklist.pop(0)
                charindex = 0
                heapindex += 1
                mainindex += 1 
                continue
            if charobj&BINLIST[charindex]:
                heapobj = heapobj|BINLIST[heapindex]
            heapindex += 1
            charindex += 1
            mainindex += 1
        if heapindex == 8:       #    if we've got all 8 bit's.. but the loop has ended...
            out.append(chr(heapobj))
        out1.append(chr(outbyte))

    return ''.join(out1), ''.join(out+data)

def test():                     # the test suite
    from time import clock
    from os.path import exists
    print 'Printing the TABLE : '
    index = 0
    while index < len(TABLE):
        print TABLE[index], TABLE.find(TABLE[index])
        index +=1

    print '\nEnter test password to encode using table_enc :\n(Hit enter to continue past this)\n'
    while True:
        dummy = raw_input('>>...')
        if not dummy: break
        test =  table_enc(dummy)
        test2 = table_dec(test)
        print test
        print 'length  : ', len(test), '    modulo 4 of length - 1  : ', (len(test)-1) % 4
        print 'Decoded : ', test2
        print 'Length dec : ', len(test2)

    print '\nEnter password - to timestamp and then encode :\n(Hit enter to continue past this)\n'
    while True:
        instring = raw_input('>>...')
        if not instring:
            break
        code = pass_enc(instring, sha_hash=False, daynumber=True, timestamp=True)
        print code
        print pass_dec(code)


    print '\n\nTesting interleaving a 1000 byte random string with a 1500 byte random string :'
    print
    print 'Overall length of combined string : ',
    a=0
    b=''
    c = ''
    while a < 1000:
        a += 1
        b = b + chr(int(random()*256))
        c = c + chr(int(random()*256))
    while a < 1500:
        a += 1
        c = c + chr(int(random()*256))
    d = clock()
    test = binleave(c, b, True)
    print  len(test)
    a1, a2 = binunleave(test)
    print 'Time taken (including print statements ;-) ', str(clock()-d)[:6], ' seconds'
    print 'Test for equality of extracted data against original :'
    print a1 == b
    print a2 == c


# If you give it two test files 'test1.zip' and 'test2.zip' it will interleave the two files,
# unleave them again and write out the first file as 'test4.zip'
# It prints how long it takes and you can verify that the returned file is undamaged.
    
    if exists('test1.zip') and exists('test2.zip'):
        print
        print "Reading 'test1.zip' and 'test2.zip'"
        print "Interleaving them together and writing the combined file out as 'test3.zip'"
        print "Then unleaving them and writing 'test1.zip' back out as 'test4.zip'",
        print " to confirm it is unchanged by the process"
        a = file('test1.zip','rb')
        b = a.read()
        a.close()
        a = file('test2.zip','rb')
        c  = a.read()
        a.close()
        d = clock()
        test = binleave(c,b, True)

        print len(test)
        a = file('test3.zip','wb')
        a.write(test)
        a.close()
        a1, a2 = binunleave(test)
        print str(clock()-d)[:6]
        a = file('test4.zip','wb')
        a.write(a1)
        a.close()
    else:
        print
        print 'Unable to perform final test.'
        print "We need two files to use for the test : 'test1.zip' and 'test2.zip'"
        print "We then interleave them together, and write the combined file out as 'test3.zip'"
        print "Then we unleave them again, and write 'test1.zip' back out as 'test4.zip'",
        print "(So we can confirm that it's unchanged by the process.)"

    



if __name__ == '__main__':

# the start of making dataenc an application - but I don't think it will be used :-)
# just runs the test suite instead

### this is executed if dataenc is run from the commandline
##
### first we get the arguments we were called with using optparse
##
### minimum arguments :
### input file
### output file
##
### default :
### if three file arguments are given the two are interelaved and saved as the third file
### so long as the third file doesn't already exist.
##
### if two filenames are given it reads the first file and datestamps it
### saves as the second file (assuming it doesn't exist)
##
### if one filename is given it assumes it is a n interleaved file to extract
##
### options :
### overwrite output file             - default OFF
### encode or decode                  - default is encode (specifying three files forces encode)
### table_enc on or off               - default is OFF
### specify a TABLE file              - default is to use inbuilt
### datestamp/interleave on or off    - default is ON (datestamping)
### endleaving on or off              - default is OFF
### header file                       - default is to use the header in the file when decoding, and to leave it in the file when encoding
### (If a header file is specified the 3 byte header from binary interleaving will be saved seperately).
##
### **special**
### config file - *all* values are read from the config file
##
##    from optparse import OptionParser
##
##    parser = OptionParser()
##    parser.add_option("-q", "--quiet",
##                      action="store_false", dest="quiet", default = False,
##                      help="Set a verbosity level of 0, print no messages.")
##    
##    parser.add_option("--test",
##                      action="store_true", dest="test", default=False,
##                      help="Run the tests, all other options ignored. Verbosity of tests is 9.")
##    
##    parser.add_option("-v", "--verbose", type = 'int', dest="verbose", default=9,
##                      help="Set the verbosity level. Should be an integer from 0 to 9,"+\
##                      "9 means the most verbose and 0 means don't ouput any messages. Default is 9.")
##    
##    parser.add_option("-d", "--decode",
##                      action="store_true", dest="decode", default = False,
##                      help="Set to decode rather than encode. Default is encode.")
##    
##    parser.add_option("-t", "--table",
##                      action="store_true", dest="table", default = False,
##                      help="Encode or decode files using the TABLE. (ASCII  to binary or binary to ASCII).")
##    
##    parser.add_option("-T","--TABLE", dest="table_file", 
##                  help="Specify a 64 character file to use as the TABLE for encoding/decoding.")
##    
##    parser.add_option("-o", "--off",
##                      action="store_false", dest="datestamp", default = True,
##                      help="Switches datestamping OFF. Default is ON.")
##
##    parser.add_option("-e", "--end",
##                      action="store_false", dest="end", default = True,
##                      help="Switches endleaving ON. default is OFF.")
##    
##    parser.add_option("-H","--header", dest="header_file", default = False,
##                  help="Specify a separate file to use as the header file when binary encoding/decoding.")
##
##    parser.add_option("-c","--config", dest="config_file", default = False,
##                  help="Specify a config file to read *all* the other options from.")
##
##
##
##    options, args = parser.parse_args()
###    print args
##
##
### next import StandOut which allows us to set variable levels of verbosity
##    try:
##        from standout import StandOut
##        stout = StandOut()
##    except:
##        print 'dataenc uses the standout module to handle varying levels of verbosity'
##        print 'Without it, all messages will be printed.'
##        class dummy:                # a dummy object that we can twiddle if StandOut isn't available
##            def __init__(self):
##                self.priority = 0
##                self.verbosity = 0
##            def close(self):
##                pass
##        stout = dummy()
##    
##    defaults = { 'header_file' : False, 'datestamp' : True
##    if options.config_file:          # a configfile, the settings here override all the others
##        try:
##            from configobj import ConfigObj
##        except ImportError:
##            print "Without the ConfigObj module I can't import a config file."
##            print 'See http://www.voidspace.org.uk/atlantibots/pythonutils.html'
##            raise
##        config = ConfigObj(options.config_file, fileerror=True)
##        
##
##    if options.verbose:
##        stout.verbosity = 10 - options.verbose          # a higher verbosity level here, actually means quiter
##    else:
##        stout.verbosity = 0                             # except for 0, which means silent
##    if options.quiet:                                   # if the quiet option is explicitly set
##        stout.verbosity = 0
##    stout.priority = 2
##    print 'Welcome to dataenc - the data encoding and interleaving program by Fuzzyman'
##    print 'See http://www.voidspace.org.uk/atlantibots/pythonutils.html'
##    print 'Written in Python.'
##    stout.priority = 3
##    if not psycoin:
##        print 'Having the Psyco module installed (Python Specialising compiler) would vastly speed up dataenc.'
##    if not DATEIN:
##        print 'Some of the datestamping features are only available when the dateutils module is available.'
##    stout.priority = 5
##
##        
##    if options.test:
    test()


    



"""

BUGS
No more known bugs... yet.
I'm sure they'll surface.

ISSUES
binleave and bin_unleave are still quite slow.
For stamping small password hashes with a date stamp it's fast enough - for weaving larger files together it's *too slow*.
Also for weaving similar sized files together we may be better with a pattern of 2 bits of water mark per 3 bits of string.
(or a 3 to 4 or 5 to 7 etc..)
Currrently it will only work with 1 bit of watermark per 1 or 2 or 3 or 4 etc bits of main string. (Exact multiples)
Again, for small watermarks this works fine - and as that is all I'm using it for I'm not inclined to change it.
The logic would be simple - just fiddly.


TODO :
Might make it a simple application - so it can be used from the command line for encoding, decoding
timestamping and combining files.....
Could replace use of the BINLIST and the if test with a single inline statement with more << >> in binleave and binunleave
Could move the binleave and binuleave into C


CHANGELOG
13-09-04        Version 1.1.5
Increased speed in table_enc and table_dec.

30-08-04        Version 1.1.4
Slight docs improvement.
Slight speed improvement in binleave and binunleave.

22-08-04        Version 1.1.3
Added the unexpired alias and the check_pass function.
Changed license text.
Minor preemptive bugfix in some default values.

11-04-04        Version 1.1.2
Added the expired function for testing validity of timestamps.
Changed the TABLE to be URL safe for passing in forms using the 'GET' method.
Added OLD_TABLE with the old encoding, and gave table_dec and table_enc the ability to receive an explicit TABLE.

07-04-04        Version 1.1.1
Improved the tests a bit.
Corrected a bug that affected large files or large files with small watermarks.

05-04-04        Version 1.1.0
Replaced the bf object with much faster bitwise logical operations. It is now about 2.5 times faster.
With Psyco enabled it becomes 11 times faster than the first version....
Added the bit setting and testing operations as functions.

03-04-04        Version 1.0.0
Initial testing is a success.


"""

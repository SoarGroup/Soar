# 2005/01/06
# v2.1.0

# A flexible object to redirect standard output and standard error
# Allows logging to a file and to set a level of verbosity

# Copyright Michael Foord, 2004.
# Released subject to the BSD License
# Please see http://www.voidspace.org.uk/documents/BSD-LICENSE.txt

# For information about bugfixes, updates and support, please join the Pythonutils mailing list.
# http://voidspace.org.uk/mailman/listinfo/pythonutils_voidspace.org.uk
# Comments, suggestions and bug reports welcome.
# Scripts maintained at http://www.voidspace.org.uk/python/index.shtml
# E-mail fuzzyman@voidspace.org.uk

"""
StandOut - the Flexible Output Object (FOO !)
Adds optional logging to a file and setting of verbosity levels to the stdout stream
This means that, for the most part, standard print statments can be used throughout
your program and StandOut handles the rest.

Your user can choose a 'verbosity' level (how much information they want to receive), you give your messages
a priority level, and only messages with a high enough priority are actually displayed.

A simple way of implementing varying degrees of verbosity.
Additionally the output can be captured to a log file with no extra work.
(simply pass in a filename when you craete the object and anything printed is sent to the file as well)

StandOut can now be used with sys.stderr as well as sys.stdout.
This includes logging both sys.stdout *and* sys.stderr to the same file.
See the sys.stderr section at the bottom of this.

SIMPLE USAGE
(also see the tests which illustrate usage).

stout = StandOut(verbosity=verbositylevel)

or to log to a file :
stout = StandOut(filename='log.txt')

The verbosity level can be changed at any time by setting stout.verbosity :
stout.verbosity = 6

The priority of messages defaults to 5. This can be changed by setting
stout.priority = 6 *or*
print '&priority-6;'

The priority of an individual line can be set by *starting* the line with a priority marker :
print '&priority-6;This text has a priority 6.'
*or* by using the stout.write() method with a priority value:
stout.write('This text has a priority 6.\n', 6)
(notice you must add the '\n' when using the stout.write method.)

Only messages with a priority equal to or greater than the current verbosity level will be printed.
e.g. if  stout.verbosity = 6
(or the stout object was created using stout=StandOut(verbosity=6) )
Only messages with a priority of 6 or above will be printed.

stout.write('This won't get printed\n, 5)
print '&priority-4;Nor will this'
stout.write('But this will\n', 6)
print '&priority-7;And so will this'

If for *any* reason you want to *actually* print a '&priority-n' marker at the start of a line
then you can escape it with a '&priority-e;' :
print '&priority-e;&priority-1;'
will actually print :
&priority-1;

StandOut will log to a file as well.
Set this by passing in a filename=filename keyword when you create the object *or* by setting stout.filename at any time.
The file has it's own priority, stout.file_verbosity.
Again this can be set when the object is created and/or changed at any time. See the full docs below.

This means your user can set a verbosity level (at the command line probably), you give each message a priority
setting and just use normal print statements in your program.
Only messages above your user's setting are actually displayed.
You can also set the log file to have a different priority threshhold to what is printed to the screen.
(So either less or more is logged to the file than is displayed at runtime.)

You can also pass in another function which can be used to display messages with (e.g. to a GUI window or whatever).
It also has it's own priority setting.

Any output method can be silenced by setting it to 0
All output can be silenced by setting the priority to 0

The stdout stream can be restored and any log file closed by calling stout.close()

verbosity = 1 is the highest
verbosity = 9 is the lowest     (only messages of priority 9 are printed)
verbosity = 0 is special - it switches off printing altogether

LIST OF KEYWORDS AND METHODS

StandOut Possible keyword arguments (with defaults shown) are :
(The following keywords also map to attributes of the StandOut object which can be read or set)
priority = 5
verbosity = 5
filename = None
file_verbosity = 5
file_mode = 'w'
print_fun = None
printfun_verbosity = 5

Keyword arguments should either be passed in as a dictionary *or* as keywords when the object is created.
If a dictionary is passed in, any other keywords will be ignored.
Any missing keywords will use the defaults.

Methods ( stout = StandOut() ):
stout.close()
stout.write(line, priority)
stout.set_print(function)
stout.setall(verbosity)

the original stdout can be reached using :
stout.output.write()

**NOTE** normal print statements make two calls to stdout.write(). Once for the text you are printing and another for the
trailing '\n' or ' '. StandOut captures this to make sure the trailing '\n' or ' ' is printed at the same priority
as the original line. This means you shouldn't use stout.write(line) where line uses the '&priority-n;' markers.
(Because stout.write(line) only makes one call, not two).
Either call stout.write(line, priority) to set a priority for that line.
*or* set stout.priority directly.



EXPLANATION OF KEYWORDS AND METHODS

priority = 5
This sets the priority for messages.
If priority is 5 - then only output methods with a 'verbosity' of 5 or lower will display them.
This value can later be set by adjusting the stout.priority attribute or using the priority markers.

verbosity = 5
This is the verbosity level for messages to be printed to the screen.
If the verbosity is 5 then only messages with a priority of 5 or higher will be sent to the screen.
(Like a normal print statement).
You can nadjust this at stout.verbosity

filename = None
If you pass in a filename when you create the object it will be used as a logfile.
It has it's own 'verbosity' level called 'file_verbosity'.
If you don't pass in a filename, you can later add one by setting stout.filename
Changing stout.filename after you have already set one is a bad thing to do :-)

file_verbosity = 5
This is the verbosity level of the log file.
Only messages with a priority higher than this will be sent to the logfile.

print_fun = None
If you pass in a function (that takes one parameter - the line to be printed) this will be used to print as well.
The function *isn't* stored at stout.print_fun - this value is just set to True to say we have a function.
This could be used for displaying to the output window of a GUI, for example.
If you want to pass in a function after obect creation then use the stout.set_print(function) method.
You musn't have print statements in your function or you will get stuck in a loop (call stout.output.write(line) instead)

printfun_verbosity = 5
Any function you pass in also has it's own verbosity setting - printfun_verbosity.

stream = 'output'
By default StandOut will divert the sys.stdout stream. Set to 'error' to divert the sys.stderr

share = False
You can divert both sys.stdout and sys.stderr. You can log both to the same file.
Set a filename for your sys.stdout object and set share = True for your sys.stderr object.
Any lines sent to sys.stderr will have a prefix attached to them. See 'error_marker'

error_marker = '[err] '
This is the marker put before every line logged from sys.stderr.
It only applies if share is on - this means both streams are logged to the same file.

stout.close()
When your program has finished with the obejct it should call stout.close() which restores sy.stdout and
closes any logfile we have been using.

stout.write(line, priority)
This can be used as an alternative way of specifying a priority for an individual line.
It leaves stout.priority unaffected.
Any calls to stout.write must have '\n' at the end if you want it to end with a newline.
If you don't specify a priority then it behaves like sys.stdout.write would.
Don't use priority markers with this and method and you can't use  priority = 0 (the priority setting will be ignored)

stout.set_print(function)
This is used to pass in an additional printing function after the object has been created.

stout.setall(verbosity)
Thisis a quick way of changing the verbosity for all three output methods.

Setting verbosity, file_verbosity or printfun_verbosity to 0 disables that ouput method.
Setting priority to 0 switches off all output.

If you want to print to stdout directly and bypass the stout object for any reason - it is saved at stout.output
Calls to stout.output.write(line) have the same effect that sys.stdout.write(line) would have had.

PRIORITY MARKERS

As well as directly setting stout.priority and using stout.write(line, priority)
You can set the priority of a individual  line *or* change the general priority setting just using print statements.
This is using 'priority markers'.

print '&priority-n;'  # sets the priority to n, where n is 0-9
print '&priority-n;The stuff to print'      # sets the priority of just that line to n

If you actually want to print '&priority-n;' at the start of a line then you should escape it by putting '&priority-e;'
in front of it. '&priority-e;' can also be escaped in the same way !

Don't use priority markers if you are making direct calls to stout.write()
use stout.write(line, priority) to set the priority of an individual line
or alter stout.priority to adjust the general priority.


sys.stderr

StandOut can now be used to divert sys.stderr as well as sys.stdout.
To create an output object that does for sys.stderr *exactly* the same as we would do for sys.stdout use :
stout2 = StandOut(stream='error')

It can log to a file and has all the properties that we had for sys.stdout.
If you wanted to log to the *same* file as you are using for sys.stdout you *can't* just pass it the same filename.
The two objects would both try to have a write lock on the same file.

What you do is pass the 'share' keyword to the error object when you create it :

stout = StandOut(filename='log.txt')
stout2 = StandOut(stream='error', share=True)

Anything sent to sys.stdout *or* sys.stderr will now be logged in the 'log.txt' file.
Every line sent to sys.stderr will be prefixed with '[err] ' which is the default error marker.
You can adjust this with the 'error_marker' keyword.

stout2 = StandOut(stream='error', share=True, error_marker='**ERROR** ')

"""

__all__ = ['StandOut']

import sys

class StandOut:
    
    stdout = None
    stderr = None
    
    def __init__(self, indict=None, **keywargs):
        """StandOut - the Flexible Output Object (FOO !)"""
        if indict is None:
            indict = {}
        #
        defaults = {
            'priority': 5,
            'verbosity': 5,
            'filename': None,
            'file_verbosity': 5,
            'file_mode': 'w',
            'print_fun': None,
            'printfun_verbosity': 5 ,
            'stream': 'output',
            'share': False,
            'error_marker': '[err] '
            }
        #
        if not indict:
            indict = keywargs
        for value in defaults:
            if not indict.has_key(value):
                indict[value] = defaults[value]
        #
        if indict['stream'].lower() == 'error':
            self.output = sys.stderr
            sys.stderr = StandOut.stderr = self
            self.stream = indict['stream'].lower()
        else:       
            self.output = sys.stdout
            sys.stdout = StandOut.stdout = self
            self.stream = indict['stream'].lower()

        self.filename = indict['filename']
        if self.filename:
            self.filehandle = file(self.filename, indict['file_mode'])
        else:
            self.filehandle = None

        self.file_mode = indict['file_mode']
        self.share = indict['share']
        self.err_marker = indict['error_marker']
        self.done_linefeed = True

        self.priority = indict['priority']              # current message priority
        self.file_verbosity = indict['file_verbosity']            # file output threshold
        self.verbosity = indict['verbosity']                    # stdout threshhold
        self.printfun_verbosity = indict['printfun_verbosity']    # print_fun threshold

        if indict['print_fun']:     # set up the print_fun if we have been given one
            self.print_fun = True
            self.thefun = [indict['print_fun']]
        else:
            self.print_fun = False

        self.markers = {}
        for num in range(10):                               # define the markers
            thismarker = '&priority-' + str(num) + ';'
            self.markers[thismarker] = num
        self.escapemarker = '&priority-e;'

        self.skip = 0
        self._lastpriority = 0
            
#########################################################################
        # public methods - available as methods of any instance of StandOut you create
        
    def write(self, line, priority = 0):
        """Print to any of the output methods we are using.
        Capture lines which set priority."""

        if self.skip:           # if the last line was a priority marker then self.skip is set and we should miss the '\n' or ' ' that is sent next
            self.skip = 0
            return
        
        if not priority:                
            if self._lastpriority:          # if the last line had a priority marker at the start of it, then the '\n' or ' ' that is sent next should have the same priority
                priority = self._lastpriority
                self._lastpriority = 0
            else:
                priority = self.priority

        if line in self.markers:                # if the line is a priority marker
            self.skip = 1                       # either a '\n' or a ' ' will now be sent  to sys.stdout.write() by print
            self.priority = self.markers[line]
            return
        
        if line[:12] in self.markers:           # if the line starts with a priority marker
            priority = int(line[10])            # the priority of this line is at position 10
            self._lastpriority = priority       # set this value so that the '\n' or ' ' that follows also has the same priority
            line = line[12:]                    # chop off the marker
        elif line[:12] == self.escapemarker:       
            line = line[12:]                    # this just removes our 'escape marker'
            
        if not priority:                    # if priority is set to 0 then we mute all output
            return

        if self.filename and not self.filehandle:                           # if a filename has been added since we opened
            self.filehandle = file(self.filename, self.file_mode)
        if self.filehandle and self.file_verbosity and priority >= self.file_verbosity:      # if we have a file and file_verbosity is high enough to output
            self.filehandle.write(line)
            
        if self.verbosity and priority >= self.verbosity:              # if verbosity is set high enough we print
            if self.share and self.stream == 'error' and hasattr(StandOut.stdout, 'filename'):      # if we are the error stream *and* share is on *and* stdout has a filename attribute..
                if self.done_linefeed:
                    StandOut.stdout.filehandle.write(self.err_marker)
                    self.done_linefeed = False
                if line.endswith('\n'):
                    self.done_linefeed = True
                    line = line[:-1]
                line = line.replace('\n', '\n' + self.err_marker)
                if self.done_linefeed:
                    line = line + '\n'
                StandOut.stdout.filehandle.write(line)                  # if 'share' is on we log to stdout file as well as print
#            StandOut.stdout.output.write('hello')
            self.output.write(line)
        # if we have a print function set and it's priority is high enough
        if self.print_fun and self.printfun_verbosity and priority >= self.printfun_verbosity:
            self.use_print(line)

    def close(self):
        """Restore the stdout stream and close the logging file if it's open."""
        if self.stream == 'error':
            sys.stderr = self.output
        else:    
            sys.stdout = self.output
        if self.filename and self.filehandle:
            self.filehandle.close()
            del self.filehandle
        
    def set_print(self, print_fun):
        """Set a new print_fun."""
        self.print_fun = True
        self.thefun = [print_fun]

    def setall(self, verbosity):
        """Sets the verbosity level for *all* the output methods."""
        self.verbosity = self.file_verbosity = self.printfun_verbosity = verbosity

    def flush(self):
        return self.output.flush()
    
    def writelines(self, inline):
        for line in inlines:
            self.write(line)
   
    def __getattr__(self, attribute):
        if not self.__dict__.has_key(attribute) or attribute == '__doc__':
            return getattr(self.output, attribute)
        return self.__dict__[attribute]

##########################################################
    # private methods, you shouldn't need to call these directly
    def use_print(self, line):
        """A wrapper function for the function passed in as 'print_fun'."""
        self.thefun[0](line)
        

if __name__ == '__main__':

    test = StandOut()
    print 'hello'
    test.priority = 4
    print "You shouldn't see this"
    test.verbosity = 4
    print 'You should see this'
    test.priority = 0
    print 'but not this'
    test.write('And you should see this\n', 5)
    print 'but not this'
    test.filename = 'test.txt'
    test.priority = 5
    test.setall(5)
    print 'This should go to the file test.txt as well as the screen.'
    test.file_verbosity = 7
    print '&priority-8;'
    print 'And this should be printed to both'
    print '&priority-6;But this should only go to the screen.'
    print 'And this should be printed to both, again.'

    def afunction(line):
        test.output.write('\nHello\n')

    test.set_print(afunction)
    print "We're now using another print function - which should mirror 'hello' to the screen."
    print "In practise you could use it to send output to a GUI window."
    print "Or perhaps format output."

    test2 = StandOut(stream='error', share=True)        # anything printed to sys.stderr, should now be logged to the stdout file as well 
    sys.stderr.write('Big Mistake')
    sys.stderr.write('\n')
    sys.stderr.write('Another Error')
    sys.stderr.write('\n')    
    
    test.close()
    test2.close()
    print 'Normality is now restored'
    print 'Any further problems, are entirely your own.'

"""
ISSUES/TODO
===========

Doctests

Could trap when stout.write(line) is used with a priority marker. (By checking
for the default value of priority).

Could add a ``both`` keyword argument to avoid having to use the `share``
keyword argument. It can just instantiate another StandOut itself.

CHANGELOG
=========

2005/01/06      Version 2.1.0
Added flush and writelines method.
Added the 'stream' keyword for diverting the sys.stderr stream as well.
Added __getattr__ for any undefined methods.
Added the 'share' and 'error_marker' keywords for logging sys.stderr to the same file as sys.stdout.

07-04-04        Version 2.0.0
A complete rewrite. It now redirects the stdout stream so that normal print statements can be used.
Much better.

06-04-04        Version 1.1.0
Fixed a bug in passing in newfunc. Previously it only worked if you had a dummy variable for self.
"""

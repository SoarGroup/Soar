# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - LogFile package

    This module supports buffered log reads, iterating forward and backward line-by-line, etc.

    @copyright: 2005-2007 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import log
logging = log.getLogger(__name__)

import os, codecs, errno
from MoinMoin import config, wikiutil

class LogError(Exception):
    """ Base class for log errors """

class LogMissing(LogError):
    """ Raised when the log is missing """


class LineBuffer:
    """
    Reads lines from a file
        self.len      number of lines in self.lines
        self.lines    list of lines (unicode)
        self.offsets  list of file offsets for each line. additionally the position
                      after the last read line is stored into self.offsets[-1]
    """
    def __init__(self, file, offset, size, forward=True):
        """

        TODO: when this gets refactored, don't use "file" (is a builtin)

        @param file: open file object
        @param offset: position in file to start from
        @param size: aproximate number of bytes to read
        @param forward : read from offset on or from offset-size to offset
        @type forward: boolean
        """
        self.loglevel = logging.NOTSET
        if forward:
            begin = offset
            logging.log(self.loglevel, "LineBuffer.init: forward seek %d read %d" % (begin, size))
            file.seek(begin)
            lines = file.readlines(size)
        else:
            if offset < 2 * size:
                begin = 0
                size = offset
            else:
                begin = offset - size
            logging.log(self.loglevel, "LineBuffer.init: backward seek %d read %d" % (begin, size))
            file.seek(begin)
            lines = file.read(size).splitlines(True)
            if begin != 0:
                # remove potentially incomplete first line
                begin += len(lines[0])
                lines = lines[1:]
                # XXX check for min one line read

        linecount = len(lines)

        # now calculate the file offsets of all read lines
        offsets = [len(line) for line in lines]
        offsets.append(0) # later this element will have the file offset after the last read line

        lengthpreviousline = 0
        offset = begin
        for i in xrange(linecount+1):
            offset += lengthpreviousline
            lengthpreviousline = offsets[i]
            offsets[i] = offset

        self.offsets = offsets
        self.len = linecount
        # Decode lines after offset in file is calculated
        self.lines = [unicode(line, config.charset) for line in lines]


class LogFile:
    """
    .filter: function that gets the values from .parser.
             must return True to keep it or False to remove it
    Overwrite .parser() and .add() to customize this class to special log files
    """

    def __init__(self, filename, buffer_size=4096):
        """
        @param filename: name of the log file
        @param buffer_size: approx. size of one buffer in bytes
        """
        self.loglevel = logging.NOTSET
        self.__filename = filename
        self.__buffer = None # currently used buffer, points to one of the following:
        self.__buffer1 = None
        self.__buffer2 = None
        self.buffer_size = buffer_size
        self.__lineno = 0
        self.filter = None

    def __iter__(self):
        return self

    def reverse(self):
        """ yield log entries in reverse direction starting from last one

        @rtype: iterator
        """
        self.to_end()
        while 1:
            try:
                logging.log(self.loglevel, "LogFile.reverse %s" % self.__filename)
                result = self.previous()
            except StopIteration:
                return
            yield result

    def sanityCheck(self):
        """ Check for log file write access.

        @rtype: string (error message) or None
        """
        if not os.access(self.__filename, os.W_OK):
            return "The log '%s' is not writable!" % (self.__filename, )
        return None

    def __getattr__(self, name):
        """
        generate some attributes when needed
        """
        if name == "_LogFile__rel_index": # Python black magic: this is the real name of the __rel_index attribute
            # starting iteration from begin
            self.__buffer1 = LineBuffer(self._input, 0, self.buffer_size)
            self.__buffer2 = LineBuffer(self._input,
                                        self.__buffer1.offsets[-1],
                                        self.buffer_size)
            self.__buffer = self.__buffer1
            self.__rel_index = 0
            return 0
        elif name == "_input":
            try:
                # Open the file (NOT using codecs.open, it breaks our offset calculation. We decode it later.).
                # Use binary mode in order to retain \r - otherwise the offset calculation would fail.
                self._input = file(self.__filename, "rb", )
            except IOError, err:
                if err.errno == errno.ENOENT: # "file not found"
                    # XXX workaround if edit-log does not exist: just create it empty
                    # if this workaround raises another error, we don't catch
                    # it, so the admin will see it.
                    f = file(self.__filename, "ab")
                    f.write('')
                    f.close()
                    self._input = file(self.__filename, "rb", )
                else:
                    logging.error("logfile: %r IOERROR errno %d (%s)" % (self.__filename, err.errno, os.strerror(err.errno)))
                    raise
            return self._input
        elif name == "_output":
            self._output = codecs.open(self.__filename, 'a', config.charset)
            return self._output
        else:
            raise AttributeError(name)

    def size(self):
        """ Return log size in bytes

        Return 0 if the file does not exist. Raises other OSError.

        @return: size of log file in bytes
        @rtype: Int
        """
        try:
            return os.path.getsize(self.__filename)
        except OSError, err:
            if err.errno == errno.ENOENT:
                return 0
            raise

    def lines(self):
        """ Return number of lines in the log file

        Return 0 if the file does not exist. Raises other OSError.

        Expensive for big log files - O(n)

        @return: size of log file in lines
        @rtype: Int
        """
        try:
            f = file(self.__filename, 'r')
            try:
                count = 0
                for line in f:
                    count += 1
                return count
            finally:
                f.close()
        except (OSError, IOError), err:
            if err.errno == errno.ENOENT:
                return 0
            raise

    def date(self):
        # ToDo check if we need this method
        """ Return timestamp of log file in usecs """
        try:
            mtime = os.path.getmtime(self.__filename)
        except OSError, err:
            if err.errno == errno.ENOENT:
                # This can happen on fresh wiki when building the index
                # Usually the first request will create an event log
                raise LogMissing(str(err))
            raise
        return wikiutil.timestamp2version(mtime)

    def peek(self, lines):
        """ Move position in file forward or backwards by "lines" count

        It adjusts .__lineno if set.
        This function is not aware of filters!

        @param lines: number of lines, may be negative to move backward
        @rtype: boolean
        @return: True if moving more than to the beginning and moving
                 to the end or beyond
        """
        logging.log(self.loglevel, "LogFile.peek %s" % self.__filename)
        self.__rel_index += lines
        while self.__rel_index < 0:
            if self.__buffer is self.__buffer2:
                if self.__buffer.offsets[0] == 0:
                    # already at the beginning of the file
                    self.__rel_index = 0
                    self.__lineno = 0
                    return True
                else:
                    # change to buffer 1
                    self.__buffer = self.__buffer1
                    self.__rel_index += self.__buffer.len
            else: # self.__buffer is self.__buffer1
                if self.__buffer.offsets[0] == 0:
                    # already at the beginning of the file
                    self.__rel_index = 0
                    self.__lineno = 0
                    return True
                else:
                    # load previous lines
                    self.__buffer2 = self.__buffer1
                    self.__buffer1 = LineBuffer(self._input,
                                                self.__buffer.offsets[0],
                                                self.buffer_size,
                                                forward=False)
                    self.__buffer = self.__buffer1
                    self.__rel_index += self.__buffer.len

        while self.__rel_index >= self.__buffer.len:
            if self.__buffer is self.__buffer1:
                # change to buffer 2
                self.__rel_index -= self.__buffer.len
                self.__buffer = self.__buffer2
            else: # self.__buffer is self.__buffer2
                # try to load next buffer
                tmpbuff = LineBuffer(self._input,
                                     self.__buffer.offsets[-1],
                                     self.buffer_size)
                if tmpbuff.len == 0:
                    # end of file
                    if self.__lineno is not None:
                        self.__lineno += (lines -
                                         (self.__rel_index - self.__buffer.len))
                    self.__rel_index = self.__buffer.len # point to after last read line
                    return True
                # shift buffers
                self.__rel_index -= self.__buffer.len
                self.__buffer1 = self.__buffer2
                self.__buffer2 = tmpbuff
                self.__buffer = self.__buffer2

        if self.__lineno is not None:
            self.__lineno += lines
        return False

    def __next(self):
        """get next line already parsed"""
        if self.peek(0):
            raise StopIteration
        result = self.parser(self.__buffer.lines[self.__rel_index])
        self.peek(1)
        return result

    def next(self):
        """get next line that passes through the filter
        @return: next entry
        raises StopIteration at file end
        """
        result = None
        while result is None:
            while result is None:
                logging.log(self.loglevel, "LogFile.next %s" % self.__filename)
                result = self.__next()
            if self.filter and not self.filter(result):
                result = None
        return result

    def __previous(self):
        """get previous line already parsed"""
        if self.peek(-1):
            raise StopIteration
        return self.parser(self.__buffer.lines[self.__rel_index])

    def previous(self):
        """get previous line that passes through the filter
        @return: previous entry
        raises StopIteration at file begin
        """
        result = None
        while result is None:
            while result is None:
                logging.log(self.loglevel, "LogFile.previous %s" % self.__filename)
                result = self.__previous()
            if self.filter and not self.filter(result):
                result = None
        return result

    def to_begin(self):
        """moves file position to the begin"""
        logging.log(self.loglevel, "LogFile.to_begin %s" % self.__filename)
        if self.__buffer1 is None or self.__buffer1.offsets[0] != 0:
            self.__buffer1 = LineBuffer(self._input,
                                        0,
                                        self.buffer_size)
            self.__buffer2 = LineBuffer(self._input,
                                        self.__buffer1.offsets[-1],
                                        self.buffer_size)
        self.__buffer = self.__buffer1
        self.__rel_index = 0
        self.__lineno = 0

    def to_end(self):
        """moves file position to the end"""
        logging.log(self.loglevel, "LogFile.to_end %s" % self.__filename)
        self._input.seek(0, 2) # to end of file
        size = self._input.tell()
        if self.__buffer2 is None or size > self.__buffer2.offsets[-1]:
            self.__buffer2 = LineBuffer(self._input,
                                        size,
                                        self.buffer_size,
                                        forward=False)

            self.__buffer1 = LineBuffer(self._input,
                                        self.__buffer2.offsets[0],
                                        self.buffer_size,
                                        forward=False)
        self.__buffer = self.__buffer2
        self.__rel_index = self.__buffer2.len
        self.__lineno = None

    def position(self):
        """ Return the current file position

        This can be converted into a String using back-ticks and then be rebuild.
        For this plain file implementation position is an Integer.
        """
        return self.__buffer.offsets[self.__rel_index]

    def seek(self, position, line_no=None):
        """ moves file position to an value formerly gotten from .position().
        To enable line counting line_no must be provided.
        .seek is much more efficient for moving long distances than .peek.
        raises ValueError if position is invalid
        """
        logging.log(self.loglevel, "LogFile.seek %s pos %d" % (self.__filename, position))
        if self.__buffer1:
            logging.log(self.loglevel, "b1 %r %r" % (self.__buffer1.offsets[0], self.__buffer1.offsets[-1]))
        if self.__buffer2:
            logging.log(self.loglevel, "b2 %r %r" % (self.__buffer2.offsets[0], self.__buffer2.offsets[-1]))
        if self.__buffer1 and self.__buffer1.offsets[0] <= position < self.__buffer1.offsets[-1]:
            # position is in .__buffer1
            self.__rel_index = self.__buffer1.offsets.index(position)
            self.__buffer = self.__buffer1
        elif self.__buffer2 and self.__buffer2.offsets[0] <= position < self.__buffer2.offsets[-1]:
            # position is in .__buffer2
            self.__rel_index = self.__buffer2.offsets.index(position)
            self.__buffer = self.__buffer2
        elif self.__buffer1 and self.__buffer1.offsets[-1] == position:
            # we already have one buffer directly before where we want to go
            self.__buffer2 = LineBuffer(self._input,
                                        position,
                                        self.buffer_size)
            self.__buffer = self.__buffer2
            self.__rel_index = 0
        elif self.__buffer2 and self.__buffer2.offsets[-1] == position:
            # we already have one buffer directly before where we want to go
            self.__buffer1 = self.__buffer2
            self.__buffer2 = LineBuffer(self._input,
                                        position,
                                        self.buffer_size)
            self.__buffer = self.__buffer2
            self.__rel_index = 0
        else:
            # load buffers around position
            self.__buffer1 = LineBuffer(self._input,
                                        position,
                                        self.buffer_size,
                                        forward=False)
            self.__buffer2 = LineBuffer(self._input,
                                        position,
                                        self.buffer_size)
            self.__buffer = self.__buffer2
            self.__rel_index = 0
            # XXX test for valid position
        self.__lineno = line_no

    def line_no(self):
        """@return: the current line number or None if line number is unknown"""
        return self.__lineno

    def calculate_line_no(self):
        """ Calculate the current line number from buffer offsets

        If line number is unknown it is calculated by parsing the whole file.
        This may be expensive.
        """
        self._input.seek(0, 0)
        lines = self._input.read(self.__buffer.offsets[self.__rel_index])
        self.__lineno = len(lines.splitlines())
        return self.__lineno

    def parser(self, line):
        """
        @param line: line as read from file
        @return: parsed line or None on error
        Converts the line from file to program representation
        This implementation uses TAB separated strings.
        This method should be overwritten by the sub classes.
        """
        return line.split("\t")

    def add(self, *data):
        """
        add line to log file
        This implementation save the values as TAB separated strings.
        This method should be overwritten by the sub classes.
        """
        line = "\t".join(data)
        self._add(line)

    def _add(self, line):
        """
        @param line: flat line
        @type line: String
        write on entry in the log file
        """
        if line is not None:
            if line[-1] != '\n':
                line += '\n'
            self._output.write(line)
            self._output.close() # does this maybe help against the sporadic fedora wikis 160 \0 bytes in the edit-log?
            del self._output # re-open the output file automagically

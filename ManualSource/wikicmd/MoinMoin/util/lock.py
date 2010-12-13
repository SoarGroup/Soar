# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - locking functions

    @copyright: 2005 Florian Festi, Nir Soffer,
                2008 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""

import os, sys, tempfile, time, errno

from MoinMoin import log
logging = log.getLogger(__name__)

from MoinMoin.util import filesys

class Timer:
    """ Simple count down timer

    Useful for code that needs to complete a task within some timeout.
    """
    defaultSleep = 0.25
    maxSleep = 0.25

    def __init__(self, timeout):
        self.setTimeout(timeout)
        self._start = None
        self._stop = None

    def setTimeout(self, timeout):
        self.timeout = timeout
        if timeout is None:
            self._sleep = self.defaultSleep
        else:
            self._sleep = min(timeout / 10.0, self.maxSleep)

    def start(self):
        """ Start the countdown """
        if self.timeout is None:
            return
        now = time.time()
        self._start = now
        self._stop = now + self.timeout

    def haveTime(self):
        """ Check if timeout has not passed """
        if self.timeout is None:
            return True
        return time.time() <= self._stop

    def sleep(self):
        """ Sleep without sleeping over timeout """
        if self._stop is not None:
            timeLeft = max(self._stop - time.time(), 0)
            sleep = min(self._sleep, timeLeft)
        else:
            sleep = self._sleep
        time.sleep(sleep)

    def elapsed(self):
        return time.time() - self._start


class ExclusiveLock:
    """ Exclusive lock

    Uses a directory as portable lock method. On all platforms,
    creating a directory will fail if the directory exists.

    Only one exclusive lock per resource is allowed. This lock is not
    used directly by clients, but used by both ReadLock and WriteLock.

    If created with a timeout, the lock will expire timeout seconds
    after it has been acquired. Without a timeout, it will never expire.
    """
    fileName = '' # The directory is the lockDir
    timerClass = Timer

    def __init__(self, dir, timeout=None):
        """ Init a write lock

        @param dir: the lock directory. Since this lock uses a empty
            filename, the dir is the lockDir.
        @param timeout: while trying to acquire, the lock will expire
            other exclusive locks older than timeout.
            WARNING: because of file system timing limitations, timeouts
            must be at least 2 seconds.
        """
        self.dir = dir
        if timeout is not None and timeout < 2.0:
            raise ValueError('timeout must be at least 2 seconds')
        self.timeout = timeout
        if self.fileName:
            self.lockDir = os.path.join(dir, self.fileName)
            self._makeDir()
        else:
            self.lockDir = dir
        self._locked = False

    def acquire(self, timeout=None):
        """ Try to acquire a lock.

        Try to create the lock directory. If it fails because another
        lock exists, try to expire the other lock. Repeat after little
        sleep until timeout passed.

        Return True if a lock was acquired; False otherwise.
        """
        timer = self.timerClass(timeout)
        timer.start()
        while timer.haveTime():
            try:
                filesys.mkdir(self.lockDir)
                self._locked = True
                logging.debug('acquired exclusive lock: %s' % (self.lockDir, ))
                return True
            except OSError, err:
                if err.errno != errno.EEXIST:
                    raise
                if self.expire():
                    continue # Try immediately to acquire
                timer.sleep()
        logging.debug('failed to acquire exclusive lock: %s' % (self.lockDir, ))
        return False

    def release(self):
        """ Release the lock """
        if not self._locked:
            raise RuntimeError('lock already released: %s' % self.lockDir)
        self._removeLockDir()
        self._locked = False
        logging.debug('released lock: %s' % self.lockDir)

    def isLocked(self):
        return self._locked

    def exists(self):
        return os.path.exists(self.lockDir)

    def isExpired(self):
        """ Return True if too old or missing; False otherwise

        TODO: Since stat returns times using whole seconds, this is
        quite broken. Maybe use OS specific calls like Carbon.File on
        Mac OS X?
        """
        if self.timeout is None:
            return not self.exists()
        try:
            lock_age = time.time() - filesys.stat(self.lockDir).st_mtime
            return lock_age > self.timeout
        except OSError, err:
            if err.errno == errno.ENOENT:
                # No such lock file, therefore "expired"
                return True
            raise

    def expire(self):
        """ Return True if the lock is expired or missing; False otherwise. """
        if self.isExpired():
            self._removeLockDir()
            logging.debug("expired lock: %s" % self.lockDir)
            return True
        return False

    # Private -------------------------------------------------------

    def _makeDir(self):
        """ Make sure directory exists """
        try:
            filesys.mkdir(self.dir)
            logging.debug('created directory: %s' % self.dir)
        except OSError, err:
            if err.errno != errno.EEXIST:
                raise

    def _removeLockDir(self):
        """ Remove lockDir ignoring 'No such file or directory' errors """
        try:
            filesys.rmdir(self.lockDir)
            logging.debug('removed directory: %s' % self.dir)
        except OSError, err:
            if err.errno != errno.ENOENT:
                raise


class WriteLock(ExclusiveLock):
    """ Exclusive Read/Write Lock

    When a resource is locked with this lock, clients can't read
    or write the resource.

    This super-exclusive lock can't be acquired if there are any other
    locks, either WriteLock or ReadLocks. When trying to acquire, this
    lock will try to expire all existing ReadLocks.
    """
    fileName = 'write_lock'

    def __init__(self, dir, timeout=None, readlocktimeout=None):
        """ Init a write lock

        @param dir: the lock directory. Every resource should have one
            lock directory, which may contain read or write locks.
        @param timeout: while trying to acquire, the lock will expire
            other unreleased write locks older than timeout.
        @param readlocktimeout: while trying to acquire, the lock will
            expire other read locks older than readlocktimeout.
        """
        ExclusiveLock.__init__(self, dir, timeout)
        if readlocktimeout is None:
            self.readlocktimeout = timeout
        else:
            self.readlocktimeout = readlocktimeout

    def acquire(self, timeout=None):
        """ Acquire an exclusive write lock

        Try to acquire an exclusive lock, then try to expire existing
        read locks. If timeout has not passed, the lock is acquired.
        Otherwise, the exclusive lock is released and the lock is not
        acquired.

        Return True if lock acquired, False otherwise.
        """
        if self._locked:
            raise RuntimeError("lock already locked")
        result = False
        timer = self.timerClass(timeout)
        timer.start()
        if ExclusiveLock.acquire(self, timeout):
            try:
                while timer.haveTime():
                    self._expireReadLocks()
                    if not self._haveReadLocks():
                        result = timer.haveTime()
                        break
                    timer.sleep()
            finally:
                if result:
                    logging.debug('acquired write lock: %s' % self.lockDir)
                    return True
                else:
                    self.release()
        return False

    # Private -------------------------------------------------------

    def _expireReadLocks(self):
        """ Expire old read locks """
        readLockFileName = ReadLock.fileName
        for name in os.listdir(self.dir):
            if not name.startswith(readLockFileName):
                continue
            LockDir = os.path.join(self.dir, name)
            ExclusiveLock(LockDir, self.readlocktimeout).expire()

    def _haveReadLocks(self):
        """ Return True if read locks exists; False otherwise """
        readLockFileName = ReadLock.fileName
        for name in os.listdir(self.dir):
            if name.startswith(readLockFileName):
                return True
        return False

class ReadLock(ExclusiveLock):
    """ Read lock

    The purpose of this lock is to mark the resource as read only.
    Multiple ReadLocks can be acquired for same resource, but no
    WriteLock can be acquired until all ReadLocks are released.

    Allows only one lock per instance.
    """
    fileName = 'read_lock_'

    def __init__(self, dir, timeout=None):
        """ Init a read lock

        @param dir: the lock directory. Every resource should have one
            lock directory, which may contain read or write locks.
        @param timeout: while trying to acquire, the lock will expire
            other unreleased write locks older than timeout.
        """
        ExclusiveLock.__init__(self, dir, timeout)
        writeLockDir = os.path.join(self.dir, WriteLock.fileName)
        self.writeLock = ExclusiveLock(writeLockDir, timeout)

    def acquire(self, timeout=None):
        """ Try to acquire a 'read' lock

        To prevent race conditions, acquire first an exclusive lock,
        then acquire a read lock. Finally release the exclusive lock so
        other can have read lock, too.
        """
        if self._locked:
            raise RuntimeError("lock already locked")
        if self.writeLock.acquire(timeout):
            try:
                self.lockDir = tempfile.mkdtemp('', self.fileName, self.dir)
                self._locked = True
                logging.debug('acquired read lock: %s' % self.lockDir)
                return True
            finally:
                self.writeLock.release()
        return False


class LazyReadLock(ReadLock):
    """ Lazy Read lock

    See ReadLock, but we do an optimization here:
    If (and ONLY if) the resource protected by this lock is updated in a POSIX
    style "write new content to tmpfile, rename tmpfile -> origfile", then reading
    from an open origfile handle will give either the old content (when opened
    before the rename happens) or the new content (when opened after the rename
    happened), but never cause any trouble. This means that we don't have to lock
    at all in that case.

    Of course this doesn't work for us on the win32 platform:
    * using MoveFileEx requires opening the file with some FILE_SHARE_DELETE
      mode - we currently don't do that
    * Win 95/98/ME do not have MoveFileEx
    We currently solve by using the non-lazy locking code in ReadLock class.
    """
    def __init__(self, dir, timeout=None):
        if sys.platform == 'win32':
            ReadLock.__init__(self, dir, timeout)
        else: # POSIX
            self._locked = False

    def acquire(self, timeout=None):
        if sys.platform == 'win32':
            return ReadLock.acquire(self, timeout)
        else: # POSIX
            self._locked = True
            return True

    def release(self):
        if sys.platform == 'win32':
            return ReadLock.release(self)
        else:  # POSIX
            self._locked = False

    def exists(self):
        if sys.platform == 'win32':
            return ReadLock.exists(self)
        else: # POSIX
            return True

    def isExpired(self):
        if sys.platform == 'win32':
            return ReadLock.isExpired(self)
        else: # POSIX
            return True

    def expire(self):
        if sys.platform == 'win32':
            return ReadLock.expire(self)
        else: # POSIX
            return True

class LazyWriteLock(WriteLock):
    """ Lazy Write lock

    See WriteLock and LazyReadLock docs.
    """
    def __init__(self, dir, timeout=None):
        if sys.platform == 'win32':
            WriteLock.__init__(self, dir, timeout)
        else: # POSIX
            self._locked = False

    def acquire(self, timeout=None):
        if sys.platform == 'win32':
            return WriteLock.acquire(self, timeout)
        else: # POSIX
            self._locked = True
            return True

    def release(self):
        if sys.platform == 'win32':
            return WriteLock.release(self)
        else:  # POSIX
            self._locked = False

    def exists(self):
        if sys.platform == 'win32':
            return WriteLock.exists(self)
        else: # POSIX
            return True

    def isExpired(self):
        if sys.platform == 'win32':
            return WriteLock.isExpired(self)
        else: # POSIX
            return True

    def expire(self):
        if sys.platform == 'win32':
            return WriteLock.expire(self)
        else: # POSIX
            return True

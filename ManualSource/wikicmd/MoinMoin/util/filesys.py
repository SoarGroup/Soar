# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - File System Utilities

    @copyright: 2002 Juergen Hermann <jh@web.de>,
                2006-2008 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""

import sys, os, shutil, time, errno
from stat import S_ISDIR, ST_MODE, S_IMODE
import warnings

from MoinMoin import log
logging = log.getLogger(__name__)

#############################################################################
### Misc Helpers
#############################################################################

def chmod(name, mode, catchexception=True):
    """ change mode of some file/dir on platforms that support it.
        usually you don't need this because we use os.umask() when importing
        request.py
    """
    try:
        os.chmod(name, mode)
    except OSError:
        if not catchexception:
            raise


# begin copy of werkzeug.posixemulation from werkzeug 0.6.1(pre) repo
r"""
    werkzeug.posixemulation
    ~~~~~~~~~~~~~~~~~~~~~~~

    Provides a POSIX emulation for some features that are relevant to
    web applications.  The main purpose is to simplify support for
    systems such as Windows NT that are not 100% POSIX compatible.

    Currently this only implements a :func:`rename` function that
    follows POSIX semantics.  Eg: if the target file already exists it
    will be replaced without asking.

    This module was introduced in 0.6.1 and is not a public interface.
    It might become one in later versions of Werkzeug.

    :copyright: (c) 2010 by the Werkzeug Team, see AUTHORS for more details.
    :license: BSD, see LICENSE for more details.
"""
import os
import errno
import random


can_rename_open_file = False
if os.name == 'nt':
    _rename = lambda src, dst: False
    _rename_atomic = lambda src, dst: False

    try:
        import ctypes
        _GetLastError = ctypes.windll.kernel32.GetLastError

        def _LogLastError(fn):
            err = _GetLastError()
            logging.debug("%s returned: %r" % (fn, err))
            # 5 = Access Denied
            # 6800 = The function attempted to use a name that is reserved
            #        for use by another transaction.

        _MOVEFILE_REPLACE_EXISTING = 0x1
        _MOVEFILE_WRITE_THROUGH = 0x8
        _MoveFileEx = ctypes.windll.kernel32.MoveFileExW

        def _rename(src, dst):
            if not isinstance(src, unicode):
                src = unicode(src, sys.getfilesystemencoding())
            if not isinstance(dst, unicode):
                dst = unicode(dst, sys.getfilesystemencoding())
            if _rename_atomic(src, dst):
                return True
            logging.debug("PSEUDO-atomic rename %r %r" % (src, dst))
            retry = 0
            ret = 0
            while not ret and retry < 100:
                ret = _MoveFileEx(src, dst, _MOVEFILE_REPLACE_EXISTING |
                                            _MOVEFILE_WRITE_THROUGH)
                if ret == 0:
                    _LogLastError("MoveFileExW")
                    time.sleep(0.001)
                    retry += 1
                    logging.debug("retry %d after sleeping" % retry)
            return ret

        # this stuff only exists in windows >= vista / windows server 2008
        _CreateTransaction = ctypes.windll.ktmw32.CreateTransaction
        _CommitTransaction = ctypes.windll.ktmw32.CommitTransaction
        _MoveFileTransacted = ctypes.windll.kernel32.MoveFileTransactedW
        _CloseHandle = ctypes.windll.kernel32.CloseHandle
        can_rename_open_file = True

        def _rename_atomic(src, dst):
            ta = _CreateTransaction(None, 0, 0, 0, 0, 1000, 'Werkzeug rename')
            if ta == -1:
                _LogLastError("CreateTransaction")
                return False
            try:
                logging.debug("atomic rename %r %r" % (src, dst))
                retry = 0
                ret = 0
                while not ret and retry < 100:
                    ret = _MoveFileTransacted(src, dst, None, None,
                                               _MOVEFILE_REPLACE_EXISTING |
                                               _MOVEFILE_WRITE_THROUGH, ta)
                    if ret == 0:
                        _LogLastError("MoveFileTransacted")
                        time.sleep(0.001)
                        retry += 1
                        logging.debug("retry %d after sleeping" % retry)
                    else:
                        ret = _CommitTransaction(ta)
                        if ret == 0:
                            _LogLastError("CommitTransaction")
                return ret
            finally:
                ret = _CloseHandle(ta)
                if ret == 0:
                    _LogLastError("CloseHandle")
    except Exception:
        pass

    def rename(src, dst):
        # Try atomic or pseudo-atomic rename
        if _rename(src, dst):
            return
        # Fall back to "move away and replace"
        logging.debug("NON-atomic rename %r %r" % (src, dst))
        try:
            os.rename(src, dst)
        except OSError, e:
            if e.errno != errno.EEXIST:
                raise
            old = "%s-%08x" % (dst, random.randint(0, sys.maxint))
            os.rename(dst, old)
            os.rename(src, dst)
            try:
                os.unlink(old)
            except Exception:
                pass
else:
    #logging.debug("atomic os.rename (POSIX)")
    rename = os.rename
    can_rename_open_file = True

# end copy of werkzeug.posixemulation

rename_overwrite = rename

def rename_no_overwrite(oldname, newname, delete_old=False):
    """ Multiplatform rename

    This kind of rename is doing things differently: it fails if newname
    already exists. This is the usual thing on win32, but not on posix.

    If delete_old is True, oldname is removed in any case (even if the
    rename did not succeed).
    """
    if os.name == 'nt':
        try:
            try:
                os.rename(oldname, newname)
                success = True
            except:
                success = False
                raise
        finally:
            if not success and delete_old:
                os.unlink(oldname)
    else:
        try:
            try:
                os.link(oldname, newname)
                success = True
            except:
                success = False
                raise
        finally:
            if success or delete_old:
                os.unlink(oldname)


def touch(name):
    if sys.platform == 'win32':
        import win32file, win32con, pywintypes

        access = win32file.GENERIC_WRITE
        share = (win32file.FILE_SHARE_DELETE |
                 win32file.FILE_SHARE_READ |
                 win32file.FILE_SHARE_WRITE)
        create = win32file.OPEN_EXISTING
        mtime = time.gmtime()
        handle = win32file.CreateFile(name, access, share, None, create,
                                      win32file.FILE_ATTRIBUTE_NORMAL |
                                      win32con.FILE_FLAG_BACKUP_SEMANTICS,
                                      None)
        try:
            newTime = pywintypes.Time(mtime)
            win32file.SetFileTime(handle, newTime, newTime, newTime)
        finally:
            win32file.CloseHandle(handle)
    else:
        os.utime(name, None)


def access_denied_decorator(fn):
    """ Due to unknown reasons, some os.* functions on Win32 sometimes fail
        with Access Denied (although access should be possible).
        Just retrying it a bit later works and this is what we do.
    """
    if sys.platform == 'win32':
        def wrapper(*args, **kwargs):
            max_retries = 42
            retry = 0
            while True:
                try:
                    return fn(*args, **kwargs)
                except OSError, err:
                    retry += 1
                    if retry > max_retries:
                        raise
                    if err.errno == errno.EACCES:
                        logging.warning('%s(%r, %r) -> access denied. retrying...' % (fn.__name__, args, kwargs))
                        time.sleep(0.01)
                        continue
                    raise
        return wrapper
    else:
        return fn

stat = access_denied_decorator(os.stat)
mkdir = access_denied_decorator(os.mkdir)
rmdir = access_denied_decorator(os.rmdir)


def fuid(filename, max_staleness=3600):
    """ return a unique id for a file

        Using just the file's mtime to determine if the file has changed is
        not reliable - if file updates happen faster than the file system's
        mtime granularity, then the modification is not detectable because
        the mtime is still the same.

        This function tries to improve by using not only the mtime, but also
        other metadata values like file size and inode to improve reliability.

        For the calculation of this value, we of course only want to use data
        that we can get rather fast, thus we use file metadata, not file data
        (file content).

        Note: depending on the operating system capabilities and the way the
              file update is done, this function might return the same value
              even if the file has changed. It should be better than just
              using file's mtime though.
              max_staleness tries to avoid the worst for these cases.

        @param filename: file name of the file to look at
        @param max_staleness: if a file is older than that, we may consider
                              it stale and return a different uid - this is a
                              dirty trick to work around changes never being
                              detected. Default is 3600 seconds, use None to
                              disable this trickery. See below for more details.
        @return: an object that changes value if the file changed,
                 None is returned if there were problems accessing the file
    """
    try:
        st = os.stat(filename)
    except (IOError, OSError):
        uid = None  # for permanent errors on stat() this does not change, but
                    # having a changing value would be pointless because if we
                    # can't even stat the file, it is unlikely we can read it.
    else:
        fake_mtime = int(st.st_mtime)
        if not st.st_ino and max_staleness:
            # st_ino being 0 likely means that we run on a platform not
            # supporting it (e.g. win32) - thus we likely need this dirty
            # trick
            now = int(time.time())
            if now >= st.st_mtime + max_staleness:
                # keep same fake_mtime for each max_staleness interval
                fake_mtime = int(now / max_staleness) * max_staleness
        uid = (st.st_mtime,  # might have a rather rough granularity, e.g. 2s
                             # on FAT, 1s on ext3 and might not change on fast
                             # updates
               st.st_ino,  # inode number (will change if the update is done
                           # by e.g. renaming a temp file to the real file).
                           # not supported on win32 (0 ever)
               st.st_size,  # likely to change on many updates, but not
                            # sufficient alone
               fake_mtime,  # trick to workaround file system / platform
                            # limitations causing permanent trouble
              )
    return uid


def copystat(src, dst):
    """Copy stat bits from src to dst

    This should be used when shutil.copystat would be used on directories
    on win32 because win32 does not support utime() for directories.

    According to the official docs written by Microsoft, it returns ENOACCES if the
    supplied filename is a directory. Looks like a trainee implemented the function.
    """
    if sys.platform == 'win32' and S_ISDIR(os.stat(dst)[ST_MODE]):
        if os.name == 'nt':
            st = os.stat(src)
            mode = S_IMODE(st[ST_MODE])
            if hasattr(os, 'chmod'):
                os.chmod(dst, mode) # KEEP THIS ONE!
        #else: pass # we are on Win9x,ME - no chmod here
    else:
        shutil.copystat(src, dst)


def copytree(src, dst, symlinks=False):
    """Recursively copy a directory tree using copy2().

    The destination directory must not already exist.
    If exception(s) occur, an Error is raised with a list of reasons.

    If the optional symlinks flag is true, symbolic links in the
    source tree result in symbolic links in the destination tree; if
    it is false, the contents of the files pointed to by symbolic
    links are copied.

    In contrary to shutil.copytree, this version also copies directory
    stats, not only file stats.

    """
    names = os.listdir(src)
    os.mkdir(dst)
    copystat(src, dst)
    errors = []
    for name in names:
        srcname = os.path.join(src, name)
        dstname = os.path.join(dst, name)
        try:
            if symlinks and os.path.islink(srcname):
                linkto = os.readlink(srcname)
                os.symlink(linkto, dstname)
            elif os.path.isdir(srcname):
                copytree(srcname, dstname, symlinks)
            else:
                shutil.copy2(srcname, dstname)
            # XXX What about devices, sockets etc.?
        except (IOError, os.error), why:
            errors.append((srcname, dstname, why))
    if errors:
        raise EnvironmentError, errors

# Code could come from http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/65203

# we currently do not support locking
LOCK_EX = LOCK_SH = LOCK_NB = 0

def lock(file, flags):
    raise NotImplementedError

def unlock(file):
    raise NotImplementedError


# ----------------------------------------------------------------------
# Get real case of path name on case insensitive file systems
# TODO: nt version?

if sys.platform == 'darwin':

    def realPathCase(path):
        """ Return the real case of path e.g. PageName for pagename

        HFS and HFS+ file systems, are case preserving but case
        insensitive. You can't have 'file' and 'File' in the same
        directory, but you can get the real name of 'file'.

        @param path: string
        @rtype: string
        @return the real case of path or None
        """
        try:
            from Carbon import File
            try:
                return File.FSRef(path).as_pathname()
            except File.Error:
                return None
        except ImportError:
            return None

else:

    def realPathCase(path):
        return None

# dircache stuff seems to be broken on win32 (at least for FAT32, maybe NTFS).
# dircache stuff is also broken on POSIX if updates happen too fast (< 1s).
DCENABLED = 0 # set to 0 to completely disable dircache usage

# Note: usage of the dc* functions below is deprecated, they'll get removed soon.
dc_deprecated = "dircache function calls (dcdisable,dclistdir,dcreset) are deprecated, please fix caller"

def dcdisable():
    warnings.warn(dc_deprecated, DeprecationWarning, stacklevel=2)
    global DCENABLED
    DCENABLED = 0

import dircache

def dclistdir(path):
    warnings.warn(dc_deprecated, DeprecationWarning, stacklevel=2)
    if sys.platform == 'win32' or not DCENABLED:
        return os.listdir(path)
    else:
        return dircache.listdir(path)

def dcreset():
    warnings.warn(dc_deprecated, DeprecationWarning, stacklevel=2)
    if sys.platform == 'win32' or not DCENABLED:
        return
    else:
        return dircache.reset()


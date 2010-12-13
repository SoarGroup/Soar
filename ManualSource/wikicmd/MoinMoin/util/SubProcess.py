"""
Enhanced subprocess.Popen subclass, supporting:
    * .communicate() with timeout
    * kill/terminate/send_signal (like in Py 2.6) for Py 2.4 / 2.5

Sample usage:
    out, err = Popen(...).communicate(input, timeout=300)
"""

import os
import subprocess
import threading
import signal

if subprocess.mswindows:
    try:
        # Python >= 2.6 should have this:
        from _subprocess import TerminateProcess
    except ImportError:
        # otherwise you need win32 extensions:
        from win32process import TerminateProcess
else:
    import select
    import errno

class Popen(subprocess.Popen):
    # send_signal, terminate, kill copied from Python 2.6
    # (we want to support Python >= 2.4)
    if subprocess.mswindows:
        def send_signal(self, sig):
            """Send a signal to the process
            """
            if sig == signal.SIGTERM:
                self.terminate()
            else:
                raise ValueError("Only SIGTERM is supported on Windows")

        def terminate(self):
            """Terminates the process
            """
            TerminateProcess(self._handle, 1)

        kill = terminate

    else: # POSIX
        def send_signal(self, sig):
            """Send a signal to the process
            """
            os.kill(self.pid, sig)

        def terminate(self):
            """Terminate the process with SIGTERM
            """
            self.send_signal(signal.SIGTERM)

        def kill(self):
            """Kill the process with SIGKILL
            """
            self.send_signal(signal.SIGKILL)

    def communicate(self, input=None, timeout=None):
        """Interact with process: Send data to stdin.  Read data from
        stdout and stderr, until end-of-file is reached.  Wait for
        process to terminate.  The optional input argument should be a
        string to be sent to the child process, or None, if no data
        should be sent to the child.

        communicate() returns a tuple (stdout, stderr)."""

        self.timeout = timeout

        # Optimization: If we are only using one pipe, or no pipe at
        # all, using select() or threads is unnecessary.
        if [self.stdin, self.stdout, self.stderr].count(None) >= 2:
            stdout = None
            stderr = None
            if self.stdin:
                if input:
                    self._fo_write_no_intr(self.stdin, input)
                self.stdin.close()
            elif self.stdout:
                stdout = self._fo_read_no_intr(self.stdout)
                self.stdout.close()
            elif self.stderr:
                stderr = self._fo_read_no_intr(self.stderr)
                self.stderr.close()
            self.wait()
            return (stdout, stderr)

        return self._communicate(input)

    if subprocess.mswindows:
        def _communicate(self, input):
            stdout = None # Return
            stderr = None # Return

            if self.stdout:
                stdout = []
                stdout_thread = threading.Thread(target=self._readerthread,
                                                 args=(self.stdout, stdout))
                stdout_thread.setDaemon(True)
                stdout_thread.start()
            if self.stderr:
                stderr = []
                stderr_thread = threading.Thread(target=self._readerthread,
                                                 args=(self.stderr, stderr))
                stderr_thread.setDaemon(True)
                stderr_thread.start()

            if self.stdin:
                if input is not None:
                    self.stdin.write(input)
                self.stdin.close()

            if self.stdout:
                stdout_thread.join(self.timeout)
            if self.stderr:
                stderr_thread.join(self.timeout)

            # if the threads are still alive, that means the thread join timed out
            timed_out = (self.stdout and stdout_thread.isAlive() or
                         self.stderr and stderr_thread.isAlive())
            if timed_out:
                self.kill()
            else:
                self.wait()

            # All data exchanged.  Translate lists into strings.
            if stdout is not None:
                stdout = stdout[0]
            if stderr is not None:
                stderr = stderr[0]

            # Translate newlines, if requested.  We cannot let the file
            # object do the translation: It is based on stdio, which is
            # impossible to combine with select (unless forcing no
            # buffering).
            if self.universal_newlines and hasattr(file, 'newlines'):
                if stdout:
                    stdout = self._translate_newlines(stdout)
                if stderr:
                    stderr = self._translate_newlines(stderr)

            return (stdout, stderr)

    else: # POSIX
        def _communicate(self, input):
            timed_out = False
            read_set = []
            write_set = []
            stdout = None # Return
            stderr = None # Return

            if self.stdin:
                # Flush stdio buffer.  This might block, if the user has
                # been writing to .stdin in an uncontrolled fashion.
                self.stdin.flush()
                if input:
                    write_set.append(self.stdin)
                else:
                    self.stdin.close()
            if self.stdout:
                read_set.append(self.stdout)
                stdout = []
            if self.stderr:
                read_set.append(self.stderr)
                stderr = []

            input_offset = 0
            while read_set or write_set:
                try:
                    rlist, wlist, xlist = select.select(read_set, write_set, [], self.timeout)
                except select.error, e:
                    if e.args[0] == errno.EINTR:
                        continue
                    raise

                timed_out = not (rlist or wlist or xlist)
                if timed_out:
                    break

                if self.stdin in wlist:
                    # When select has indicated that the file is writable,
                    # we can write up to PIPE_BUF bytes without risk
                    # blocking.  POSIX defines PIPE_BUF >= 512
                    chunk = input[input_offset:input_offset + 512]
                    bytes_written = os.write(self.stdin.fileno(), chunk)
                    input_offset += bytes_written
                    if input_offset >= len(input):
                        self.stdin.close()
                        write_set.remove(self.stdin)

                if self.stdout in rlist:
                    data = os.read(self.stdout.fileno(), 1024)
                    if data == "":
                        self.stdout.close()
                        read_set.remove(self.stdout)
                    stdout.append(data)

                if self.stderr in rlist:
                    data = os.read(self.stderr.fileno(), 1024)
                    if data == "":
                        self.stderr.close()
                        read_set.remove(self.stderr)
                    stderr.append(data)

            # All data exchanged.  Translate lists into strings.
            if stdout is not None:
                stdout = ''.join(stdout)
            if stderr is not None:
                stderr = ''.join(stderr)

            # Translate newlines, if requested.  We cannot let the file
            # object do the translation: It is based on stdio, which is
            # impossible to combine with select (unless forcing no
            # buffering).
            if self.universal_newlines and hasattr(file, 'newlines'):
                if stdout:
                    stdout = self._translate_newlines(stdout)
                if stderr:
                    stderr = self._translate_newlines(stderr)

            if timed_out:
                self.kill()
            else:
                self.wait()
            return (stdout, stderr)


def exec_cmd(cmd, input=None, timeout=None):
    p = Popen(cmd, shell=True,
              close_fds=not subprocess.mswindows,
              bufsize=1024,
              stdin=subprocess.PIPE,
              stdout=subprocess.PIPE,
              stderr=subprocess.PIPE)
    data, errors = p.communicate(input, timeout=timeout)
    return data, errors, p.returncode


if __name__ == '__main__':
    print exec_cmd("python", "import time ; time.sleep(20) ; print 'never!' ;", timeout=10)
    print exec_cmd("python", "import time ; time.sleep(20) ; print '20s gone' ;")


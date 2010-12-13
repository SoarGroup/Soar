# copied from email.Header because the original is broken

# Copyright (C) 2002-2004 Python Software Foundation
# Author: Ben Gertzfield, Barry Warsaw
# Contact: email-sig@python.org

import sys, binascii

from email.Header import ecre

import email.quopriMIME
import email.base64MIME
from email.Errors import HeaderParseError
from email.Charset import Charset

SPACE = ' '

if sys.version_info[:3] < (2, 9, 0): # insert the version number
                                     # of a fixed python here

    def decode_header(header):
        """Decode a message header value without converting charset.
    
        Returns a list of (decoded_string, charset) pairs containing each of the
        decoded parts of the header.  Charset is None for non-encoded parts of the
        header, otherwise a lower-case string containing the name of the character
        set specified in the encoded string.
    
        An email.Errors.HeaderParseError may be raised when certain decoding error
        occurs (e.g. a base64 decoding exception).
        """
        # If no encoding, just return the header
        header = str(header)
        if not ecre.search(header):
            return [(header, None)]
        decoded = []
        dec = ''
        for line in header.splitlines():
            # This line might not have an encoding in it
            if not ecre.search(line):
                decoded.append((line, None))
                continue
            parts = ecre.split(line)
            while parts:
                unenc = parts.pop(0).rstrip()
                if unenc:
                    # Should we continue a long line?
                    if decoded and decoded[-1][1] is None:
                        decoded[-1] = (decoded[-1][0] + SPACE + unenc, None)
                    else:
                        decoded.append((unenc, None))
                if parts:
                    charset, encoding = [s.lower() for s in parts[0:2]]
                    encoded = parts[2]
                    dec = None
                    if encoding == 'q':
                        dec = email.quopriMIME.header_decode(encoded)
                    elif encoding == 'b':
                        try:
                            dec = email.base64MIME.decode(encoded)
                        except binascii.Error:
                            # Turn this into a higher level exception.  BAW: Right
                            # now we throw the lower level exception away but
                            # when/if we get exception chaining, we'll preserve it.
                            raise HeaderParseError
                    if dec is None:
                        dec = encoded
    
                    if decoded and decoded[-1][1] == charset:
                        decoded[-1] = (decoded[-1][0] + dec, decoded[-1][1])
                    else:
                        decoded.append((dec, charset))
                del parts[0:3]
        return decoded

else:
    from email.Header import decode_header

# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - site-wide configuration defaults (NOT per single wiki!)

    @copyright: 2005-2006 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""
import re
from MoinMoin import version

# unicode: set the char types (upper, lower, digits, spaces)
from MoinMoin.util.chartypes import *

# List of image types browser do support regulary
browser_supported_images = ('gif', 'jpg', 'jpeg', 'png', 'bmp', 'ico', )

# Parser to use mimetype text
parser_text_mimetype = ('plain', 'csv', 'rst', 'docbook', 'latex', 'tex', 'html', 'css',
                       'xml', 'python', 'perl', 'php', 'ruby', 'javascript',
                       'cplusplus', 'java', 'pascal', 'diff', 'gettext', 'xslt', 'creole', )

# When creating files, we use e.g. 0666 & config.umask for the mode:
umask = 0770

# Default value for the static stuff URL prefix (css, img, js).
# Caution:
# * do NOT use this directly, it is only the DEFAULT value to be used by
#   server Config classes and by multiconfig.py for request.cfg.
# * must NOT end with '/'!
# * some servers expect '/' at beginning and only 1 level deep.
url_prefix_static = '/moin_static' + version.release_short

# Threads flag - if you write a moin server that use threads, import
# config in the server and set this flag to True.
use_threads = False

# Charset - we support only 'utf-8'. While older encodings might work,
# we don't have the resources to test them, and there is no real
# benefit for the user. IMPORTANT: use only lowercase 'utf-8'!
charset = 'utf-8'

# Regex to find lower->upper transitions (word boundaries in WikiNames), used by split_title
split_regex = re.compile('([%s])([%s])' % (chars_lower, chars_upper), re.UNICODE)

# Invalid characters - invisible characters that should not be in page
# names. Prevent user confusion and wiki abuse, e.g u'\u202aFrontPage'.
page_invalid_chars_regex = re.compile(
    ur"""
    \u0000 | # NULL

    # Bidi control characters
    \u202A | # LRE
    \u202B | # RLE
    \u202C | # PDF
    \u202D | # LRM
    \u202E   # RLM
    """,
    re.UNICODE | re.VERBOSE
    )

# used for wikiutil.clean_input
clean_input_translation_map = {
    # these chars will be replaced by blanks
    ord(u'\t'): u' ',
    ord(u'\r'): u' ',
    ord(u'\n'): u' ',
}
for c in u'\x00\x01\x02\x03\x04\x05\x06\x07\x08\x0b\x0c\x0e\x0f' \
          '\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f':
    # these chars will be removed
    clean_input_translation_map[ord(c)] = None
del c

# Other stuff
url_schemas = ['http', 'https', 'ftp', 'file',
               'mailto', 'nntp', 'news',
               'ssh', 'telnet', 'irc', 'ircs', 'xmpp', 'mumble',
               'webcal', 'ed2k', 'apt', 'rootz',
               'gopher',
               'notes',
               'rtp', 'rtsp', 'rtcp',
              ]

smileys = (r"X-( :D <:( :o :( :) B) :)) ;) /!\ <!> (!) :-? :\ >:> |) " +
           r":-( :-) B-) :-)) ;-) |-) (./) {OK} {X} {i} {1} {2} {3} {*} {o}").split()

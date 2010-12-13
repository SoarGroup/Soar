# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Send a raw object from the caching system (and offer utility
    functions to put data into cache, calculate cache key, etc.).

    Sample usage
    ------------
    Assume we have a big picture (bigpic) and we want to efficiently show some
    thumbnail (thumbpic) for it:

    # first calculate a (hard to guess) cache key (this key will change if the
    # original data (bigpic) changes):
    key = cache.key(..., attachname=bigpic, ...)

    # check if we don't have it in cache yet
    if not cache.exists(..., key):
        # if we don't have it in cache, we need to render it - this is an
        # expensive operation that we want to avoid by caching:
        thumbpic = render_thumb(bigpic)
        # put expensive operation's results into cache:
        cache.put(..., key, thumbpic, ...)

    url = cache.url(..., key)
    html = '<img src="%s">' % url

    @copyright: 2008 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import log
logging = log.getLogger(__name__)

# keep both imports below as they are, order is important:
from MoinMoin import wikiutil
import mimetypes

from MoinMoin import config, caching
from MoinMoin.util import filesys
from MoinMoin.action import AttachFile
from MoinMoin.support.python_compatibility import hmac_new

action_name = __name__.split('.')[-1]

# Do NOT get this directly from request.values or user would be able to read any cache!
cache_arena = 'sendcache'  # just using action_name is maybe rather confusing

# We maybe could use page local caching (not 'wiki' global) to have less directory entries.
# Local is easier to automatically cleanup if an item changes. Global is easier to manually cleanup.
# Local makes data_dir much larger, harder to backup.
cache_scope = 'wiki'

do_locking = False

def key(request, wikiname=None, itemname=None, attachname=None, content=None, secret=None):
    """
    Calculate a (hard-to-guess) cache key.

    Important key properties:
    * The key must be hard to guess (this is because do=get does no ACL checks,
      so whoever got the key [e.g. from html rendering of an ACL protected wiki
      page], will be able to see the cached content.
    * The key must change if the (original) content changes. This is because
      ACLs on some item may change and even if somebody was allowed to see some
      revision of some item, it does not implicate that he is allowed to see
      any other revision also. There will be no harm if he can see exactly the
      same content again, but there could be harm if he could access a revision
      with different content.

    If content is supplied, we will calculate and return a hMAC of the content.

    If wikiname, itemname, attachname is given, we don't touch the content (nor do
    we read it ourselves from the attachment file), but we just calculate a key
    from the given metadata values and some metadata we get from the filesystem.

    Hint: if you need multiple cache objects for the same source content (e.g.
          thumbnails of different sizes for the same image), calculate the key
          only once and then add some different prefixes to it to get the final
          cache keys.

    @param request: the request object
    @param wikiname: the name of the wiki (if not given, will be read from cfg)
    @param itemname: the name of the page
    @param attachname: the filename of the attachment
    @param content: content data as unicode object (e.g. for page content or
                    parser section content)
    @param secret: secret for hMAC calculation (default: use secret from cfg)
    """
    if secret is None:
        secret = request.cfg.secrets['action/cache']
    if content:
        hmac_data = content
    elif itemname is not None and attachname is not None:
        wikiname = wikiname or request.cfg.interwikiname or request.cfg.siteid
        fuid = filesys.fuid(AttachFile.getFilename(request, itemname, attachname))
        hmac_data = u''.join([wikiname, itemname, attachname, repr(fuid)])
    else:
        raise AssertionError('cache_key called with unsupported parameters')

    hmac_data = hmac_data.encode('utf-8')
    key = hmac_new(secret, hmac_data).hexdigest()
    return key


def put(request, key, data,
        filename=None,
        content_type=None,
        content_disposition=None,
        content_length=None,
        last_modified=None,
        original=None):
    """
    Put an object into the cache to send it with cache action later.

    @param request: the request object
    @param key: non-guessable key into cache (str)
    @param data: content data (str or open file-like obj)
    @param filename: filename for content-disposition header and for autodetecting
                     content_type (unicode, default: None)
    @param content_type: content-type header value (str, default: autodetect from filename)
    @param content_disposition: type for content-disposition header (str, default: None)
    @param content_length: data length for content-length header (int, default: autodetect)
    @param last_modified: last modified timestamp (int, default: autodetect)
    @param original: location of original object (default: None) - this is just written to
                     the metadata cache "as is" and could be used for cache cleanup,
                     use (wikiname, itemname, attachname or None))
    """
    import os.path
    from MoinMoin.util import timefuncs

    if filename:
        # make sure we just have a simple filename (without path)
        filename = os.path.basename(filename)

        if content_type is None:
            # try autodetect
            mt, enc = mimetypes.guess_type(filename)
            if mt:
                content_type = mt

    if content_type is None:
        content_type = 'application/octet-stream'

    data_cache = caching.CacheEntry(request, cache_arena, key+'.data', cache_scope, do_locking=do_locking)
    data_cache.update(data)
    content_length = content_length or data_cache.size()
    last_modified = last_modified or data_cache.mtime()

    httpdate_last_modified = timefuncs.formathttpdate(int(last_modified))
    headers = [('Content-Type', content_type),
               ('Last-Modified', httpdate_last_modified),
               ('Content-Length', content_length),
              ]
    if content_disposition and filename:
        # TODO: fix the encoding here, plain 8 bit is not allowed according to the RFCs
        # There is no solution that is compatible to IE except stripping non-ascii chars
        filename = filename.encode(config.charset)
        headers.append(('Content-Disposition', '%s; filename="%s"' % (content_disposition, filename)))

    meta_cache = caching.CacheEntry(request, cache_arena, key+'.meta', cache_scope, do_locking=do_locking, use_pickle=True)
    meta_cache.update({
        'httpdate_last_modified': httpdate_last_modified,
        'last_modified': last_modified,
        'headers': headers,
        'original': original,
    })


def exists(request, key, strict=False):
    """
    Check if a cached object for this key exists.

    @param request: the request object
    @param key: non-guessable key into cache (str)
    @param strict: if True, also check the data cache, not only meta (bool, default: False)
    @return: is object cached? (bool)
    """
    if strict:
        data_cache = caching.CacheEntry(request, cache_arena, key+'.data', cache_scope, do_locking=do_locking)
        data_cached = data_cache.exists()
    else:
        data_cached = True  # we assume data will be there if meta is there

    meta_cache = caching.CacheEntry(request, cache_arena, key+'.meta', cache_scope, do_locking=do_locking, use_pickle=True)
    meta_cached = meta_cache.exists()

    return meta_cached and data_cached


def remove(request, key):
    """ delete headers/data cache for key """
    meta_cache = caching.CacheEntry(request, cache_arena, key+'.meta', cache_scope, do_locking=do_locking, use_pickle=True)
    meta_cache.remove()
    data_cache = caching.CacheEntry(request, cache_arena, key+'.data', cache_scope, do_locking=do_locking)
    data_cache.remove()


def url(request, key, do='get'):
    """ return URL for the object cached for key """
    return request.href(action=action_name, do=do, key=key)

def _get_headers(request, key):
    """ get last_modified and headers cached for key """
    meta_cache = caching.CacheEntry(request, cache_arena, key+'.meta', cache_scope, do_locking=do_locking, use_pickle=True)
    meta = meta_cache.content()
    return meta['httpdate_last_modified'], meta['headers']


def _get_datafile(request, key):
    """ get an open data file for the data cached for key """
    data_cache = caching.CacheEntry(request, cache_arena, key+'.data', cache_scope, do_locking=do_locking)
    data_cache.open(mode='r')
    return data_cache


def _do_get(request, key):
    """ send a complete http response with headers/data cached for key """
    try:
        last_modified, headers = _get_headers(request, key)
        if request.if_modified_since == last_modified:
            request.status_code = 304
        else:
            for k, v in headers:
                request.headers.add(k, v)
            data_file = _get_datafile(request, key)
            request.send_file(data_file)
    except caching.CacheError:
        request.status_code = 404


def _do_remove(request, key):
    """ delete headers/data cache for key """
    remove(request, key)


def _do(request, do, key):
    if do == 'get':
        _do_get(request, key)
    elif do == 'remove':
        _do_remove(request, key)

def execute(pagename, request):
    do = request.values.get('do')
    key = request.values.get('key')
    _do(request, do, key)


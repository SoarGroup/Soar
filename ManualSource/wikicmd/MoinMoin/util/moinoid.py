"""
    MoinMoin - OpenID utils

    @copyright: 2006, 2007 Johannes Berg <johannes@sipsolutions.net>
    @license: GNU GPL, see COPYING for details.
"""

from random import randint
import time

from openid import oidutil
from openid.store.interface import OpenIDStore
from openid.association import Association
from openid.store import nonce

from MoinMoin import caching
from MoinMoin.support.python_compatibility import hash_new

from MoinMoin import log
logging = log.getLogger(__name__)

# redirect openid logging to moin log
def log(msg, level=0):
    logging.log(level, msg)

oidutil.log = log

def strbase64(value):
    from base64 import encodestring
    return encodestring(str(value)).replace('\n', '')


def _cleanup_nonces(request):
    cachelist = caching.get_cache_list(request, 'openid-nonce', 'farm')
    # really openid should have a method to check this...
    texpired = time.time() - nonce.SKEW
    for name in cachelist:
        entry = caching.CacheEntry(request, 'openid-nonce', name,
                                   scope='farm', use_pickle=False)
        try:
            timestamp = int(entry.content())
            if timestamp < texpired:
                entry.remove()
        except caching.CacheError:
            pass


class MoinOpenIDStore(OpenIDStore):
    '''OpenIDStore for MoinMoin'''
    def __init__(self, request):
        self.request = request
        OpenIDStore.__init__(self)

    def key(self, url):
        '''return cache key'''
        return hash_new('sha1', url).hexdigest()

    def storeAssociation(self, server_url, association):
        ce = caching.CacheEntry(self.request, 'openid', self.key(server_url),
                                scope='wiki', use_pickle=True)
        if ce.exists():
            assocs = ce.content()
        else:
            assocs = []
        assocs += [association.serialize()]
        ce.update(assocs)

    def getAssociation(self, server_url, handle=None):
        ce = caching.CacheEntry(self.request, 'openid', self.key(server_url),
                                scope='wiki', use_pickle=True)
        if not ce.exists():
            return None
        assocs = ce.content()
        found = False
        for idx in xrange(len(assocs)-1, -1, -1):
            assoc_str = assocs[idx]
            association = Association.deserialize(assoc_str)
            if association.getExpiresIn() == 0:
                del assocs[idx]
            else:
                if handle is None or association.handle == handle:
                    found = True
                    break
        ce.update(assocs)
        if found:
            return association
        return None

    def removeAssociation(self, server_url, handle):
        ce = caching.CacheEntry(self.request, 'openid', self.key(server_url),
                                scope='wiki', use_pickle=True)
        if not ce.exists():
            return
        assocs = ce.content()
        for idx in xrange(len(assocs)-1, -1, -1):
            assoc_str = assocs[idx]
            association = Association.deserialize(assoc_str)
            if association.handle == handle:
                del assocs[idx]
        if len(assocs):
            ce.update(assocs)
        else:
            ce.remove()

    def useNonce(self, server_url, timestamp, salt):
        val = ''.join([str(server_url), str(timestamp), str(salt)])
        csum = hash_new('sha1', val).hexdigest()
        ce = caching.CacheEntry(self.request, 'openid-nonce', csum,
                                scope='farm', use_pickle=False)
        if ce.exists():
            # nonce already used!
            return False
        ce.update(str(timestamp))
        if randint(0, 999) == 0:
            self.request.add_finisher(_cleanup_nonces)
        return True

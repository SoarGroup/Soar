"""
    MoinMoin - Support Package

    Stuff for compatibility with older Python versions

    @copyright: 2007 Heinrich Wendel <heinrich.wendel@gmail.com>,
                2009 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""

min_req_exc = Exception("Minimum requirement for MoinMoin is Python 2.4.")

try:
    import string
    rsplit = string.rsplit # Python >= 2.4 needed
except AttributeError:
    raise min_req_exc

try:
    sorted = sorted # Python >= 2.4 needed
except NameError:
    raise min_req_exc

try:
    set = set # Python >= 2.4 needed
    frozenset = frozenset
except NameError:
    raise min_req_exc


try:
    from functools import partial # Python >= 2.5 needed
except (NameError, ImportError):
    class partial(object):
        def __init__(*args, **kw):
            self = args[0]
            self.fn, self.args, self.kw = (args[1], args[2:], kw)

        def __call__(self, *args, **kw):
            if kw and self.kw:
                d = self.kw.copy()
                d.update(kw)
            else:
                d = kw or self.kw
            return self.fn(*(self.args + args), **d)


try:
    import hashlib, hmac # Python >= 2.5 needed
    hash_new = hashlib.new
    def hmac_new(key, msg, digestmod=hashlib.sha1):
        return hmac.new(key, msg, digestmod)

except (NameError, ImportError):
    import sha
    def hash_new(name, string=''):
        if name in ('SHA1', 'sha1'):
            return sha.new(string)
        elif name in ('MD5', 'md5'):
            import md5
            return md5.new(string)
        raise ValueError("unsupported hash type")

    def hmac_new(key, msg, digestmod=sha):
        import hmac
        return hmac.new(key, msg, digestmod)


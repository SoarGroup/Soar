# taken from Amos' XML-RPC HowTo:

import xmlrpclib, httplib
from base64 import encodestring

class BasicAuthTransport(xmlrpclib.Transport):
    def __init__(self, username=None, password=None):
        self.username = username
        self.password = password
        self.verbose = 0

    def request(self, host, handler, request_body, **kw):
        # issue XML-RPC request
        h = httplib.HTTP(host)
        h.putrequest("POST", handler)

        # required by HTTP/1.1
        h.putheader("Host", host)

        # required by XML-RPC
        h.putheader("User-Agent", self.user_agent)
        h.putheader("Content-Type", "text/xml")
        h.putheader("Content-Length", str(len(request_body)))

        # basic auth
        if self.username is not None and self.password is not None:
            authhdr = "Basic %s" % encodestring("%s:%s" % (self.username, self.password)).replace("\012", "")
            h.putheader("Authorization", authhdr)
        h.endheaders()

        if request_body:
            h.send(request_body)

        errcode, errmsg, headers = h.getreply()

        if errcode != 200:
            raise xmlrpclib.ProtocolError(
                host + handler,
                errcode, errmsg,
                headers
                )

        return self.parse_response(h.getfile())


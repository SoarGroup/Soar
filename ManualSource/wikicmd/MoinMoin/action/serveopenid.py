# -*- coding: utf-8 -*-
"""
    MoinMoin - OpenID server action

    This is the UI and provider for OpenID.

    @copyright: 2006, 2007, 2008 Johannes Berg <johannes@sipsolutions.net>
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin.support.python_compatibility import rsplit
from MoinMoin.util.moinoid import MoinOpenIDStore, strbase64
from MoinMoin import wikiutil
from openid.consumer.discover import OPENID_1_0_TYPE, \
    OPENID_1_1_TYPE, OPENID_2_0_TYPE, OPENID_IDP_2_0_TYPE
from openid import sreg
from openid.cryptutil import randomString
from openid.server import server
from openid.message import IDENTIFIER_SELECT
from MoinMoin.widget import html
from MoinMoin.web.request import MoinMoinFinish

def execute(pagename, request):
    return MoinOpenIDServer(pagename, request).handle()

class MoinOpenIDServer:
    def __init__(self, pagename, request):
        self.request = request
        self._ = request.getText
        self.cfg = request.cfg

    def serveYadisEP(self, endpoint_url):
        request = self.request
        request.content_type = 'application/xrds+xml'

        user_url = request.getQualifiedURL(request.page.url(request))
        self.request.write("""\
<?xml version="1.0" encoding="UTF-8"?>
<xrds:XRDS
    xmlns:xrds="xri://$xrds"
    xmlns="xri://$xrd*($v*2.0)">
  <XRD>

    <Service priority="0">
      <Type>%(type10)s</Type>
      <URI>%(uri)s</URI>
      <LocalID>%(id)s</LocalID>
    </Service>

    <Service priority="0">
      <Type>%(type11)s</Type>
      <URI>%(uri)s</URI>
      <LocalID>%(id)s</LocalID>
    </Service>

    <!-- older version of the spec draft -->
    <Service priority="0">
      <Type>http://openid.net/signon/2.0</Type>
      <URI>%(uri)s</URI>
      <LocalID>%(id)s</LocalID>
    </Service>

    <Service priority="0">
      <Type>%(type20)s</Type>
      <URI>%(uri)s</URI>
      <LocalID>%(id)s</LocalID>
    </Service>

  </XRD>
</xrds:XRDS>
""" % {
    'type10': OPENID_1_0_TYPE,
    'type11': OPENID_1_1_TYPE,
    'type20': OPENID_2_0_TYPE,
    'uri': endpoint_url,
    'id': user_url
})

    def serveYadisIDP(self, endpoint_url):
        request = self.request
        request.content_type = 'application/xrds+xml'

        user_url = request.getQualifiedURL(request.page.url(request))
        self.request.write("""\
<?xml version="1.0" encoding="UTF-8"?>
<xrds:XRDS
    xmlns:xrds="xri://$xrds"
    xmlns="xri://$xrd*($v*2.0)">
  <XRD>

    <Service priority="0">
      <Type>%(typeidp)s</Type>
      <URI>%(uri)s</URI>
      <LocalID>%(id)s</LocalID>
    </Service>

  </XRD>
</xrds:XRDS>
""" % {
    'typeidp': OPENID_IDP_2_0_TYPE,
    'uri': endpoint_url,
    'id': user_url
})

    def _verify_endpoint_identity(self, identity):
        """
           Verify that the given identity matches the current endpoint.

           We always serve out /UserName?action=... for the UserName
           OpenID and this is pure paranoia to make sure it is that way
           on incoming data.

           Also verify that the given identity is allowed to have an OpenID.
        """
        request = self.request
        cfg = request.cfg

        # we can very well split on the last slash since usernames
        # must not contain slashes
        base, received_name = rsplit(identity, '/', 1)
        check_name = received_name

        if received_name == '':
            pg = wikiutil.getFrontPage(request)
            if pg:
                received_name = pg.page_name
                check_name = received_name
                if 'openid.user' in pg.pi:
                    received_name = pg.pi['openid.user']

        # some sanity checking
        # even if someone goes to http://johannes.sipsolutions.net/
        # we'll serve out http://johannes.sipsolutions.net/JohannesBerg?action=serveopenid
        # (if JohannesBerg is set as page_front_page)
        # For the #OpenIDUser PI, we need to allow the page that includes the PI,
        # hence use check_name here (see above for how it is assigned)
        fullidentity = '/'.join([base, check_name])
        thisurl = request.getQualifiedURL(request.page.url(request))
        if not thisurl == fullidentity:
            return False

        # again, we never put an openid.server link on this page...
        # why are they here?
        openid_group_name = cfg.openid_server_restricted_users_group
        if openid_group_name and received_name not in request.groups.get(openid_group_name, []):
            return False

        return True

    def handleCheckIDRequest(self, identity, username, openidreq, server_url):
        if self.user_trusts_url(openidreq.trust_root):
            return self.approved(identity, openidreq, server_url=server_url)

        if openidreq.immediate:
            return openidreq.answer(False, identity=identity, server_url=server_url)

        self.request.session['openidserver.request'] = openidreq
        self.show_decide_page(identity, username, openidreq)
        return None

    def _make_identity(self):
        page = wikiutil.getHomePage(self.request)
        if page:
            server_url = self.request.getQualifiedURL(
                             page.url(self.request, querystr={'action': 'serveopenid'}))
            identity = self.request.getQualifiedURL(page.url(self.request))
            return identity, server_url
        return None, None

    def handle(self):
        _ = self._
        request = self.request
        form = request.values

        username = request.page.page_name
        if 'openid.user' in request.page.pi:
            username = request.page.pi['openid.user']


        if not request.cfg.openid_server_enabled:
            # since we didn't put any openid.server into
            # the page to start with, this is someone trying
            # to abuse us. No need to give a nice error
            request.makeForbidden(403, '')
            return

        server_url = request.getQualifiedURL(
                         request.page.url(request, querystr={'action': 'serveopenid'}))

        yadis_type = form.get('yadis')
        if yadis_type == 'ep':
            return self.serveYadisEP(server_url)
        elif yadis_type == 'idp':
            return self.serveYadisIDP(server_url)

        # if the identity is set it must match the server URL
        # sort of arbitrary, but we have to have some restriction
        identity = form.get('openid.identity')
        if identity == IDENTIFIER_SELECT:
            identity, server_url = self._make_identity()
            if not identity:
                return self._sorry_no_identity()
            username = request.user.name
        elif identity is not None:
            if not self._verify_endpoint_identity(identity):
                request.makeForbidden(403, 'verification failed')
                return

        if 'openid.user' in request.page.pi:
            username = request.page.pi['openid.user']

        store = MoinOpenIDStore(request)
        openidsrv = server.Server(store, op_endpoint=server_url)

        answer = None
        if 'dontapprove' in form:
            answer = self.handle_response(False, username, identity)
            if answer is None:
                return
        elif form.has_key('approve'):
            answer = self.handle_response(True, username, identity)
            if answer is None:
                return
        else:
            query = {}
            for key in form:
                query[key] = form[key]
            try:
                openidreq = openidsrv.decodeRequest(query)
            except Exception, e:
                request.makeForbidden(403, 'OpenID decode error: %r' % e)
                return

            if openidreq is None:
                request.makeForbidden(403, 'no request')
                return

            if request.user.valid and username != request.user.name:
                answer = openidreq.answer(False, identity=identity, server_url=server_url)
            elif openidreq.mode in ["checkid_immediate", "checkid_setup"]:
                answer = self.handleCheckIDRequest(identity, username, openidreq, server_url)
                if answer is None:
                    return
            else:
                answer = openidsrv.handleRequest(openidreq)
        webanswer = openidsrv.encodeResponse(answer)
        request.status = '%d OpenID status' % webanswer.code
        for hdr in webanswer.headers:
            request.headers.add(hdr, webanswer.headers[hdr])
        request.write(webanswer.body)
        raise MoinMoinFinish

    def handle_response(self, positive, username, identity):
        request = self.request
        form = request.values

        # check form submission nonce, use None for stored value default
        # since it cannot be sent from the user
        session_nonce = self.request.session.get('openidserver.nonce')
        if session_nonce is not None:
            del self.request.session['openidserver.nonce']
        # use empty string if nothing was sent
        form_nonce = form.get('nonce', '')
        if session_nonce != form_nonce:
            self.request.makeForbidden(403, 'invalid nonce')
            return None

        openidreq = request.session.get('openidserver.request')
        if not openidreq:
            request.makeForbidden(403, 'no response request')
            return None
        del request.session['openidserver.request']

        if (not positive or
            not request.user.valid or
            request.user.name != username):
            return openidreq.answer(False)


        if form.get('remember', 'no') == 'yes':
            if not hasattr(request.user, 'openid_trusted_roots'):
                request.user.openid_trusted_roots = []
            request.user.openid_trusted_roots.append(strbase64(openidreq.trust_root))
            request.user.save()
        dummyidentity, server_url = self._make_identity()
        return self.approved(identity, openidreq, server_url=server_url)

    def approved(self, identity, openidreq, server_url=None):
        # TODO: If simple registration is implemented, this needs
        #       to do something like the following:
        #
        #       sreg_data = { fill this dict with real values }
        #       sreq_req = sreg.SRegRequest.fromOpenIDRequest(openidreq.message)
        #       # do something with the request to see what values are required?
        #       sreg_resp = sreg.SRegResponse.extractResponse(openidreq, sreg_data)
        #       sreg_resp.addToOpenIDResponse(reply.fields)

        reply = openidreq.answer(True, identity=identity, server_url=server_url)
        return reply

    def user_trusts_url(self, trustroot):
        user = self.request.user
        if hasattr(user, 'openid_trusted_roots'):
            return strbase64(trustroot) in user.openid_trusted_roots
        return False

    def show_decide_page(self, identity, username, openidreq):
        request = self.request
        _ = self._

        if not request.user.valid or username != request.user.name:
            request.makeForbidden(403, _('''You need to manually go to your OpenID provider wiki
and log in before you can use your OpenID. MoinMoin will
never allow you to enter your password here.

Once you have logged in, simply reload this page.'''))
            return

        request.theme.send_title(_("OpenID Trust verification"), pagename=request.page.page_name)
        # Start content (important for RTL support)
        request.write(request.formatter.startContent("content"))

        request.write(request.formatter.paragraph(1))
        request.write(_('The site %s has asked for your identity.') % openidreq.trust_root)
        request.write(request.formatter.paragraph(0))
        request.write(request.formatter.paragraph(1))
        request.write(_('''
If you approve, the site represented by the trust root below will be
told that you control the identity URL %s. (If you are using a delegated
identity, the site will take care of reversing the
delegation on its own.)''') % openidreq.identity)
        request.write(request.formatter.paragraph(0))

        form = html.FORM(method='POST', action=request.page.url(request))
        form.append(html.INPUT(type='hidden', name='action', value='serveopenid'))
        form.append(html.INPUT(type='hidden', name='openid.identity', value=openidreq.identity))
        form.append(html.INPUT(type='hidden', name='openid.return_to', value=openidreq.return_to))
        form.append(html.INPUT(type='hidden', name='openid.trust_root', value=openidreq.trust_root))
        form.append(html.INPUT(type='hidden', name='openid.mode', value=openidreq.mode))
        form.append(html.INPUT(type='hidden', name='name', value=username))

        nonce = randomString(32, 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789')
        form.append(html.INPUT(type='hidden', name='nonce', value=nonce))
        request.session['openidserver.nonce'] = nonce

        table = html.TABLE()
        form.append(table)

        tr = html.TR()
        table.append(tr)
        tr.append(html.TD().append(html.STRONG().append(html.Text(_('Trust root')))))
        tr.append(html.TD().append(html.Text(openidreq.trust_root)))

        tr = html.TR()
        table.append(tr)
        tr.append(html.TD().append(html.STRONG().append(html.Text(_('Identity URL')))))
        tr.append(html.TD().append(html.Text(identity)))

        tr = html.TR()
        table.append(tr)
        tr.append(html.TD().append(html.STRONG().append(html.Text(_('Name')))))
        tr.append(html.TD().append(html.Text(username)))

        tr = html.TR()
        table.append(tr)
        tr.append(html.TD().append(html.STRONG().append(html.Text(_('Remember decision')))))
        td = html.TD()
        tr.append(td)
        td.append(html.INPUT(type='checkbox', name='remember', value='yes'))
        td.append(html.Text(_('Remember this trust decision and don\'t ask again')))

        tr = html.TR()
        table.append(tr)
        tr.append(html.TD())
        td = html.TD()
        tr.append(td)

        td.append(html.INPUT(type='submit', name='approve', value=_("Approve")))
        td.append(html.INPUT(type='submit', name='dontapprove', value=_("Don't approve")))

        request.write(unicode(form))

        request.write(request.formatter.endContent())
        request.theme.send_footer(request.page.page_name)
        request.theme.send_closing_html()

    def _sorry_no_identity(self):
        request = self.request
        _ = self._

        request.theme.send_title(_("OpenID not served"), pagename=request.page.page_name)
        # Start content (important for RTL support)
        request.write(request.formatter.startContent("content"))

        request.write(request.formatter.paragraph(1))
        request.write(_('''
Unfortunately you have not created your homepage yet. Therefore,
we cannot serve an OpenID for you. Please create your homepage first
and then reload this page or click the button below to cancel this
verification.'''))
        request.write(request.formatter.paragraph(0))

        form = html.FORM(method='POST', action=request.page.url(request))
        form.append(html.INPUT(type='hidden', name='action', value='serveopenid'))

        nonce = randomString(32, 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789')
        form.append(html.INPUT(type='hidden', name='nonce', value=nonce))
        request.session['openidserver.nonce'] = nonce

        form.append(html.INPUT(type='submit', name='dontapprove', value=_("Cancel")))

        request.write(unicode(form))

        request.write(request.formatter.endContent())
        request.theme.send_footer(request.page.page_name)
        request.theme.send_closing_html()

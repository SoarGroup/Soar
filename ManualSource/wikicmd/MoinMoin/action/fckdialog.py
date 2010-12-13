# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - feed some FCKeditor dialogues

    @copyright: 2005-2006 Bastian Blank, Florian Festi, Thomas Waldmann
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import config, wikiutil
from MoinMoin.action.AttachFile import _get_files
from MoinMoin.Page import Page
import re

##############################################################################
### Macro dialog
##############################################################################

def macro_dialog(request):
    help = get_macro_help(request)
    request.write(
        '''<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
 <head>
  <title>Insert Macro</title>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
  <meta content="noindex,nofollow" name="robots">
  <script src="%s/applets/FCKeditor/editor/dialog/common/fck_dialog_common.js" type="text/javascript"></script>
  <script language="javascript">

var oEditor = window.parent.InnerDialogLoaded() ;
var FCKLang = oEditor.FCKLang ;
var FCKMacros = oEditor.FCKMacros ;

window.onload = function ()
{
 // First of all, translate the dialog box texts
 oEditor.FCKLanguageManager.TranslatePage( document ) ;

 OnChange( "BR" );

 // Show the "Ok" button.
 window.parent.SetOkButton( true ) ;
}

function Ok()
{
 if ( document.getElementById('txtName').value.length == 0 )
 {
  alert( FCKLang.MacroErrNoName ) ;
  return false ;
 }

 FCKMacros.Add( txtName.value ) ;
 return true ;
}

function OnChange( sMacro )
{
  // sMacro = GetE("txtName").value;
  oHelp = GetE("help");
  for (var i=0; i<oHelp.childNodes.length; i++)
  {
    var oDiv = oHelp.childNodes[i];
    if (oDiv.nodeType==1)
    {
      // oDiv.style.display = (GetAttribute(oDiv, "id", "")==sMacro) ? '' : 'none';
      if (GetAttribute(oDiv, "id", "") == sMacro)
      {
          oDiv.style.display = '' ;
          // alert("enabled div id " + sMacro) ;
      }
      else
      {
          oDiv.style.display = 'none' ;
      }
    }
  }
}

  </script>
 </head>
 <body scroll="no" style="OVERFLOW: hidden">
  <table height="100%%" cellSpacing="0" cellPadding="0" width="100%%" border="0">
   <tr>
    <td>
     <table cellSpacing="0" cellPadding="0" align="center" border="0">
      <tr>
       <td valign="top">
       <span fckLang="MacroDlgName">Macro Name</span><br>
       <select id="txtName" size="10" onchange="OnChange(this.value);">
''' % request.cfg.url_prefix_static)

    macros = []
    for macro in macro_list(request):
        if macro == "BR":
            selected = ' selected="selected"'
        else:
            selected = ''
        if macro in help:
            macros.append('<option value="%s"%s>%s</option>' %
                          (help[macro].group('prototype'), selected, macro))
        else:
            macros.append('<option value="%s"%s>%s</option>' %
                          (macro, selected, macro))

    request.write('\n'.join(macros))
    request.write('''
       </select>
     </td>
     <td id="help">''')

    helptexts = []
    for macro in macro_list(request):
        if macro in help:
            match = help[macro]
            prototype = match.group('prototype')
            helptext = match.group('help')
        else:
            prototype = macro
            helptext = ""
        helptexts.append(
            '''<div id="%s" style="DISPLAY: none">
               <b>&lt;&lt;%s&gt;&gt;</b>
               <br/>
               <textarea style="color:#000000" cols="37" rows="10" disabled="disabled">%s</textarea>
               </div>'''
            % (prototype, prototype, helptext))

    request.write(''.join(helptexts))
    request.write('''
     </td>
    </tr>
   </table>
  </td>
 </tr>
</table>
</body>
</html>
''')

def macro_list(request):
    from MoinMoin import macro
    macros = macro.getNames(request.cfg)
    macros.sort()
    return macros

def get_macro_help(request):
    """ Read help texts from SystemPage('HelpOnMacros')"""
    helppage = wikiutil.getLocalizedPage(request, "HelpOnMacros")
    content = helppage.get_raw_body()
    macro_re = re.compile(
        r"\|\|(<.*?>)?\{\{\{" +
        r"<<(?P<prototype>(?P<macro>\w*).*)>>" +
        r"\}\}\}\s*\|\|" +
        r"[^|]*\|\|[^|]*\|\|<[^>]*>" +
        r"\s*(?P<help>.*?)\s*\|\|\s*(?P<example>.*?)\s*(<<[^>]*>>)*\s*\|\|$", re.U|re.M)
    help = {}
    for match in macro_re.finditer(content):
        help[match.group('macro')] = match
    return help

##############################################################################
### Link dialog
##############################################################################

def page_list(request):
    from MoinMoin import search
    name = request.values.get("pagename", "")
    if name:
        searchresult = search.searchPages(request, 't:"%s"' % name)
        pages = [p.page_name for p in searchresult.hits]
    else:
        pages = [name]
    request.write(
        '''<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
 <head>
  <title>Insert Page Link</title>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
  <meta content="noindex,nofollow" name="robots">
 </head>
 <body scroll="no" style="OVERFLOW: hidden">
  <table height="100%%" cellSpacing="0" cellPadding="0" width="100%%" border="0">
   <tr>
    <td>
     <table cellSpacing="0" cellPadding="0" align="center" border="0">
      <tr>
       <td>
       <span fckLang="PageDlgName">Page name</span><br>
       <select id="txtName" size="1">
       %s
       </select>
     </td>
    </tr>
   </table>
  </td>
 </tr>
</table>
</body>
</html>
''' % "".join(["<option>%s</option>\n" % p for p in pages]))

def link_dialog(request):
    # list of wiki pages
    name = request.values.get("pagename", "")
    if name:
        from MoinMoin import search
        # XXX error handling!
        searchresult = search.searchPages(request, 't:"%s"' % name)

        pages = [p.page_name for p in searchresult.hits]
        pages.sort()
        pages[0:0] = [name]
        page_list = '''
         <tr>
          <td colspan=2>
           <select id="sctPagename" size="1" onchange="OnChangePagename(this.value);">
           %s
           </select>
          <td>
         </tr>
''' % "\n".join(['<option value="%s">%s</option>' % (page, page)
                 for page in pages])
    else:
        page_list = ""

    # list of interwiki names
    interwiki_list = wikiutil.load_wikimap(request)
    interwiki = interwiki_list.keys()
    interwiki.sort()
    iwpreferred = request.cfg.interwiki_preferred[:]
    if not iwpreferred or iwpreferred and iwpreferred[-1] is not None:
        resultlist = iwpreferred
        for iw in interwiki:
            if not iw in iwpreferred:
                resultlist.append(iw)
    else:
        resultlist = iwpreferred[:-1]
    interwiki = "\n".join(
        ['<option value="%s">%s</option>' % (key, key) for key in resultlist])

    # wiki url
    url_prefix_static = request.cfg.url_prefix_static
    scriptname = request.script_root + '/'
    action = scriptname
    basepage = request.page.page_name
    request.write(u'''
<!--
 * FCKeditor - The text editor for internet
 * Copyright (C) 2003-2004 Frederico Caldeira Knabben
 *
 * Licensed under the terms of the GNU Lesser General Public License:
 *   http://www.opensource.org/licenses/lgpl-license.php
 *
 * For further information visit:
 *   http://www.fckeditor.net/
 *
 * File Name: fck_link.html
 *  Link dialog window.
 *
 * Version:  2.0 FC (Preview)
 * Modified: 2005-02-18 23:55:22
 *
 * File Authors:
 *   Frederico Caldeira Knabben (fredck@fckeditor.net)
-->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<meta http-equiv="Content-Type" content="text/html;charset=utf-8">
<meta name="robots" content="index,nofollow">
<html>
 <head>
  <title>Link Properties</title>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
  <meta name="robots" content="noindex,nofollow" />
  <script src="%(url_prefix_static)s/applets/FCKeditor/editor/dialog/common/fck_dialog_common.js" type="text/javascript"></script>
  <script src="%(url_prefix_static)s/applets/moinFCKplugins/moinlink/fck_link.js" type="text/javascript"></script>
  <script src="%(url_prefix_static)s/applets/moinFCKplugins/moinurllib.js" type="text/javascript"></script>
 </head>
 <body scroll="no" style="OVERFLOW: hidden">
  <div id="divInfo" style="DISPLAY: none">
   <span fckLang="DlgLnkType">Link Type</span><br />
   <select id="cmbLinkType" onchange="SetLinkType(this.value);">
    <option value="wiki" selected="selected">WikiPage</option>
    <option value="interwiki">Interwiki</option>
    <option value="url" fckLang="DlgLnkTypeURL">URL</option>
   </select>
   <br />
   <br />
   <div id="divLinkTypeWiki">
    <table height="100%%" cellSpacing="0" cellPadding="0" width="100%%" border="0">
     <tr>
      <td>
       <form action=%(action)s method="GET">
       <input type="hidden" name="action" value="fckdialog">
       <input type="hidden" name="dialog" value="link">
       <input type="hidden" id="basepage" name="basepage" value="%(basepage)s">
       <table cellSpacing="0" cellPadding="0" align="center" border="0">
        <tr>
         <td>
          <span fckLang="PageDlgName">Page Name</span><br>
          <input id="txtPagename" name="pagename" size="30" value="%(name)s">
         </td>
         <td valign="bottom">
           <input id=btnSearchpage type="submit" value="Search">
         </td>
        </tr>
        %(page_list)s
       </table>
       </form>
      </td>
     </tr>
    </table>
   </div>
   <div id="divLinkTypeInterwiki">
    <table height="100%%" cellSpacing="0" cellPadding="0" width="100%%" border="0">
     <tr>
      <td>
       <table cellSpacing="0" cellPadding="0" align="center" border="0">
        <tr>
         <td>
          <span fckLang="WikiDlgName">Wiki:PageName</span><br>
          <select id="sctInterwiki" size="1">
          %(interwiki)s
          </select>:
          <input id="txtInterwikipagename"></input>
         </td>
        </tr>
       </table>
      </td>
     </tr>
    </table>
   </div>
   <div id="divLinkTypeUrl">
    <table cellspacing="0" cellpadding="0" width="100%%" border="0">
     <tr>
      <td nowrap="nowrap">
       <span fckLang="DlgLnkProto">Protocol</span><br />
       <select id="cmbLinkProtocol">
        <option value="http://" selected="selected">http://</option>
        <option value="https://">https://</option>
        <option value="ftp://">ftp://</option>
        <option value="file://">file://</option>
        <option value="news://">news://</option>
        <option value="mailto:">mailto:</option>
        <option value="" fckLang="DlgLnkProtoOther">&lt;other&gt;</option>
       </select>
      </td>
      <td nowrap="nowrap">&nbsp;</td>
      <td nowrap="nowrap" width="100%%">
       <span fckLang="DlgLnkURL">URL</span><br />
       <input id="txtUrl" style="WIDTH: 100%%" type="text" onkeyup="OnUrlChange();" onchange="OnUrlChange();" />
      </td>
     </tr>
    </table>
    <br />
   </div>
  </div>
 </body>
</html>
''' % locals())


def attachment_dialog(request):
    """ Attachment dialog for GUI editor. """
    """ Features: This dialog can... """
    """ - list attachments in a drop down list """
    """ - list attachments also for a different page than the current one """
    """ - create new attachment """
    _ = request.getText
    url_prefix_static = request.cfg.url_prefix_static

    # wiki url
    action = request.script_root + "/"

    # The following code lines implement the feature "list attachments for a different page".
    # Meaning of the variables:
    # - requestedPagename : Name of the page where attachments shall be listed from.
    # - attachmentsPagename : Name of the page where the attachments where retrieved from.
    # - destinationPagename : Name of the page where attachment will be placed on.

    requestedPagename = wikiutil.escape(request.values.get("requestedPagename", ""), quote=True)
    destinationPagename = wikiutil.escape(request.values.get("destinationPagename", request.page.page_name), quote=True)

    attachmentsPagename = requestedPagename or request.page.page_name
    attachments = _get_files(request, attachmentsPagename)
    attachments.sort()
    attachmentList = '''
        <select id="sctAttachments" size="10" style="width:100%%;visibility:hidden;" onchange="OnAttachmentListChange();">
        %s
        </select>
''' % "\n".join(['<option value="%s">%s</option>' % (wikiutil.escape(attachment, quote=True), wikiutil.escape(attachment, quote=True))
                 for attachment in attachments])

    # Translation of dialog texts.
    langAttachmentLocation = _("Attachment location")
    langPagename = _("Page name")
    langAttachmentname = _("Attachment name")
    langListAttachmentsButton = _("Refresh attachment list")
    langAttachmentList = _("List of attachments")

    if len(attachmentsPagename) > 50:
        shortenedPagename = "%s ... %s" % (attachmentsPagename[0:25], attachmentsPagename[-25:])
    else:
        shortenedPagename = attachmentsPagename
    langAvailableAttachments = "%s: %s" % (_("Available attachments for page"), shortenedPagename)

    request.write('''
<!--
 * FCKeditor - The text editor for internet
 * Copyright (C) 2003-2004 Frederico Caldeira Knabben
 *
 * Licensed under the terms of the GNU Lesser General Public License:
 *   http://www.opensource.org/licenses/lgpl-license.php
 *
 * For further information visit:
 *   http://www.fckeditor.net/
 *
 * File Name: fck_attachment.html
 *  Attachment dialog window.
 *
 * Version:  2.0 FC (Preview)
 * Modified: 2005-02-18 23:55:22
 *
 * File Authors:
 *   Frederico Caldeira Knabben (fredck@fckeditor.net)
-->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<meta http-equiv="Content-Type" content="text/html;charset=utf-8">
<meta name="robots" content="index,nofollow">
<html>
 <head>
  <title>Attachment Properties</title>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
  <meta name="robots" content="noindex,nofollow" />
  <script src="%(url_prefix_static)s/applets/FCKeditor/editor/dialog/common/fck_dialog_common.js" type="text/javascript"></script>
  <script src="%(url_prefix_static)s/applets/moinFCKplugins/moinattachment/fck_attachment.js" type="text/javascript"></script>
  <script src="%(url_prefix_static)s/applets/moinFCKplugins/moinurllib.js" type="text/javascript"></script>
 </head>
 <body scroll="no" style="OVERFLOW: hidden">
    <form id="DlgAttachmentForm" name="DlgAttachmentForm" action=%(action)s method="GET">
    <input type="hidden" name="action" value="fckdialog">
    <input type="hidden" name="dialog" value="attachment">
    <input type="hidden" id="requestedPagename" name="requestedPagename" value="%(requestedPagename)s">
    <input type="hidden" id="attachmentsPagename" name="attachmentsPagename" value="%(attachmentsPagename)s">
    <input type="hidden" id="destinationPagename" name="destinationPagename" value="%(destinationPagename)s">

    <div id="divInfo" style="valign=top;">
    <div id="divLinkTypeAttachment">
    <fieldset>
    <legend>%(langAttachmentLocation)s</legend>
    <table cellSpacing="0" cellPadding="0" width="100%%" border="0">
        <tr>
            <td valign="bottom" style="width:90%%" style="padding-bottom:10px">
                <span>%(langPagename)s</span><br>
            </td>
        </tr>
        <tr>
            <td valign="bottom" style="width:100%%" style="padding-bottom:10px;padding-right:10px;">
                <input id="txtPagename" type="text" onkeyup="OnPagenameChange();" onchange="OnPagenameChange();" style="width:98%%">
            </td>
        </tr>
        <tr>
            <td valign="bottom" style="width:90%%" style="padding-bottom:10px;">
                <span>%(langAttachmentname)s</span><br>
            </td>
        </tr>
        <tr valign="bottom">
            <td valign="bottom" style="width:100%%" style="padding-bottom:10px;padding-right:10px;">
                <input id="txtAttachmentname" type="text" onkeyup="OnAttachmentnameChange();" onchange="OnPagenameChange();" style="width:98%%"><br>
            </td>
        </tr>
    </table>
    </fieldset>
    <fieldset>
    <legend>%(langAvailableAttachments)s</legend>
    <table cellSpacing="0" cellPadding="0" width="100%%" border="0">
        <tr>
            <td valign="bottom" style="width:100%%" style="padding-bottom:10px">
                <input id="btnListAttachments" type="submit" value="%(langListAttachmentsButton)s">
            </td>
        </tr>
        <tr>
            <td valign="top" style="padding-top:10px">
                <label for="sctAttachments">%(langAttachmentList)s</label><br>
                %(attachmentList)s
            </td>
        </tr>
    </table>
    </fieldset>
   </div>
  </div>
   </form>
 </body>
</html>
''' % locals())


##############################################################################
### Image dialog
##############################################################################

def image_dialog(request):
    url_prefix_static = request.cfg.url_prefix_static
    request.write('''
<!--
 * FCKeditor - The text editor for internet
 * Copyright (C) 2003-2004 Frederico Caldeira Knabben
 *
 * Licensed under the terms of the GNU Lesser General Public License:
 *   http://www.opensource.org/licenses/lgpl-license.php
 *
 * For further information visit:
 *   http://www.fckeditor.net/
 *
 * File Authors:
 *   Frederico Caldeira Knabben (fredck@fckeditor.net)
 *   Florian Festi
-->
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
 <head>
  <title>Link Properties</title>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
  <meta name="robots" content="noindex,nofollow" />
  <script src="%(url_prefix_static)s/applets/FCKeditor/editor/dialog/common/fck_dialog_common.js" type="text/javascript"></script>
  <script src="%(url_prefix_static)s/applets/moinFCKplugins/moinimage/fck_image.js" type="text/javascript"></script>
  <script src="%(url_prefix_static)s/applets/moinFCKplugins/moinurllib.js" type="text/javascript"></script>
 </head>
 <body scroll="no" style="OVERFLOW: hidden">
    <table cellspacing="0" cellpadding="0" width="100%%" border="0">
     <tr>
      <td nowrap="nowrap">
       <span fckLang="DlgLnkProto">Protocol</span><br />
       <select id="cmbLinkProtocol" onchange="OnProtocolChange();">
        <option value="attachment:" selected="selected">attachment:</option>
        <option value="http://">http://</option>
        <option value="https://">https://</option>
        <!-- crashes often: <option value="drawing:">drawing:</option> -->
        <option value="" fckLang="DlgLnkProtoOther">&lt;other&gt;</option>
       </select>
      </td>
      <td nowrap="nowrap">&nbsp;</td>
      <td nowrap="nowrap" width="100%%">
       <span fckLang="DlgLnkURL">URL or File Name (attachment:)</span><br />
       <input id="txtUrl" style="WIDTH: 100%%" type="text" onkeyup="OnUrlChange();" onchange="OnUrlChange();" />
      </td>
     </tr>
     <tr>
      <td colspan=2>
       <div id="divChkLink">
        <input id="chkLink" type="checkbox"> Link to
       </div>
      </td>
    </table>
 </body>
</html>
''' % locals())


#############################################################################
### Main
#############################################################################

def execute(pagename, request):
    dialog = request.values.get("dialog", "")

    if dialog == "macro":
        macro_dialog(request)
    elif dialog == "macrolist":
        macro_list(request)
    elif dialog == "pagelist":
        page_list(request)
    elif dialog == "link":
        link_dialog(request)
    elif dialog == "attachment":
        attachment_dialog(request)
    elif dialog == 'image':
        image_dialog(request)
    else:
        from MoinMoin.Page import Page
        request.theme.add_msg("Dialog unknown!", "error")
        Page(request, pagename).send_page()


# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Plain Text Parser, fallback for text/*

    @copyright: 2000-2002 Juergen Hermann <jh@web.de>
    @license: GNU GPL, see COPYING for details.
"""

Dependencies = []

class Parser:
    """
        Send plain text in a HTML <pre> element.
    """

    ## specify extensions willing to handle
    ## should be a list of extensions including the leading dot
    ## TODO: remove the leading dot from the extension. This is stupid.
    #extensions = ['.txt']
    ## use '*' instead of the list(!) to specify a default parser
    ## which is used as fallback
    extensions = '*'
    Dependencies = []

    def __init__(self, raw, request, **kw):
        self.raw = raw
        self.request = request
        self.form = request.form
        self._ = request.getText
        self.start_line = kw.get('start_line', 0)

    def format(self, formatter):
        """ Send the text. """

        self.lines = self.raw.expandtabs().split('\n')
        if self.lines[-1] == '':
            del self.lines[-1]

        self.lineno = self.start_line

        self.request.write(formatter.preformatted(1))

        for lineno in range(1, self.start_line + 1):
            self.request.write(formatter.line_anchordef(lineno))

        for line in self.lines:
            if self.lineno != self.start_line:
                self.request.write(formatter.text('\n'))

            self.lineno += 1
            self.request.write(formatter.line_anchordef(self.lineno))
            self.request.write(formatter.text(line))

        self.request.write(formatter.preformatted(0))

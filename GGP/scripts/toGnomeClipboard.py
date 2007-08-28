#!/usr/bin/env python

import sys

import pygtk
pygtk.require('2.0')
import gtk

buffer = []
for line in sys.stdin:
        buffer.append(line)

        c = gtk.Clipboard()
        c.set_text(''.join(buffer))
        c.store()


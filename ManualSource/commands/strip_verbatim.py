#!/usr/bin/python

# Since sections in <pre> </pre> tags gets translated to the verbatim
# environment in tex, they better not have any other formatting associated with
# them. Unfortunately the entries in the wiki have a lot of cases where <i>
# </i> tags are embedded in <pre>, so we have to strip these out
import os
import sys

in_pre = False
in_tag = False
tag = ""
s = ""

for c in sys.stdin.read():
    if in_pre:
        if in_tag:
            tag += c
            if c == '>':
                in_tag = False
            if tag == '</pre>':
                in_pre = False
                s += '</pre>'
        else:
            if c == '<':
                in_tag = True
                tag = '<'
            else:
                s += c
    else:
        s += c
        if s.endswith('<pre>'):
            in_pre = True

print s

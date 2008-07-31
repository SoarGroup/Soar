#!/usr/bin/env python

import re
import string
import sys


class Rule:

    def __init__(self):

        self.patterns = []
        self.replacements = []
        self.head = ''
        self.foot = ''
        return


class Replace:
    pass



def compile_comment():
    """Compile comment rule."""

    rule = Rule()
    rule.type = 'comment'
    rule.patterns += [re.compile('/\*.*?\*/', re.DOTALL)]
    rule.patterns += [re.compile('//.*')]

    return [rule]



def compile(prefix):
    """Compute the grammar."""

    rules = []

    # Create rule for typedefs
    #rule = Rule()
    #rule.type = 'typedef'
    #rule.patterns += [re.compile('\s*\}\s*%s_t\s*;' % prefix)]
    #rules += [rule]

    # Create rule for constructor
    rule = Rule()
    rule.type = 'constructor'
    rule.patterns += [re.compile('%s_t\s*\*\s*%s_create\s*\(.*?;' % (prefix, prefix), re.DOTALL)]

    rule.head = '\n%%extend %s\n{\n' % prefix
    rule.foot = '\n}\n'

    rep = Replace()
    rep.src = re.compile('\s*%s_create\s*' % (prefix))
    rep.dst = '%s' % prefix
    rule.replacements += [rep]

    rep = Replace()
    rep.src = re.compile('\(*\s*%s_t\s*\*\w*\s*\)' % prefix)
    rep.dst = '()'
    rule.replacements += [rep]

    rep = Replace()
    rep.src = re.compile('\(*\s*%s_t\s*\*\w*\s*,\s*' % prefix)
    rep.dst = '('
    rule.replacements += [rep]

    rules += [rule]

    # Create rule for destructor
    rule = Rule()
    rule.type = 'destructor'
    rule.patterns += [re.compile('\w*\s*%s_destroy\s*\(.*?;' % (prefix), re.DOTALL)]

    rule.head = '\n%%extend %s\n{\n' % prefix
    rule.foot = '\n}\n'

    rep = Replace()
    rep.src = re.compile('\w*\s*%s_destroy\s*' % (prefix))
    rep.dst = '~%s' % prefix
    rule.replacements += [rep]

    rep = Replace()
    rep.src = re.compile('\(*\s*%s_t\s*\*\w*\s*\)' % prefix)
    rep.dst = '()'
    rule.replacements += [rep]

    rep = Replace()
    rep.src = re.compile('\(*\s*%s_t\s*\*\w*\s*,\s*' % prefix)
    rep.dst = '('
    rule.replacements += [rep]

    rules += [rule]

    # Create rule for regular functions
    rule = Rule()
    rule.type = 'method'
    rule.patterns += [re.compile('\w*\s*%s_\w*\s*\(.*?;' % prefix, re.DOTALL)]
    #rule.patterns += [re.compile('\w*\s*%s_[a-zA-Z0-9]*\s*\(.*?;' % prefix, re.DOTALL)]
    rule.patterns += [re.compile('\w*\s*\w*\s*%s_\w*\s*\(.*?;' % prefix, re.DOTALL)]
    #rule.patterns += [re.compile('\w*\s*\w*\s*%s_[a-zA-Z0-9]*\s*\(.*?;' % prefix, re.DOTALL)]
    rule.patterns += [re.compile('\w*\s*\*%s_\w*\s*\(.*?;' % prefix, re.DOTALL)]
    #rule.patterns += [re.compile('\w*\s*\*%s_[a-zA-Z0-9]*\s*\(.*?;' % prefix, re.DOTALL)]

    rule.head = '\n%%extend %s\n{\n' % prefix
    rule.foot = '\n}\n'

    rep = Replace()
    rep.src = re.compile('%s_*' % prefix)
    rep.dst = ''
    rule.replacements += [rep]

    rep = Replace()
    rep.src = re.compile('\(*\s*%s_t\s*\*\s*\w*\s*\)' % prefix)
    rep.dst = '()'
    rule.replacements += [rep]

    rep = Replace()
    rep.src = re.compile('\(*\s*%s_t\s*\*\s*\w*\s*,\s*' % prefix)
    rep.dst = '('
    rule.replacements += [rep]

    rules += [rule]

    return rules


def extract_prefixes(instream):
    """Extract class prefixes."""

    # Remove block comments from the file
    commentRule = re.compile('/\*.*?\*/', re.DOTALL)
    tempstream = commentRule.sub ('', instream)
    commentRule = re.compile('//.*')
    tempstream = commentRule.sub ('', tempstream)

    src = re.compile('playerc_\w*_create')
    constructors = src.findall(tempstream)

    prefixes = []
    for c in constructors:
        prefixes += [c[:-7]]

    return prefixes


def parse_file(instream, rules):
    """Apply replacement rules."""

    outstream = ''
    current_struct = None

    while instream:

        line = instream
        m = None

        for rule in rules:
            # See if this line matches the rule
            for pattern in rule.patterns:
                m = pattern.match(line)
                if m:
                    break
            if not m:
                continue

            func = line[m.start():m.end()]

            # Parse comment blocks
            if rule.type == 'comment':
                #print instream[m.start():m.end()]
                instream = instream[m.end():]
                outstream += func
                break

            # Parse function name and args
            (name, sig) = string.split(func, '(', 1)
            sig = '(' + sig
            tokens = string.split(name)
            (rval, name) = (string.join(tokens[:-1]), tokens[-1])
            if name[0] == '*':
                name = name[1:]
                rval = rval + ' *'
            #print '%s | %s | %s' % (rval, name, sig)

            # Apply replacement rules to return type
            if rule.type == 'constructor':
                rval = ''

            # Apply replacement rules to name
            for rep in rule.replacements:
                mm = rep.src.search(name)
                if not mm:
                    continue
                name = name[:mm.start()] + rep.dst + name[mm.end():]

            # Apply replacement rules to signature
            for rep in rule.replacements:
                mm = rep.src.match(sig)
                if not mm:
                    continue
                sig = sig[:mm.start()] + rep.dst + sig[mm.end():]

            #print rval, name, sig

            outstream += rule.head
            outstream += '%s %s %s' % (rval, name, sig)
            outstream += rule.foot

            instream = instream[m.end():]

            break

        # If no rule matches
        if not m:
            outstream += instream[:1]
            instream = instream[1:]

    return outstream




if __name__ == '__main__':

    infilename = sys.argv[1]
    outfilename = sys.argv[2]

    # Read in the entire file
    file = open(infilename, 'r')
    instream = file.read()

    # Extract "class prefixes" from the header
    prefixes = extract_prefixes(instream)

    #prefixes = ['playerc_log']

    # Compute the grammar
    rules = []
    rules += compile_comment()
    for prefix in prefixes:
        print 'prefix: %s' % prefix
        rules += compile(prefix)

    # Parse the file and appy replacement rules
    outstream = parse_file(instream, rules)

    # Do some final replacements
    for prefix in prefixes:
        outstream = string.replace(outstream, '%s_t' % prefix, '%s' % prefix)

    guff = ''
    propguff = ''
    for prefix in prefixes:
        guff += '%%header\n %%{\ntypedef %s_t %s;\n' % (prefix, prefix)
        guff += '#define new_%s %s_create\n' % (prefix, prefix)
        guff += '#define del_%s %s_destroy\n' % (prefix, prefix)
        guff += '%}\n'
        
        # stuff for properties 
        if prefix != "playerc_mclient" and prefix != "playerc_client":
            propguff += """
%%extend %(prefix)s
{
int get_intprop (char * propname)
{
    int ret;
    if (playerc_device_get_intprop(&self->info,propname,&ret) == 0)
        return ret;
    else
        return 0;
};
int set_intprop (char * propname, int value)
{
    return playerc_device_set_intprop(&self->info,propname,value);
};

double get_dblprop (char * propname)
{
    double ret;
    if (playerc_device_get_dblprop(&self->info,propname,&ret) == 0)
        return ret;
    else
        return 0;
};
int set_dblprop (char * propname, double value)
{
    return playerc_device_set_dblprop(&self->info,propname,value);
};

char * get_strprop (char * propname)
{
    char * ret;
    if (playerc_device_get_strprop(&self->info,propname,&ret) == 0)
        return ret;
    else
        return NULL;
};
int set_strprop (char * propname, char * value)
{
    return playerc_device_set_strprop(&self->info,propname,value);
};
} """ % {"prefix": prefix}
        

    outstream = guff + outstream + propguff

    file = open(outfilename, 'w+')
    file.write(outstream)


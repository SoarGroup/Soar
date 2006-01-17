#!/usr/bin/env python
#
"""StickyForm -- state maintaining HTML forms

Intended to work with HTMLgen.

    What is StickyForm?

        It is a class which works with HTMLgen to provide state
        maintaining forms. Forms can load and save their state to and
        from files. This persistent state is what makes them 'sticky'.

    Why would I want to use it?

        The same reason you'd want to use CGI.pm ;) Actually there are
        many CGI tasks which benefit from forms which can remember
        their settings. For example, in a hypothetical CGI program, a
        user could set their preferences with a form, and later return
        to the form to modify or review them.

    How do I use it?

        Simply create forms the same way you normally would with
        HTMLgen, only use the StickyForm class instead of the Form
        class.

    StickyForm works like a Form with the addition of a state
    attribute, a save method and a restore method.

        state -- is a FormState instance which holds the default values 
                 for all form elements.
        save -- tells the FormState instance to save itself to a file.
        restore -- tells the FormState instance to restore itself from a file.

    So how do I set a form's state?

        You can set a form's state when you create it, and you can
        always change the state later by assigning something else to
        the form's state attribute.

    There are three ways to indicate a StickyForm's state upon initialization:

        filename -- this loads the state of the form from a file
        FieldStorage -- this sets the state of the form from the 
                        information in the FieldStorage
        FormState -- this sets the state to the FormState instance

    For each method here are some examples:

        form=StickyForm(state="/tmp/form.txt")

        cgi_results=cgi.FieldStorage()
        form=StickyForm(state=cgi_results)

        fs=FormState()
        form=StickyForm(state=fs)

    What is the FormState class?

        This is a simple dictionary which StickyForm uses to store its
        state information. A FormState instance can be intitalized
        with a FieldStorage instance, to create a form state which
        reflects the form data in the FieldStorage instance. This just
        means turning a slightly complex FieldStorage into a simple
        dictionary.

    How is the form's state saved?

        It is pickled to a file.

    Does StickyForm work with Bobo or DocumentTemplate?

        Yes, see the [StickyForm page].

.. [StickyForm page] http://www.aracnet.com/~amos/stickyform/
"""
# copyright 1998 by  amos latteier, amos@aracnet.com

from HTMLgen import *
from types import ListType, StringType
import UserDict
#'$Id$'
__version__ = '1.0.1'
__author__  = 'Amos Latteier, amos@aracnet.com'

# RKF - Trying out cPickle as a faster mechanism for the persistant
#       storage.
try:
    import cPickle
    def dump(data, filename):
        file = open(filename,'wb')
        cPickle.dump(data, file, 1)
        file.close()

    def load(filename):
        file = open(filename, 'rb')
        unpickled_object = cPickle.load(file)
        file.close()
        return unpickled_object
except ImportError:
    import pickle
    def dump(data, filename):
        file = open(filename,'w')
        pickle.dump(data, file)
        file.close()

    def load(filename):
        file = open(filename, 'r')
        unpickled_object = pickle.load(file)
        file.close()
        return unpickled_object

class FormState(UserDict.UserDict):
    """This is a Dictionary which holds the state of a form.

    It is like a simplified FieldStorage class.
    Each key is the name of a form element.
    Each value is either a string or a list of strings which 
    define that value.

    You can create a FormState from a FieldStorage.
    You can save and restore FormStates from text files"""

    def __init__(self, field_storage=None):
        self.data={}
        if not field_storage:
            field_storage={}

        for key in field_storage.keys():
            item=field_storage[key]
            if type(item) == ListType:
                value=[]
                for sub_item in item:
                    value.append(sub_item.value)
            else:
                value=item.value
            self.data[key]=value

    def save(self,filename):
        dump(self.data, filename)

    def restore(self,filename):
        self.data = load(filename)


class StickyForm(Form):
    """Works like a Form with the addition of a state attribute, a
    save method and a restore method.  A form's state is a FormState
    instance which holds the default values for all form elements.

    save tells the FormState instance to save itself to a file.

    restore tells the FormState instance to restore itself from a file.

    You can initalize the state 3 ways, with a:

        filename -- this loads the state of the form from a file
        FieldStorage -- this sets the state of the form from the
                        information in the FieldStorage
        FormState -- this sets the state to the FormState instance

    For example:

       form=StickyForm(state="/tmp/form.txt")

       fs=cgi.FieldStorage()
       form=StickyForm(state=fs)

       fs=FormState()
       form=StickyForm(state=fs)
"""

    def __init__(self, cgi = None, state=None,**kw):
        apply(Form.__init__, (self,cgi), kw)
        if type(state) == StringType:
            self.state=FormState()
            self.state.restore(state)
        elif state is None:
            self.state=FormState()
        elif state.__class__.__name__ == "FieldStorage":
            self.state=FormState(state)
        else:
            self.state=state

    def __str__(self):
        if not self.submit:
            self.contents.append(Input(type='submit',
                          name='SubmitButton',value='Send'))
        else:
            self.contents.append(self.submit)
        if self.reset:
            self.contents.append(self.reset)

        s = ['\n<FORM METHOD="POST"']
        if self.cgi: s.append(' ACTION="%s"' % self.cgi)
        if self.enctype: s.append(' ENCTYPE="%s"' % self.enctype)
        if self.target: s.append(' TARGET="%s"' % self.target)
        if self.name: s.append(' NAME="%s"' % self.name)
        if self.onSubmit: s.append(' onSubmit="%s"' % self.onSubmit)
        s.append('>\n')
        for item in self.contents:
            if self.state is not None:
                s.append(self.with_state(item))
            else:
                s.append(str(item))
        s.append('\n</FORM>\n')
        return string.join(s, '')

    def with_state(self,input):
        """Here's where the actual work gets done. Each Input, Select
        and Textarea object in the form is modified to reflect it's
        value as defined in the form's state.  """
        try:
            input_class=input.__class__.__name__
            if input_class not in ("Input","Select","Textarea"):
                return str(input)
        except:
            return str(input)

        if self.state.has_key(input.name):
            input_state=self.state[input.name]
        else:
            return str(input)

        if type(input_state) == ListType:
            input_state_list=input_state
        else:
            input_state_list=[input_state]

        if input_class == "Input":
            if input.type in ('checkbox','radio'):
                if input.value in input_state_list:
                    input.checked=1
            else:
                input.value=input_state

        elif input_class == "Select":
            input.selected=input_state_list

        elif input_class == "Textarea":
            input.text=input_state

        return str(input)

    def save(self,filename):
        self.state.save(filename)

    def restore(self,filename):
        self.state.restore(filename)

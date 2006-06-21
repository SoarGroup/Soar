#!/usr/bin/python

def action_link(action=None, text=None):
    if (action != None) & (text == None):
        text = action
    elif text == None:
        return "<a href='index.cgi'>Home</a>"
    return "<a href='index.cgi?action=" + action + "'>" + text + "</a>"

def welcome_page(userid=None):
    if userid == None:
        guest = True
        userid = "guest"
    print "<p>Welcome, " + userid + ".</p>"
    
    if guest == True:
        print action_link(action="login", text="Log in") + " or " + action_link(action="register") + "."
    else:
        print action_link(action="logout", text="Log out") + "."
    return

def upload_page():
    print "<form method='POST' enctype='multipart/form-data' action='index.cgi'>"
    print "User ID: <input type='text' name='userid' /><br />"
    print "Tank ID: <input type='text' name='tankid' /><br />"
    print "Tank zip: <input type='file' name='upfile' /><br />"
    print "<input type='submit' value='Upload Tank' />"
    print "<input type='hidden' name='action' value='upload' />"
    print "</form>"
    return

def save_tank():
    f = open("tanks/" + form["upfile"].filename, 'w')
    f.write(form["upfile"].value)
    f.close()
    return

#!/usr/bin/python

import smtplib

def action_link(action=None):
    if action == None:
        return "index.cgi"
    return "index.cgi?action=" + action

def action_anchor(action=None, text=None):
    if (action != None) & (text == None):
        text = action
    elif text == None:
        text = "Home"
    return "<a href='" + action_link(action) + "'>" + text + "</a>"

def welcome_page(userid=None):
    if userid == None:
        guest = True
        userid = "guest"
    print "<p>Welcome, " + userid + ".</p>"
    
    if guest == True:
        print action_anchor(action="login", text="Log in") + " or " + action_anchor(action="register") + "."
    else:
        print action_anchor(action="logout", text="Log out") + "."
    return

def register_page(userid=None, email=None):
    print "<form method='POST' action='index.cgi'>"
    
    print "User ID: <input type='text' name='userid'",
    if userid != None:
        print "value='" + userid + "'",
    print " /><br />"

    print "Email address (required, must be valid, will not be published): <input type='text' name='email'",
    if email != None:
        print "value='" + email + "'",
    print " /><br />"
    
    print "<input type='hidden' name='action' value='register' />"
    print "<input type='hidden' name='confirm' value='True' />"
    print "<input type='submit' value='Register' />"
    print "</form>"
    
def send_confirmation(userid=None, email=None):
    if (email == None) | (userid == None):
        return False
    
    message = "From: Java TankSoar Ladder <tsladder@winter.eecs.umich.edu>\r\n"
    message += "To: " + userid + " <" + email + ">\r\n"
    message += "Subject: Registration\r\n\r\n"
    message += "Confirmation code: abc123"
    
    server = smtplib.SMTP('localhost')
    server.sendmail(from_addr="tsladder@winter.eecs.umich.edu", to_addrs=email, msg=message)

def confirm_page():
    print "<form method='POST' action='index.cgi'>"
    print "Enter confirmation code from email: <input type='text' name='code' />"
    print "<input type='hidden' name='action' value='confirm' />"
    print "<input type='submit' value='Confirm' />"
    print "</form>"

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

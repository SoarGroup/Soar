#!/bin/bash
cd /home/lhamel/dev/BINAH/lib_src/ate2/lib_src/TgDIx/lib_src/gSKIx/lib_src/SKx

# Script written by LH to perform a nightly build of the soar kernel.
#
# Upon success or failure, an email is sent to gski@soartech.com 
# (Perhaps email should be sent 1/week on success.)

# send message to gski@soartech.com
#let from = "host@pirate.com"
#let to = "gski@soartech.com"
export TO=gski\@soartech.com

result=1

testForFile( ) {
   if [ -f $1 ]
   then
      echo "Successfully built $1
---
      " >> email.txt

   else
      echo "FAILURE: $1 does not exist!
---
      " >> email.txt
      result=0

   fi
}



# remove existing file
#make clean
rm -fr app_src bin cppunit depscript include lib Makefile src SoarKernelUnitTests *.cpp *.sln *.dsw update.txt

# perform checkout from CVS
cvs -q update -d > update.txt


# begin the body of the email message
echo "This is an automated email from the Soar Kernel Nightly Build.

---
" > email.txt

echo `date` >> email.txt
echo "
---
" >> email.txt

echo `pwd` >> email.txt
echo "
---
" >> email.txt

echo `whoami` >> email.txt
echo "
---
" >> email.txt


# look for existance of library
make &> make.txt

# test for build that succeeded
testForFile lib/libsoarkernel.a


cat make.txt >> email.txt


cat make.txt >> email.txt

if [ ${result} -eq 0 ]
then
   tag="failed"
else
   tag="succeeded"
fi


mail -s"Soar Kernel nightly build $tag" $TO < email.txt






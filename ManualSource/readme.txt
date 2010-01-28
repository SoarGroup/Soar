Instructions on compiling the Soar Manual

Joseph Xu
June 5, 2009

The first thing to do is to go into the "commands" subdirectory and
run the script "generate.pl", which tries to grab documentation for
all CLI commands listed on the Soar wiki at

http://winter.eecs.umich.edu/soarwiki/Documentation/CLI

and generate Tex sources representing them. You'll need these perl
modules to run it successfully:

HTML::TreeBuilder
HTML::Latex

These are all obtainable from CPAN (Comprehensive Perl Archive
Network). If you have the cpan command line program, you can just run

cpan -i HTML::TreeBuilder HTML::Latex

After generating all the tex sources for cli commands, you should be
able to just run "make" in this directory to make the final pdf.

Note that because most of the figures included in the manual are
stored as pdf's, you'll need to have pdflatex installed to compile
things successfully.

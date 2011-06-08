# Makefile to generate the Soar manual. Hopefully, all you have to do is type
# "make" (after the tex files for the commands are generated, see below)
#
# Joseph Xu Aug 2008

LATEX = pdflatex -halt-on-error # need to use pdf latex to insert some of the pdf figures correctly

SRC = a-blockscode.tex a-gds.tex a-grammars.tex a-preferences.tex a-support.tex a-tcl-io.tex a-tclexp.tex architecture.tex chunking.tex functions.tex interface.tex intro.tex manual.tex syntax.tex funclist.tex rl.tex smem.tex epmem.tex

.PHONY : trans_wiki

all : trans_wiki manual.pdf

trans_wiki ::
	cd wikicmd && sh gen_tex.sh

manual.pdf : manual.ind $(SRC)
	$(LATEX) manual
	$(LATEX) manual # has to be compiled twice to get references right

manual.ind : $(SRC)
	$(LATEX) manual
	makeindex manual
	awk -f fix_index.awk manual.ind > tmp.ind
	mv tmp.ind manual.ind

funclist.tex : wikicmd/tex
	sh genfunclist.sh
	
clean :
	rm -f manual.lof manual.log manual.toc manual.out manual.pdf manual.ind manual.ilg manual.glo manual.idx *.aux
	rm -f funclist.tex
	rm -rf wikicmd/wiki wikicmd/tex
	
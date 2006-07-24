SRC=$(shell ls *.tex)
BIBSRC=report.bib

all: report.pdf

report.dvi: $(SRC) $(BIBSRC)
	latex report
	bibtex report
	latex report
	latex report

report.pdf: report.dvi
	dvipdfm report

clean:
	rm -f report.aux report.bbl report.blg report.log report.dvi report.pdf

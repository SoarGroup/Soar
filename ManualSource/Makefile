# Makefile to generate the Soar manual. Hopefully, all you have to do is type
# "make" (after the tex files for the commands are generated, see below)
#
# Joseph Xu Aug 2008

LATEX = pdflatex -halt-on-error # need to use pdf latex to insert some of the pdf figures correctly

CMDS=add-wme                  \
     alias                    \
     capture-input            \
     cd                       \
     chunk-name-format        \
     clog                     \
     command-to-file          \
     default-wme-depth        \
     dirs                     \
     echo                     \
     echo-commands            \
     edit-production          \
     excise                   \
     explain-backtraces       \
     firing-counts            \
     gds-print                \
     gp                       \
     gp-max                   \
     help                     \
     indifferent-selection    \
     init-soar                \
     internal-symbols         \
     learn                    \
     load-library             \
     ls                       \
     matches                  \
     max-chunks               \
     max-elaborations         \
     max-memory-usage         \
     max-nil-output-cycles    \
     memories                 \
     multi-attributes         \
     numeric-indifferent-mode \
     o-support-mode           \
     popd                     \
	 port                     \
     predict                  \
     preferences              \
     print                    \
     production-find          \
     pushd                    \
     pwatch                   \
     pwd                      \
     quit                     \
     remove-wme               \
     replay-input             \
     rand                     \
     rete-net                 \
     rl                       \
     run                      \
     save-backtraces          \
     select                   \
     set-library-location     \
     set-stop-phase           \
     soarnews                 \
     source                   \
     sp                       \
     srand                    \
     stats                    \
     stop-soar                \
     time                     \
     timers                   \
     unalias                  \
     verbose                  \
     version                  \
     waitsnc                  \
     warnings                 \
     watch                    \
     watch-wmes

CMD_SRC=${addsuffix .tex,${addprefix wikicmd/tex/,$(CMDS)}}

SRC = a-blockscode.tex a-gds.tex a-grammars.tex a-preferences.tex a-support.tex a-tcl-io.tex a-tclexp.tex architecture.tex chunking.tex functions.tex interface-8.6.tex intro.tex manual8.tex syntax.tex $(CMD_SRC)

GENERATED = syntax.aux manual8.lof manual8.log intro.aux manual8.toc manual8.aux chunking.aux functions.aux manual8.out manual8.pdf architecture.aux manual8.ind manual8.ilg manual8.glo manual8.idx interface-8.6.aux

manual8.pdf : manual8.ind $(SRC)
	$(LATEX) manual8
	$(LATEX) manual8 # has to be compiled twice to get references right

manual8.ind : manual8.idx
	makeindex manual8
	sed -i -f fix_index.sed manual8.ind

manual8.idx : $(SRC)
	$(LATEX) manual8

$(CMD_SRC) :
	cd wikicmd && sh gen_tex.sh
	
clean :
	rm -rf $(GENERATED)
	rm -f $(CMD_SRC)

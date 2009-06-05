# Makefile to generate the Soar manual. Hopefully, all you have to do is type
# "make" (after the tex files for the commands are generated, see below)
#
# Joseph Xu Aug 2008

LATEX = pdflatex -halt-on-error # need to use pdf latex to insert some of the pdf figures correctly

SRC = a-blockscode.tex a-gds.tex a-grammars.tex a-preferences.tex a-support.tex a-tcl-io.tex a-tclexp.tex architecture.tex chunking.tex functions.tex interface-8.6.tex intro.tex manual8.tex syntax.tex

# You'll have to go into the commands/ directory and run generate.pl to get
# these. Automating this process is too much of a hassle and too fragile to be
# worth it
CMD_SRC = commands/add-wme.tex commands/alias.tex commands/capture-input.tex commands/cd.tex commands/chunk-name-format.tex commands/clog.tex commands/command-to-file.tex commands/default-wme-depth.tex commands/dirs.tex commands/echo.tex commands/echo-commands.tex commands/edit-production.tex commands/excise.tex commands/explain-backtraces.tex commands/firing-counts.tex commands/gds-print.tex commands/gp.tex commands/gp-max.tex commands/help.tex commands/indifferent-selection.tex commands/init-soar.tex commands/internal-symbols.tex commands/learn.tex commands/load-library.tex commands/ls.tex commands/matches.tex commands/max-chunks.tex commands/max-elaborations.tex commands/max-memory-usage.tex commands/max-nil-output-cycles.tex commands/memories.tex commands/multi-attributes.tex commands/numeric-indifferent-mode.tex commands/o-support-mode.tex commands/popd.tex commands/predict.tex commands/preferences.tex commands/print.tex commands/production-find.tex commands/pushd.tex commands/pwatch.tex commands/pwd.tex commands/quit.tex commands/remove-wme.tex commands/replay-input.tex commands/rand.tex commands/rete-net.tex commands/rl.tex commands/run.tex commands/save-backtraces.tex commands/select.tex commands/set-library-location.tex commands/set-stop-phase.tex commands/soarnews.tex commands/source.tex commands/sp.tex commands/srand.tex commands/stats.tex commands/stop-soar.tex commands/time.tex commands/timers.tex commands/unalias.tex commands/verbose.tex commands/version.tex commands/waitsnc.tex commands/warnings.tex commands/watch.tex commands/watch-wmes.tex 

GENERATED = syntax.aux manual8.lof manual8.log intro.aux manual8.toc manual8.aux chunking.aux functions.aux manual8.out manual8.pdf architecture.aux manual8.ind manual8.ilg manual8.glo manual8.idx interface-8.6.aux

manual8.pdf : manual8.ind $(SRC) $(CMD_SRC)
	$(LATEX) manual8
	$(LATEX) manual8 # has to be compiled twice to get references right

manual8.ind : manual8.idx
	makeindex manual8
	sed -i -f fix_index.sed manual8.ind

manual8.idx : $(SRC)
	$(LATEX) manual8

clean :
	rm -rf $(GENERATED)

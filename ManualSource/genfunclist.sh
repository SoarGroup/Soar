rm -f funclist.tex

for f in wikicmd/tex/*
do
	cmd=`echo $f | awk -F[./] '{print $3}'`
	awk '
		BEGIN {cmd="'$cmd'"}
		/^\\index/ {
			insummary=1; next 
		}
		/^\\subsubsection/ { 
			printf("\\soar{%s} & %s & \\pageref{%s} \\\\\n", cmd, s, cmd)
			exit
		}
		insummary == 1 {
			s = s " " $0
		}
	' $f >>funclist.tex
done


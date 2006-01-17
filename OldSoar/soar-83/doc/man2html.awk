#!//bin/nawk


$0 ~ /'[\/\\]"/ || $1 == "'"    { next }    # eat [nt]roff comments

# defining macros - eat them 
/^\.de.*/ {
        getline
        while ( $0 !~ "^\.\.$" )
        {
            getline
        }
        getline
    }

$1 == ".VS" || $1 == ".VE" || $1 == ".AS"  { next }


# Convert .IP Paragraphs upto next .cmd to hanging indents
#       using <UL></UL> pairs without intervening <LI>

/^\.IP */ {
        if ( inIP > 0 )
        {
            print "</UL>"
        }
        inIP = 1 
        print "<UL>"
        match($0, /".*"/ )
        if ( RSTART > 0 )
        {
            arg = substr( $0, RSTART+1, RLENGTH-2)
            
            print arg " <BR>"
        }
        else if ( length( $2 ) > 0 )
        {
            print $2 " <BR>"
        }
        next
    }

$0 ~ /^\.[a-zA-Z]*/ && inIP > 0 {
        inIP = 0
        print "</UL>"
    }

# Convert 
# .TP
# Line1
# line 2 - n
# .Any              
#
# to
# <DL>
# <DT> Line1
# <DD> lines 2 - n 
# <DT>

/^\.TP */ {
        if ( inTP > 0 )
        {
            print "</DL>"
        }
        inTP = 1 
        print "<DL>"
        next
    }

inTP == 1 && $1 !~ /\.[a-zA-Z]*/ {
        print "<DT> " $0
        inTP = 2
        next
    }

inTP == 2 && $1 !~ /\.[a-zA-Z]*/{
        print "</I></B>"    # Belt and suspenders
        print "<DD> " $0
        inTP = 3
        next
    }

$0 ~ /^\.[a-zA-Z]*/ && inTP > 0 {
        inTP = 0
        print "</DL>"
    }



$1 == ".AP" {
        $1=""
        print "<DL >"
        print "<DT> " $2 "\t\t" $3 "\t\t("$4")"
        inTP = 2
        next
    }

# make a blank line 
$1 == ".sp" {
        print "<BR>"
        next #        print "<BR>"
    }


$1 == ".ta"  { next }

# just pass everything else on

    { print $0 }    



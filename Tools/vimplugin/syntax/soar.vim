" Vim syntax file for the Soar production language
"
" Language:    Soar
" Maintainer:  Joseph Xu (jzxu@umich.edu)
" Last Change: Apr 23,  2007 (pretty much rewrote it, folding now working)
"              Apr  3,  2007 (using hi link instead of assigning colors)
"              Sept 07, 2006 (first version)

syn clear

setlocal iskeyword+=-
syn keyword soarCommand pushd popd source multi-attributes learn watch indifferent-selection state

syn region soarProd matchgroup=soarProdBraces start=/^\s*sp\s*{/ end=/}/ contains=soarProdStart,soarProdName fold

syn match  soarProdStart /^\s*sp\s*{/ nextgroup=soarProdName contained skipwhite

syn match  soarProdName /[-_\*a-zA-Z0-9]\+\s*$/ nextgroup=soarCond,soarDoc,soarProdFlag contained skipwhite skipnl

syn region soarDoc start=/"""/ end=/"""/ nextgroup=soarCond,SoarProdFlag contained skipwhite skipnl

syn match  soarProdFlag /:\(o-support\|i-support\|chunk\|default\)/ nextgroup=soarCond contained skipwhite skipnl

syn region soarCond  matchgroup=soarCondParen start=/-\?(/ end=/)/ nextgroup=soarCond,soarDisjunc,soarArrow contains=soarCondI contained skipwhite skipnl
syn region soarCondI start=/(/ms=e+1 end=/)/me=s-1 contained

syn region soarDisjunc start=/-{/ end=/}/ nextgroup=soarCond,soarDisjunc contains=soarCond contained skipwhite skipnl

syn match  soarArrow /-->/ nextgroup=soarAction skipwhite skipnl contained

syn region soarAction  matchgroup=soarActionParen start=/(/ end=/)/ nextgroup=soarAction contains=soarActionI contained skipwhite skipnl
syn region soarActionI start=/(/ms=e+1 end=/)/me=s-1 contained

syn match  soarVar     /<[-_a-zA-Z0-9]\+>/      containedin=soarCond,soarAction contained
syn match  soarAttrib  /-\?\^[-\._a-zA-Z0-9]\+/ containedin=soarCond,soarAction contained
syn region soarString start=/|/ end=/|/ containedin=soarCond,soarAction contained

syn match  soarComment /#.*/ oneline containedin=ALL
"
"syn region soarMathExp start=/(/ end=/)/ contains=soarMathExp contained containedin=soarCondI,soarActionI
"syn region soarConjTest start=/{/ end=/}/ containedin=soarCondI,soarActionI

"hi soarProdStart  guifg=Red    ctermfg=Red     term=bold    gui=bold

hi soarProdBraces     guifg=Red          ctermfg=Red
hi soarArrow          guifg=Red          ctermfg=Red
hi soarCondParen      guifg=Blue         ctermfg=Blue
hi soarActionParen      guifg=Green         ctermfg=Green

" only for debugging
"hi soarCond ctermbg=Gray
"hi soarAction ctermbg=Blue
"hi soarMathExp ctermbg=Red

hi link soarCommand   Keyword
hi link soarProdName  Function
hi link soarProdFlag  Keyword
hi link soarDoc       String
hi link soarVar       Identifier
hi link soarAttrib    Type
hi link soarComment   Comment
hi link soarString    String

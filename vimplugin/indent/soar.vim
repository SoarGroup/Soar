" Indentation script for Soar production language
"
" Author:      Joseph Xu (jzxu@umich.edu)
" Last Change: Sept 21, 2006 (added correct behavior for braces, bug fixes)
"              Sept 07, 2006 (first version)

if exists("b:did_indent")
  finish
endif
let b:did_indent = 1

if exists("soar_did_indent")
  finish
endif
let soar_did_indent = 1

setlocal indentexpr=GetSoarIndent()
setlocal indentkeys='0=sp,0={,0},0=-{,0(,0=-(,0^,0=-^,0=-->,!^F'

function s:GetNonFullCommentLine(lnum)
  let lnum = prevnonblank(a:lnum - 1)
  let lc = getline(lnum)
  while lc =~ '^\s*#' && lnum > 0
    let lnum = prevnonblank(lnum-1)
    let lc = getline(lnum)
    echo lnum
  endwhile
  return lnum
endfunction

function s:FindCaret(lnum)
  let lnum = s:GetNonFullCommentLine(a:lnum)
  let lc = getline(lnum)
  if lc =~ ')\s*$'
    return -1
  elseif lc =~ '\^'
    return match(lc, '\^')
  else
    return -1
  endif
endfunction

function s:FindOpenBrace(lnum)
  "let lnum = s:GetNonFullCommentLine(a:lnum)
  let lnum = searchpair('{','','}','bW','getline(".") =~ "^\\s*\#"')
  let lc = getline(lnum)
  while lc !~ '^\s*\<sp\s*{' && lc !~ '^\s*-\?{\s*(' && lnum > 0
    "let lnum = s:GetNonFullCommentLine(lnum)
    let lnum = searchpair('{','','}','bW','getline(".") =~ "^\\s*\#"')
    let lc = getline(lnum)
  endwhile

  if lc =~ '\<sp\s*{' " This is for start of production
    return match(lc, '{')
  elseif lc =~ '^\s*-\?{\s*(' " This is for condition conjunction
    return match(lc, '(')
  else
    return -1
  endif
endfunction

function GetSoarIndent()
  let line = getline(v:lnum)

  if line =~ '^\s*\^'                        "attribute
    return s:FindCaret(v:lnum)
  elseif line =~ '^\s*-\^'                   "negated attribute
    let spaces = s:FindCaret(v:lnum)
    if spaces > 0
      return spaces - 1
    else
      return spaces
    endif
  elseif line =~ '^\s*('                     "condition
    return s:FindOpenBrace(v:lnum)
  elseif line =~ '^\s*-(' || line =~ '^\s*-{' "negated condition
    let spaces = s:FindOpenBrace(v:lnum)
    if spaces > 0
      return spaces - 1
    else
      return spaces
    endif
  elseif line =~ '^\s*-->'                  "arrow
    return 0
  elseif line =~ '^\s*sp'               "beginning of production
    return 0
  elseif line =~ '^\s*}'  "end of production, on its own line
    return 0
  else
    return -1
  endif
endfunction

" Useful functions for editing Soar files, including:
"
"   Chaining variables automatically
"   Commenting/Uncommenting productions
"   Jumping to next/previous production
"   Creating apply rule headers from proposal rule (to be expanded)
"   Code folding by productions and header comments (starting with ###)
"
" The mappings from key combinations to function calls is at the
" bottom of the file.
"
" Author:      Joseph Xu (jzxu@umich.edu)
" Last Change: Apr  23, 2007 (using syntax code folding)
"              Nov  11, 2006 (added code folding)
"              Sept 23, 2006 (first version)


if exists("b:did_ftplugin")
  finish
endif
let b:did_ftplugin = 1

if exists("soar_did_ftplugin")
  finish
endif
let soar_did_ftplugin = 1

function s:NextBlankLine()
  let lnum = line('.')
  while getline(lnum) !~ '^\s*$' && lnum <= line('$')
    let lnum = lnum + 1
  endwhile

  return lnum
endfunction

function s:PrevBlankLine()
  let lnum = line('.')
  while getline(lnum) !~ '^\s*$' && lnum > 0
    let lnum = lnum - 1
  endwhile

  return lnum
endfunction

function s:MatchStrRev(str, pat)
  let si = match(a:str, a:pat)
  let lsi = si
  while si >= 0
    let lsi = si
    let si = match(a:str, a:pat, si+1)
  endwhile
  
  if lsi >= 0
    return strpart(a:str, lsi, matchend(a:str, a:pat, lsi)-lsi)
  else
    return ""
  endif
endfunction

function SoarFindProdStart(flags)
  " get to the top level opening brace
  let topbracelnum = SoarFindProdOpenBrace("n")
  if topbracelnum < 0
    return -1
  endif

  if getline(topbracelnum) =~ '\<sp\>'
    let spline = topbracelnum
  else
    let spline = prevnonblank(topbracelnum-1)
    if getline(spline) !~ '\<sp\>'
      return -2
    endif
  endif

  if a:flags !~ 'n'
    call cursor(spline,1)
  endif

  return spline
endfunction

function SoarFindProdOpenBrace(flags)
  let origline = line('.')
  let origcol = col('.')

  let bracelnum = searchpair('{','','}','bW','getline(".") =~ "^\\s*\#"')
  let topbracelnum = -1
  while bracelnum > 0
    let topbracelnum = bracelnum
    call cursor(bracelnum,1)
    let bracelnum = searchpair('{','','}','bW','getline(".") =~ "^\\s*\#"')
  endwhile

  if topbracelnum < 0
    if getline(origline) =~ '{'
      let topbracelnum = origline
    else
      call cursor(origline, origcol)
      return -1
    endif
  endif

  if a:flags =~ 'n'
    call cursor(origline, origcol)
  else
    call cursor(topbracelnum,1)
  endif
  return topbracelnum

endfunction

function SoarFindProdCloseBrace(flags)
  let origline = line('.')
  let origcol = col('.')

  let bracelnum = searchpair('{','','}','W','getline(".") =~ "^\\s*\#"')
  let botbracelnum = -1
  while bracelnum > 0
    let botbracelnum = bracelnum
    call cursor(bracelnum, strlen(getline(bracelnum)))
    let bracelnum = searchpair('{','','}','W','getline(".") =~ "^\\s*\#"')
  endwhile

  if botbracelnum < 0
    if getline(origline) =~ '}'
      let botbracelnum = origline
    else
      call cursor(origline, origcol)
      return -1
    endif
  endif

  if a:flags =~ 'n'
    call cursor(origline, origcol)
"  else
"    call cursor(botbracelnum,1)
  endif
  return botbracelnum
endfunction

function SoarGetProdName()
  let lnum = SoarFindProdOpenBrace("n")
  if lnum < 0
    return "error"
  endif

  let lc = getline(lnum)
  if lc !~ '{\s*$'
    return strpart(lc, stridx(lc, '{') + 1)
  endif

  let lnum = nextnonblank(lnum + 1)

  if lnum == 0
    return ""
  endif

  return getline(lnum)
endfunction

function SoarNextProd(flags)
  if a:flags =~ 'n'
    let origline = line('.')
    let origcol = col('.')
    let ret = search('\<sp\>', 'nW')
    call cursor(origline, origcol)
    return ret
  else
    return search('\<sp\>', 'W')
  endif
endfunction

function SoarPrevProd(flags)
  if a:flags =~ 'n'
    let origline = line('.')
    let origcol = col('.')
    let ret = search('\<sp\>', 'bnW')
    call cursor(origline, origcol)
    return ret
  else
    return search('\<sp\>', 'bW')
  endif
endfunction


function SoarChainVar()
  let line = getline('.')

  " make sure this line is in the right format
  if line !~ '<\(\w\|-\)\+>\s*)\s*$'
    return 0
  endif

  let lnum = line('.')
  let vn = s:MatchStrRev(getline("."), '<\(\w\|-\)\+>')
  if strlen(vn) > 0
    call append(lnum, "(" . vn . " ^")
    execute "normal jA"
    return 1
  endif
  return 0
endfunction

function SoarCreateApplyRuleHeader()
  let prodname = SoarGetProdName()
  if prodname == "" || prodname !~ 'propose'
    return -1
  endif

  let applyname = substitute(prodname, 'propose', 'apply', '')

  let lastline = SoarFindProdCloseBrace("n")
  if lastline < 0
    let lastline = SoarNextProd("n") - 1
    if lastline < 0
      let lastline = prevnonblank(line('$')) + 1 
    endif
  endif

  call append(lastline, "")
  call append(lastline + 1, "sp {" . applyname)
  call append(lastline + 2, "   (")
  call cursor(lastline + 3, 4)

  return 1
endfunction

function SoarCommentProd()
  let prodstart = SoarFindProdStart("n")
  if prodstart < 0
    return -1
  end

  let prodend = SoarFindProdCloseBrace("n")
  if prodend < 0
    return -2
  end

  let i = prodstart
  while i <= prodend
    let cl = "##.. " . getline(i)
    call setline(i, cl)
    let i = i + 1
  endwhile
endfunction

function SoarUncommentProd()
  let origline = line('.')

  let i = origline
  while getline(i) =~ '^##\.\.'
    call setline(i, strpart(getline(i), 5))
    let i = i - 1
  endwhile

  let i = origline + 1
  while getline(i) =~ '^##\.\.'
    call setline(i, strpart(getline(i), 5))
    let i = i + 1
  endwhile

endfunction

function SoarToggleComment()
  if getline('.') =~ '^##\.\.'
    call SoarUncommentProd()
  else
    call SoarCommentProd()
  endif
endfunction

set foldmethod=syntax
set foldlevel=99

" KEY MAPPINGS


" The following allows you to press Alt-e during insertion
" mode and automatically chains the last variable to another
" condition.
"
" Example:
"
" (<x> ^attr <y>)
"
" becomes 
"
" (<x> ^attr <y>)
" (<y> ^
"
imap <M-e> <Esc>:call SoarChainVar()<CR>a<C-f>

map <M-j> :call SoarNextProd("")<CR>
map <M-k> :call SoarPrevProd("")<CR>
map <M-e> :call SoarFindProdCloseBrace("")<CR>

map <M-a> :call SoarCreateApplyRuleHeader()<CR>
map <M-c> :call SoarToggleComment()<CR>


" This is to work with Taglist
let tlist_soar_settings='soar;p:productions'

" * - are both keyword characters in soar lingo
set iskeyword+=*
set iskeyword+=-

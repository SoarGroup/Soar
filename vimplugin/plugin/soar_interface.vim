
function PrintId()
  let Word = expand('<cword>')
  exec "python exec_cmd_buf('print " Word "')"
endfunction

function PrintIdSilent()
  let Word = expand('<cword>')
  exec "python exec_cmd_prev('print " Word "')"
endfunction

function PrintIdWatch()
  let prod = expand('<cword>')
  exec "5new +set\\ bt=nofile"
  exec "python watch('p " prod "')"
endfunction

function PrintMatchSilent()
  let prod = expand('<cword>')
  exec "python exec_cmd_prev('match " prod "')"
endfunction

function PrintMatch()
  let prod = expand('<cword>')
  exec "python exec_cmd('match " prod "')"
endfunction

function PrintMatchWatch()
  let prod = expand('<cword>')
  exec "5new +set\\ bt=nofile"
  exec "python watch('match " prod "')"
endfunction

function SourceCurrFile()
  let dirname = substitute(expand("%:p:h"), '\\', '\\\\', 'g')
  let filename = expand("%:t")
  "exec "python exec_cmd('source " filename "')"
  exec "python exec_cmd('cd " dirname "')"
  exec "python exec_cmd('source " filename "')"
endfunction

function ToggleHighlightSG()
  if !exists("b:hl")
    syn match w1sg /^\s*[0-9]\+:\s*==>S: S[0-9]\+.*/
    highlight w1sg guifg=white guibg=red ctermfg=white ctermbg=red
    let b:hl=1
  else
    syn clear w1sg
    unlet b:hl
  endif
endfunction

function HighlightWord()
  call UnhighlightWord()

  let b:user_word_hl=1

  set iskeyword-=*

  let word = expand('<cword>')
  exec "syntax match user_word /.*" . word . ".*/"
  highlight user_word guifg=white guibg=red ctermfg=white ctermbg=red

  set iskeyword+=*
endfunction

function UnhighlightWord()
  if exists("b:user_word_hl")
    unlet b:user_word_hl
    syntax clear user_word
    highlight clear user_word
  endif
endfunction

function SoarInterfaceMode()

  set iskeyword+=-
  set iskeyword+=*

  " import the python module
  python import sys
  "python sys.path.append('C:\\Documents and Settings\\jzxu\\vimfiles\\plugin')
  python sys.path.append('/home/jzxu/.vim/plugin')
  python from SoarVim import *
  
  " everytime a buffer unloads, remove it from the watch list
  augroup SoarInterface
    au BufUnload * python unwatch()
    au BufUnload * python unlog_print()
  augroup END

  " set some key mappings that are useful
  
  map <C-enter> :S 
  map <F2>      :python exec_cmd('step')<CR>
  map <F3>      :python exec_cmd('run -e 1')<CR>
	map <F4>      :python exec_cmd('stop -s')<CR>

  nmap <S-F5>    :call PrintIdSilent()<CR>
  nmap <F5>      :call PrintId()<CR>
  nmap <C-S-F5>  :call PrintIdWatch()<CR>
  nmap <S-F6>    :call PrintMatchSilent()<CR>
  nmap <F6>      :call PrintMatch()<CR>
  nmap <C-S-F6>  :call PrintMatchWatch()<CR>

  nmap <F7>      :call ToggleHighlightSG()<CR>
  nmap <S-F7>    :call HighlightWord()<CR>
  nmap <C-S-F7>  :call UnhighlightWord()<CR>

  nmap <F9>      :python create_kernel()<CR>
  nmap <S-F9>    :python connect()<CR>
  nmap <F10>     :python init_agent()<CR>
  nmap <F11>     :python exec_cmd('excise --all')<CR>
  nmap <F12>     :call SourceCurrFile()<CR>

  " execute command, print output in command window
  command -nargs=* SS exec 'python exec_cmd("<args>")'

  " execute command, print output into current buffer
  command -nargs=* S exec 'python exec_cmd_buf("<args>")'

  " short hand for setting a watch
  command -nargs=* Sw exec  'python watch("<args>")'
  command          Suw exec 'python unwatch()'

  " since we make a lot of new scratch windows, might is well define a command
  command Snew exec  'new +set\ bt=nofile'
  command Svnew exec 'vnew +set\ bt=nofile'

endfunction

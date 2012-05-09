"set showcmd		" Show (partial) command in status line.
"set showmatch		" Show matching brackets.
set ignorecase		" Do case insensitive matching
"set smartcase		" Do smart case matching
set incsearch		" Incremental search
"set autowrite		" Automatically save before commands like :next and :make
"set hidden             " Hide buffers when they are abandoned
set mouse=a		" Enable mouse usage (all modes)
set nu
set ts=4
set ai
"set hlsearch
let c_space_errors=1

" Source a global configuration file if available
if filereadable("/etc/vim/vimrc.local")
  source /etc/vim/vimrc.local
endif

nmap q :xa!<cr>
nmap hq :qa!<cr>
nmap w :wa!<cr>
nmap hw :wa!<cr>i

nmap fs :%s/\s*$//g<cr>:%s/\n{\r\r\1}/g<cr>:%s/(\s*/(/g<cr>:%s\s*)/)/g<cr>
nmap fh :%s/for\s*(/for (/g<cr>:%s/while\s*(/while (/g<cr>:%s/if\s*(/if (/g<cr>
nmap fi gg:s/.*/#include \r&/g<cr>k$i
nmap fc ggVGdi
nmap < :s/\s\(.*\)/\1&/g
nmap > :s/.*/\t&/g
nmap ff	gg=G

nmap js :s/.*/struct \r{\r\r\1};/<cr>kkk$a
nmap jc :s/.*/class \r{\rprivate:\r\r\rpublic:\r\r};/<cr>kkkkkkk$a
nmap jf kyyp:s/[\t{]/\t/g<cr>:s/\(\s*\).*/\1for (i = 0; i < ; i++)\r\1{\r\r\1}/g<cr>kkk$hhhhhi
nmap jw kyyp:s/[\t{]/\t/g<cr>:s/\(\s*\).*/\1while ()\r\1{\r\r\1}/g<cr>kkk$i
nmap jd kyyp:s/[\t{]/\t/g<cr>:s/\(\s*\).*/\1do\r\1{\r\r\1} while ();/g<cr>$hi
nmap ji kyyp:s/[\t{]/\t/g<cr>:s/\(\s*\).*/\1if ()\r\1{\r\r\1}/g<cr>kkk$i
nmap je kyyp:s/[\t{]/\t/g<cr>:s/\(\s*\).*/\1else if ()\r\1{\r\r\1}/g<cr>kkk$i
nmap jl kyyp:s/[\t{]/\t/g<cr>:s/\(\s*\).*/\1else\r\1{\r\1\t\r\1}/g<cr>k$a
nmap jm gg:s/.*/#include <cavan.h>\r&/g<cr>G:s/.*/&\rint main(int argc, char *argv[])\r{\r\t\r\treturn 0;\r}/<cr>kkk$a<cr>
nmap jh :s/.*/&\rint (void)\r{\r\r\treturn 0;\r}/g<cr>kkkk$hhhhhi
nmap ja :s/.*/\t\.global main\rmain\:\r\tstmfd sp!, {lr}\r\r\tldr r0, =string1\r\tbl printf\r\t\r\r\tldmfd sp!, {pc}\r\rstring1\:\r\t\.asciz "r1 = %#x, r2 = %#x, r3 = %#x\\n"\r\t\.align 2/<cr><cr>kkkkkk$a
nmap j/ :s/\s*/&\/\//<cr>
nmap j\ :s/\(\s*\)\/*/\1/<cr>
nmap j( :s/\(\s*\).*/&()\r\1{\r\r\1}/g<cr>kkk$i
nmap j{ :s/\(\s*\).*/&\r\1{\r\1\t\r\1}/g<cr>k$a

nmap ls :set mouse=a<cr>:set nu<cr>
nmap ln :set mouse=<cr>:set nonu<cr>
nmap lc :wa<cr>:!cavan-gcc<cr>
nmap le :!./a.out<cr>
nmap lm :wa<cr>:!make<cr>

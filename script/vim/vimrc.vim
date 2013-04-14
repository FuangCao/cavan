for path in [$CAVAN_HOME, "/cavan", $HOME."/cavan", $PWD."/cavan", $PWD]
	if isdirectory(path)
		let g:CAVAN_HOME = path
		break
	endif
endfor

function s:list_directory(dirname, type)
	return split(globpath(a:dirname, a:type), "\n")
endfunction

function s:path_join(dirname, filename)
	return substitute(a:dirname, '/*$', '/' . a:filename, 'g')
endfunction

function s:path_dirname(pathname)
	let index = match(a:pathname, '/+$')
	if index == 0
		return "/"
	elseif index < 0
		let pathname = a:pathname
	else
		let pathname = strpart(a:pathname, 0, index)
	endif

	let index = strridx(pathname, '/')
	if index == 0
		return "/"
	elseif index < 0
		return "."
	else
		return strpart(pathname, 0, index)
	endif
endfunction

function s:path_basename(pathname)
	let index = match(a:pathname, '/*$')
	let pathname = strpart(a:pathname, 0, index)
	let index = strridx(pathname, '/')
	if index < 0
		return pathname
	else
		return strpart(pathname, index + 1)
	endif
endfunction

set showcmd		" Show (partial) command in status line.
set showmatch		" Show matching brackets.
set ignorecase		" Do case insensitive matching
set smartcase		" Do smart case matching
set incsearch		" Incremental search
set autowrite		" Automatically save before commands like :next and :make
set hidden             " Hide buffers when they are abandoned
set nu
set ai
" 显示特殊字符
" set list
set nowrap
set cindent
" set cursorline
set nocompatible
set smartindent
set ruler
set wildmenu
set hlsearch
" 自动重新加载外部修改内容
set autoread

set updatetime=100
set guioptions-=T
set shiftwidth=4
set tabstop=4
set history=1000
set mouse=a		" Enable mouse usage (all modes)
set fencs=utf-8,cp936
set backspace=indent,eol,start
" set columns=80
" 保存文件的格式顺序
set fileformats=unix,dos
" 设置帮助信息
set helplang=cn

filetype on
" colorscheme darkblue

let &path = s:path_join(g:CAVAN_HOME, "include") . "," . &path

let c_space_errors = 1
let Tlist_Use_Right_Window = 0
let Tlist_File_Fold_Auto_Close = 1

if has("autocmd")
	au BufReadPost * if line("'\"") > 0 && line("'\"") <= line("$") | exe "normal g'\"" |
	au FileType python set omnifunc=pythoncomplete#Complete
endif

if has("syntax")
	syntax on
endif

let dirname = s:path_join(g:CAVAN_HOME, "script/vim")

for pathname in s:list_directory(dirname, "*.vim")
	if match(pathname, '/vimrc.vim$') < 0
		execute "source " . pathname
	endif
endfor

if has("cscope")
	set csprg=/usr/bin/cscope
	set csto=0
	set cst
	set nocsverb

	set csverb
	set cscopetag
	"set cscopequickfix=s-,g-,d-,t-,e-,f-,i-

	let dirname = getcwd()

	while 1
		let pathname = s:path_join(dirname, "cscope.out")

		if filereadable(pathname)
			silent execute printf("cs add %s %s", pathname, dirname)
			break
		endif

		if dirname == '.' || dirname == '/'
			break
		endif

		let dirname = s:path_dirname(dirname)
	endwhile
endif

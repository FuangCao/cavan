vmap <S-Tab>	:s/^\s\?//g<CR>:set nohls<CR>
vmap <Tab>		:s/^.\+/\t&/g<CR>:set nohls<CR>
vmap <Space>	:s/\t/    /g<CR>:set nohls<CR>

nmap <S-Tab>	:set nohls<CR>:'<,'>s/^\s\?//g<CR>
nmap <Tab>		:set nohls<CR>:'<,'>s/^.\+/\t&/g<CR>

nmap q			:xa!<CR>
nmap w			:wa!<CR>
nmap m			:!make<CR>
nmap <C-n>		:set mouse=v<CR>:set nonu<CR>
nmap <C-a>		:set mouse=a<CR>:set nu<CR>
nmap <C-f>		:%s/\s\+$//g<CR>

function s:set_keymap_c()
	map \\				:'<,'>s/^\(\s*\)\/\/\+\s*/\1/g<CR>:set nohls<CR>
	vmap /				:s/^\(\s*\)\(.\+\)/\1\/\/ \2/g<CR>:set nohls<CR>
	vmap \\				:s/^\(\s*\)\/\/\+\s*/\1/g<CR>:set nohls<CR>

	imap (<Tab>			()<CR>{<CR>}<Esc>kk$i
	imap {<Tab>			{<CR>}<Esc>k$a<CR>
	imap {}				{<Tab>

	nmap <Tab>f			k$a<CR>for (i = 0; i < ; i++)<CR>{<CR>}<Esc>kk$hhhhhi
	nmap f<Tab>			<Tab>f
	nmap <Tab>w			k$a<CR>while (<Tab>
	nmap w<Tab>			<Tab>w
	nmap <Tab>dd		k$a<CR>do<CR>{<CR><CR>} while ();<Esc>$hi
	nmap d<Tab>			<Tab>dd
	nmap <Tab>ii		k$a<CR>if (<Tab>
	nmap <Tab>ee		k$a<CR>else<CR>{<Tab>
	nmap e<Tab>			<Tab>ee
	nmap <Tab>ei		k$a<CR>else if (<Tab>
	nmap ei<Tab>		<Tab>ei
	nmap <Tab>r			k$a<CR>return ;<Left>
	nmap r<Tab>			<Tab>r
	nmap <Tab>b			k$a<CR>break;
	nmap b<Tab>			<Tab>b
	nmap <Tab>cc		k$a<CR>continue;
	nmap c<Tab>			<Tab>cc

	nmap <Tab>s			istruct <CR>{<CR>};<CR><Esc>kkk$a
	nmap s<Tab>			<Tab>s
	nmap <Tab>en		ienum <CR>{<CR>};<CR><Esc>kkk$a
	nmap <Tab>u			iunion <CR>{<CR>};<CR><Esc>kkk$a
	nmap <Tab>h			istatic int (void)<CR>{<CR>return 0;<CR><Backspace>}<CR><Esc>kkkk$hhhhhi
	nmap h<Tab>			<Tab>h
	nmap <Tab>m			iint main(int argc, char *argv[])<CR>{<CR><CR>return 0;<CR><Backspace>}<Esc>ggi#include <stdio.h><CR><CR><Esc><C-o>kkk$a<CR>
	nmap m<Tab>			<Tab>m
	nmap <Tab>in		i#include <><Left>
	nmap <Tab>de		i#define<Space>
endfunction

function s:set_keymap_cpp()
	call s:set_keymap_c()
	nmap <Tab>f		k$a<CR>for (int i = 0; i < ; i++)<CR>{<CR>}<Esc>kk$hhhhhi
	nmap <Tab>cl	iclass <CR>{<CR><Backspace>private:<CR><CR><CR><Backspace>public:<CR><CR>};<Esc>kkkkkkk$a
endfunction

function s:set_keymap_java()
	call s:set_keymap_cpp()
	nmap <Tab>cl	ipublic class <CR>{<CR>}<Esc>kk$a
endfunction

function s:set_keymap_asm()
	map \\			:'<,'>s/^\(\s*\);\+\s*/\1/g<CR>:set nohls<CR>
	vmap /			:s/^\(\s*\)\(.\+\)/\1; \2/g<CR>:set nohls<CR>
	vmap \\			:s/^\(\s*\);\+\s*/\1/g<CR>:set nohls<CR>
	imap m<Tab>		.global main<CR><CR>main:<CR><Tab>stmfd sp!, {lr}<CR><CR>ldr r0, =string1<CR>bl printf<CR><CR><CR><CR>ldmfd sp!, {pc}<CR><CR>string1:<CR><Tab>.asciz "r1 = %#x, r2 = %#x, r3 = %#x\n"<CR>.align 2<Esc>kkkkkk$a<Tab>
endfunction

function s:set_keymap_vim()
	map \\			:'<,'>s/^\(\s*\)"\+\s*/\1/g<CR>:set nohls<CR>
	vmap /			:s/^\(\s*\)\(.\+\)/\1" \2/g<CR>:set nohls<CR>
	vmap \\			:s/^\(\s*\)"\+\s*/\1/g<CR>:set nohls<CR>
endfunction

function s:set_keymap_sh()
	map \\			:'<,'>s/^\(\s*\)#\+\s*/\1/g<CR>:set nohls<CR>
	vmap /			:s/^\(\s*\)\(.\+\)/\1# \2/g<CR>:set nohls<CR>
	vmap \\			:s/^\(\s*\)#\+\s*/\1/g<CR>:set nohls<CR>
endfunction

function s:set_keymap_make()
	call s:set_keymap_sh()
endfunction

function s:set_keymap_python()
	call s:set_keymap_sh()
endfunction

if has("autocmd")
	autocmd FileType c		call s:set_keymap_c()
	autocmd FileType cpp	call s:set_keymap_cpp()
	autocmd FileType java	call s:set_keymap_java()
	autocmd FileType asm	call s:set_keymap_asm()
	autocmd FileType vim	call s:set_keymap_vim()
	autocmd FileType sh		call s:set_keymap_sh()
	autocmd FileType make	call s:set_keymap_make()
	autocmd FileType python	call s:set_keymap_python()
endif

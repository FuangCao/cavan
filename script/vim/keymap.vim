map <S-Tab>		:set nohls<CR>:'<,'>s/^\s\?//g<CR>
map <Tab>		:set nohls<CR>:'<,'>s/^.\+/\t&/g<CR>
map <Space>		:set nohls<CR>:'<,'>s/^.\+/ &/g<CR>

vmap <S-Tab>	:s/^\s\?//g<CR>:set nohls<CR>
vmap <Tab>		:s/^.\+/\t&/g<CR>:set nohls<CR>
vmap <Space>	:s/^.\+/ &/g<CR>:set nohls<CR>

nmap q			:xa!<CR>
nmap w			:wa!<CR>
nmap <C-m>		:set mouse=a<CR>:set nu<CR>
nmap <C-n>		:set mouse=v<CR>:set nonu<CR>

function s:set_keymap_c()
	vmap //			:s/^\(\s*\)\(.\+\)/\1\/\/ \2/g<CR>:set nohls<CR>
	vmap \\			:s/^\(\s*\)\/\+\s*/\1/g<CR>:set nohls<CR>
	imap f<Tab>		for (i = 0; i < ; i++)<CR>{<CR><CR>}<CR><Esc>kkkk$hhhhhi
	imap w<Tab>		while ()<CR>{<CR><CR>}<CR><Esc>kkkk$i
	imap d<Tab>		do<CR>{<CR><CR>} while ();<CR><Esc>k$hi
	imap i<Tab>		if ()<CR>{<CR><CR>}<CR><Esc>kkkk$i
	imap e<Tab>		else<CR>{<CR>}<CR><Esc>kk$a<CR>
	imap ei<Tab>	else if ()<CR>{<CR><CR>}<CR><Esc>kkkk$i
	imap s<Tab>		struct <CR>{<CR>};<CR><Esc>kkk$a
	imap h<Tab>		static int (void)<CR>{<CR><CR>return 0;<CR><Backspace>}<CR><Esc>kkkkk$hhhhhi
	imap m<Tab>		int main(int argc, char *argv[])<CR>{<CR><CR><Tab>return 0;<CR><Backspace>}<Esc>kkk$a<CR><Tab><Esc>ggi#include <cavan.h><CR><Esc><C-o>a
	imap a<Tab>		.global main<CR><CR>main:<CR><Tab>stmfd sp!, {lr}<CR><CR>ldr r0, =string1<CR>bl printf<CR><CR><CR><CR>ldmfd sp!, {pc}<CR><CR>string1:<CR><Tab>.asciz "r1 = %#x, r2 = %#x, r3 = %#x\n"<CR>.align 2<CR><Esc><Esc>kkkkkkk$a<Tab>
	imap (<Tab>		()<CR>{<CR><CR>}<CR><Esc>kkkk$i
	imap {<Tab>		{<CR>}<CR><Esc>kk$a<CR>
	imap {}			{<CR>}<CR><Esc>kk$a<CR>
endfunction

function s:set_keymap_cpp()
	call s:set_keymap_c()
	imap f<Tab>		for (int i = 0; i < ; i++)<CR>{<CR><CR>}<CR><Esc>kkkk$hhhhhi
	imap c<Tab>		class <CR>{<CR><Backspace>private:<CR><CR><CR><Backspace>public:<CR><CR>};<CR><Esc>kkkkkkkk$a
endfunction

function s:set_keymap_asm()
	vmap //			:s/^\(\s*\)\(.\+\)/\1\/\/ \2/g<CR>:set nohls<CR>
	vmap \\			:s/^\(\s*\)\/\+\s*/\1/g<CR>:set nohls<CR>
	imap m<Tab>		.global main<CR><CR>main:<CR><Tab>stmfd sp!, {lr}<CR><CR>ldr r0, =string1<CR>bl printf<CR><CR><CR><CR>ldmfd sp!, {pc}<CR><CR>string1:<CR><Tab>.asciz "r1 = %#x, r2 = %#x, r3 = %#x\n"<CR>.align 2<CR><Esc><Esc>kkkkkkk$a<Tab>
endfunction

function s:set_keymap_vim()
	vmap //			:s/^\(\s*\)\(.\+\)/\1" \2/g<CR>:set nohls<CR>
	vmap \\			:s/^\(\s*\)"\+\s*/\1/g<CR>:set nohls<CR>
endfunction

function s:set_keymap_sh()
	vmap //			:s/^\(\s*\)\(.\+\)/\1# \2/g<CR>:set nohls<CR>
	vmap \\			:s/^\(\s*\)#\+\s*/\1/g<CR>:set nohls<CR>
endfunction

function s:set_keymap_python()
	vmap //			:s/^\(\s*\)\(.\+\)/\1# \2/g<CR>:set nohls<CR>
	vmap \\			:s/^\(\s*\)#\+\s*/\1/g<CR>:set nohls<CR>
endfunction

if has("autocmd")
	autocmd FileType c		call s:set_keymap_c()
	autocmd FileType cpp	call s:set_keymap_cpp()
	autocmd FileType asm	call s:set_keymap_asm()
	autocmd FileType vim	call s:set_keymap_vim()
	autocmd FileType sh		call s:set_keymap_sh()
	autocmd FileType python	call s:set_keymap_python()
endif

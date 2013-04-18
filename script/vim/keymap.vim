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
	map \\				:'<,'>s/^\(\s*\)\/\/\+\s*/\1/g<CR>:set nohls<CR>
	vmap /				:s/^\(\s*\)\(.\+\)/\1\/\/ \2/g<CR>:set nohls<CR>
	vmap \\				:s/^\(\s*\)\/\/\+\s*/\1/g<CR>:set nohls<CR>
	imap (<Tab>			()<CR>{<CR>}<CR><Esc>kkk$i
	imap {<Tab>			{<CR>}<Esc>k$a<CR>
	imap {}				{<Tab>
	imap for<Tab>		for (i = 0; i < ; i++)<CR>{<CR>}<Esc>kk$hhhhhi
	imap f<Tab>			for<Tab>
	imap while<Tab>		while (<Tab>
	imap w<Tab>			while<Tab>
	imap dw<Tab>		do<CR>{<CR><CR>} while ();<Esc>$hi
	imap d<Tab>			dw<Tab>
	imap do<Tab>		dw<Tab>
	imap if<Tab>		if (<Tab>
	imap i<Tab>			if<Tab>
	imap else<Tab>		else<CR>{<Tab>
	imap e<Tab>			else<Tab>
	imap ei<Tab>		else if<Tab>
	imap struct<Tab>	struct <CR>{<CR>};<CR><Esc>kkk$a
	imap s<Tab>			struct<Tab>
	imap enum<Tab>		enum <CR>{<CR>};<CR><Esc>kkk$a
	imap en<Tab>		enum<Tab>
	imap union<Tab>		union <CR>{<CR>};<CR><Esc>kkk$a
	imap u<Tab>			union<Tab>
	imap h<Tab>			static int (void)<CR>{<CR><CR>return 0;<CR><Backspace>}<CR><Esc>kkkkk$hhhhhi
	imap m<Tab>			int main(int argc, char *argv[])<CR>{<CR><CR>return 0;<CR><Backspace>}<Esc>ggi#include <cavan.h><CR><CR><Esc><C-o>kkk$a<CR>
	imap in<Tab>		#include <><Left>
	imap de<Tab>		#define<Space>
	imap ret<Tab>		return ;<Left>
	imap r<Tab>			ret<Tab>
	imap b<Tab>			break;
	imap c<Tab>			continue;
endfunction

function s:set_keymap_cpp()
	call s:set_keymap_c()
	imap f<Tab>		for (int i = 0; i < ; i++)<CR>{<CR><CR>}<CR><Esc>kkkk$hhhhhi
	imap c<Tab>		class <CR>{<CR><Backspace>private:<CR><CR><CR><Backspace>public:<CR><CR>};<CR><Esc>kkkkkkkk$a
endfunction

function s:set_keymap_java()
	call s:set_keymap_cpp()
	imap f<Tab>		for (int i = 0; i < ; i++)<CR>{<CR><CR>}<CR><Esc>kkkk$hhhhhi
	imap c<Tab>		public class <CR>{<CR><CR>}<CR><Esc>kkkk$a
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
	map \\			:'<,'>s/^\(\s*\)#\+\s*/\1/g<CR>:set nohls<CR>
	vmap /			:s/^\(\s*\)\(.\+\)/\1# \2/g<CR>:set nohls<CR>
	vmap \\			:s/^\(\s*\)#\+\s*/\1/g<CR>:set nohls<CR>
endfunction

function s:set_keymap_python()
	map \\			:'<,'>s/^\(\s*\)#\+\s*/\1/g<CR>:set nohls<CR>
	vmap /			:s/^\(\s*\)\(.\+\)/\1# \2/g<CR>:set nohls<CR>
	vmap \\			:s/^\(\s*\)#\+\s*/\1/g<CR>:set nohls<CR>
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

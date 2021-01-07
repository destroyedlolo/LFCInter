# MakeFile pour compiler LFCInter avec GCC
# LF: 29/04/1996 Unix est 'case sensitif' contrairement à l'Amiga...
# LF: 30/09/2000 Modif to make it fully UNIX compliant.

#opts = -DDEBUG
opts=-Wall -W

LFCinter : LFCInter.o Token.o LFCI_Lex.o LFCI_Var.o LFCI_fnc.o LFCI_Cal.o LFCI_icl.o LFCI_exe.o LFCI_ver.cxx
	gcc -s -o LFCInter LFCInter.o Token.o LFCI_Lex.o LFCI_Var.o LFCI_fnc.o LFCI_Cal.o LFCI_icl.o LFCI_exe.o LFCI_ver.cxx -lstdc++ $(opts)

LFCInter.o : LFCInter.cxx Token.h LFCInter.h LFCI_Cal.h LFDStack.h
	gcc $(opts) -c -o LFCInter.o -c LFCInter.cxx

Token.o : Token.cxx Token.h LFCInter.h
	gcc $(opts) -c -o Token.o -c Token.cxx

LFCI_Lex.o : LFCI_Lex.cxx LFCI_Cal.h Token.h LFCInter.h
	gcc $(opts) -c -o LFCI_Lex.o -c LFCI_Lex.cxx

LFCI_Var.o : LFCI_Var.cxx LFCInter.h LFCI_Cal.h
	gcc $(opts) -c -o LFCI_Var.o -c LFCI_Var.cxx

LFCI_fnc.o : LFCI_fnc.cxx LFCI_Cal.h LFCInter.h LFDStack.h Token.h
	gcc $(opts) -c -o LFCI_fnc.o -c LFCI_fnc.cxx

LFCI_exe.o : LFCI_exe.cxx LFCI_Cal.h LFCInter.h LFDStack.h Token.h
	gcc $(opts) -c -o LFCI_exe.o -c LFCI_exe.cxx

LFCI_Cal.o : LFCI_Cal.cxx LFCI_Cal.h LFCInter.h LFDStack.h Token.h LFCI_icl.h
	gcc $(opts) -c -o LFCI_Cal.o -c LFCI_Cal.cxx

LFCI_icl.o : LFCI_icl.cxx LFCI_Cal.h LFCInter.h LFDStack.h Token.h LFCI_icl.h
	gcc $(opts) -c -o LFCI_icl.o -c LFCI_icl.cxx

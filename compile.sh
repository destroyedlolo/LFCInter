#                            Compile.sh
#                           © LFSoft 1996
#
# Make de GCC plante mon 1000. Ce script permet de compiler de manière
# sélective les éléments de LFCInter.
#   Pour CSH...
#

local opts

#options de compilation
set opts "-DDEBUG"

#il faut que la compilations s'arrete à la premiere erreur
set _failat 1

#GCC ne remet pas la priorité du shell à 0
set _every "pri 0 0; echo >speak: \"the end\";unset _every"

if -t t:LFCInter.o LFCInter.cxx Token.h LFCInter.h LFCI_Cal.h LFDStack.h
    echo "Compile: lfcinter"
    g++ -o /t/LFCInter.o -c LFCInter.cxx $opts
endif

if -t t:Token.o Token.cxx Token.h LFCInter.h
    echo "Compile: token"
    g++ -o /t/Token.o -c Token.cxx $opts
endif

if -t t:LFCI_Lex.o LFCI_Lex.cxx LFCI_Cal.h Token.h LFCInter.h
    echo "Compile: lfci_lex"
    g++ -o /t/LFCI_Lex.o -c LFCI_Lex.cxx $opts
endif

if -t t:LFCI_Var.o LFCI_Var.cxx LFCInter.h LFCI_Cal.h
    echo "Compile: lfci_var"
    g++ -o /t/LFCI_Var.o -c LFCI_Var.cxx $opts
endif

if -t t:LFCI_fnc.o LFCI_fnc.cxx LFCI_Cal.h LFCInter.h LFDStack.h Token.h
    echo "Compile: lfci_fnc"
    g++ -o /t/LFCI_fnc.o -c LFCI_fnc.cxx $opts
endif

if -t t:LFCI_exe.o LFCI_exe.cxx LFCI_Cal.h LFCInter.h LFDStack.h Token.h
    echo "Compile: lfci_cal"
    g++ -o /t/LFCI_exe.o -c LFCI_exe.cxx $opts
endif

if -t t:LFCI_Cal.o LFCI_Cal.cxx LFCI_Cal.h LFCInter.h LFDStack.h Token.h LFCI_icl.h
    echo "Compile: lfci_cal"
    g++ -o /t/LFCI_Cal.o -c LFCI_Cal.cxx $opts
endif

if -t t:LFCI_icl.o LFCI_icl.cxx LFCI_Cal.h LFCInter.h LFDStack.h Token.h LFCI_icl.h
    echo "Compile: lfci_icl"
    g++ -o /t/LFCI_icl.o -c LFCI_icl.cxx $opts
endif

if -t LFCinter t:lfcinter.o t:token.o t:LFCI_Lex.o t:lfci_var.o t:lfci_fnc.o t:lfci_cal.o t:lfci_icl.o t:LFCI_exe.o lfci_ver.cxx
    echo "Compile: Exec"
    g++ -o LFCInter /t/LFCInter.o /t/Token.o /t/LFCI_Lex.o /t/LFCI_Var.o /t/LFCI_fnc.o /t/LFCI_Cal.o /t/LFCI_icl.o /t/LFCI_exe.o LFCI_ver.cxx $opts
endif

set _failat 20

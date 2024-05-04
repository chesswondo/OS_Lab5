run: main.exe func.exe
    main.exe 5 7

func.exe: func.cpp
    cl /O2 /Ot /Oi /Ox /Oy /Ob2 /EHsc /FC /W4 /nologo /MT func.cpp

main.exe: main.cpp
    cl /O2 /Ot /Oi /Ox /Oy /Ob2 /EHsc /FC /W4 /nologo /MT main.cpp

test: myShell.cpp myShell.h
	g++ -o test -ggdb3 -Wall -Werror -pedantic -std=gnu++98 myShell.cpp myShell.h

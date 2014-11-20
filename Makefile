
all:
	cl65 -Osir -o synthmark64.prg main.c helper.s

clean:
	rm -f *~
	rm -f *.o
	rm -f *.prg
all: fpgrowth.o fptree.o tract.o
	icc fpgrowth.o fptree.o tract.o -qopenmp -lm -o fpgrowth
fpgrowth.o: fpgrowth.c tract.h fptree.h
	icc -qopenmp -c fpgrowth.c
fptree.o: fptree.c fptree.h tract.h
	icc -qopenmp -c fptree.c
tract.o: tract.c tract.h
	icc -qopenmp -c tract.c
clean:
	rm -f *.o fpgrowth
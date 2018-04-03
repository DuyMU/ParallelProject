all: fpgrowth.o fptree.o tract.o
	gcc fpgrowth.o fptree.o tract.o -fopenmp -lm -o fpgrowth
fpgrowth.o: fpgrowth.c tract.h fptree.h
	gcc -fopenmp -c fpgrowth.c
fptree.o: fptree.c fptree.h tract.h
	gcc -fopenmp -c fptree.c
tract.o: tract.c tract.h
	gcc -fopenmp -c tract.c
clean:
	rm -f *.o fpgrowth

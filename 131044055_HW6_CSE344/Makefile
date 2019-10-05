all: pCp

pCp: hw6.o -lpthread
	gcc hw6.o -lpthread -o pCp

hw6.o: hw6.c -lpthread
	gcc -c hw6.c -lpthread


clean:
	rm *.o pCp

all: banka musteri

banka: banka.o 
	gcc -o banka banka.o -lm

musteri: musteri.o
	gcc -o musteri musteri.o -lm


banka.o: banka.c
	gcc -c banka.c

musteri.o: musteri.c
	gcc -c musteri.c

clean:
	rm *.o *.fifo banka musteri; rm -rf log


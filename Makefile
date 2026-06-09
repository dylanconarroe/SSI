all: ssi

ssi: ssi.c
	gcc ssi.c -o ssi

clean:
	rm -f *.o
	rm -f ssi
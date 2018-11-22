CC=mpicc -fopenmp
CFLAGS=-lm

tec508-p3: main.o csv.o
	$(CC) -o tec508-p3 main.o csv.o $(CFLAGS)

clean:
	rm -f tec508-p3 main.o csv.o

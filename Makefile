CC=gcc
CFLAGS=-O3 -g
EXEC=louvain

all: $(EXEC)

louvain: partition.o louvain.o
	$(CC) -o louvain partition.o louvain.o $(CFLAGS)

clean:
	rm *.o louvain

%.o: %.c %.h
	$(CC) -o $@ -c $< $(CFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)
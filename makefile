CC = gcc

cache: cache.o
	$(CC) -g -o cache cache.o -lm
	
cache.o: cache.c
	$(CC) -c cache.c

clean:
	rm -f *.o *~ cache1
t:
	./cache1 2048 64 64 l

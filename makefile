CC = gcc
CFLAGS = -Wall

cache: cache.o
	$(CC) -o cache cache.o -lm
	
cache.o: cache.c
	$(CC) -c cache.c
	

clean:
	rm -f *.o *~ cache
test1:
	./cache 2048 64 64 l
test2:
	./P74XXXXXX -input trace/trace2.txt -output out2
test3:
	./P74XXXXXX -input trace/trace3.txt -output out3

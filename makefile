F74XXXXXX: 
	g++ P74XXXXXX.cpp -o P74XXXXXX
clean:
	rm P74XXXXXX
test1:
	./P74XXXXXX -input trace/trace1.txt -output out1
test2:
	./P74XXXXXX -input trace/trace2.txt -output out2
test3:
	./P74XXXXXX -input trace/trace3.txt -output out3

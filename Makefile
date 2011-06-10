# Makefile for VMWareComponentExtractor
# Nick Glynn (n.s.glynn@gmail.com)

BINARY:=VMWareComponentExtractor

default:
	$(CXX) VMWareComponentExtractor.cpp -o $(BINARY)

clean:
	rm $(BINARY) *.o 2>&1 1> /dev/null

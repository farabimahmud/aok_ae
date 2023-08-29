CXXFLAGS=-g -std=c++20 -gdwarf-4 -Wall -O0 -Wno-unused-function 

AES-key-extraction: AES-key-extraction.cpp
	g++ $(CXXFLAGS) -Wall -O3 -Wno-unused-function -o $@ $<


clean:
	rm -rf AES-key-extraction

headers = headers/common.h headers/cpu.h headers/pagemap.h headers/perf.h
CXXFLAGS=-g -std=c++20 -gdwarf-4 -Wall -O0 -Wno-unused-function -Wno-volatile


trace-generation: trace-generation.cpp
	g++ $(CXXFLAGS) -o $@ $< -lpthread


clean:
	rm -rf trace-generation
all: dce-pass.so

CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -g -O0

%.so: %.o 
	$(CXX) -dylib -flat_namespace -shared $^ -o $@

dce-pass.o: dce-pass.cpp dceAnalysis.h IDFA.h

clean:
	rm -f *.o *~ *.so

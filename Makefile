all: licm-pass.so

CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -g -O0

%.so: %.o 
	$(CXX) -dylib -flat_namespace -shared $^ -o $@

licm-pass.o: licm-pass.cpp

clean:
	rm -f *.o *~ *.so

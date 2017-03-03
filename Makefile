TOP=$(HOME)/genii/export
CPPFLAGS= #-I$(TOP)/include
CXXFLAGS=-g -Wall -g0 -O3
LDFLAGS=-lboost_thread-mt

prod_cons: prod_cons.o myrand.o
	$(LINK.cpp) -o $@ $^

clean:
	rm -f *.o prod_cons

myrand.o: myrand.h
prod_cons.o: myrand.h

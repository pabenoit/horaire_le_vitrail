CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g  -std=c++11
LDFLAGS=-g 
LDLIBS=

SRCS=horaire.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

horaire: $(OBJS)
	$(CXX) $(LDFLAGS) -o horaire $(OBJS) $(LDLIBS) 


horaire.o: horaire.cpp horaire.hpp
	$(CXX) $(CPPFLAGS) -c horaire.cpp



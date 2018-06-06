CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g  -std=c++11
LDFLAGS=-g 
LDLIBS=

SRCS=Horaire.cpp Configuration.cpp Cour.cpp Prof.cpp LocalPool.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

horaire: $(OBJS)
	$(CXX) $(LDFLAGS) -o horaire $(OBJS) $(LDLIBS) 


$(OBJS): $(SRCS)
	$(CXX) $(CPPFLAGS) -c $(SRCS)



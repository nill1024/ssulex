
CC = g++
CFLAGS = -std=c++17

BDIR = bin
ODIR = ../obj
IDIR = ..

LIBS = -lavformat -lavcodec

all : indexing

% : %.cpp $(ODIR)/%.o
	$(CC) $(CFLAGS) -o $(BDIR)/$@ $@.cpp $(ODIR)/$@.o -I$(IDIR) $(LIBS)


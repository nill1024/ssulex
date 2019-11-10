
CC = g++
CFLAGS = -std=c++17 -g

BDIR = bin
ODIR = ../obj
IDIR = ..

_LIBS = libavutil libavcodec libavformat libavfilter
LIBS = $(shell pkg-config --libs --cflags $(_LIBS))

_OBJ = indexing seek_remux
OBJ = $(patsubst %,$(BDIR)/%,$(_OBJ))

all : $(BDIR) $(OBJ)

$(BDIR):
	mkdir $@

$(BDIR)/% : %.cpp $(shell find $(ODIR) -name '*.o')
	$(CC) $(CFLAGS) -I$(IDIR) -o $@ $< $(shell find $(ODIR) -name '*.o') $(LIBS)



CC = g++
CFLAGS = -std=c++17 -g

BDIR = bin
ODIR = ../obj
IDIR = .. ../third-party
INCLUDE = $(patsubst %,-I%,$(IDIR))

_PKG_LIBS = libavutil libavcodec libavformat libavfilter gumbo
PKG_LIBS = $(shell pkg-config --libs --cflags $(_PKG_LIBS))

LIBS = -lgq

_OBJ = indexing seek_remux parse_imdb
OBJ = $(patsubst %,$(BDIR)/%,$(_OBJ))

all : $(BDIR) $(OBJ)

$(BDIR):
	mkdir $@

$(BDIR)/% : %.cpp $(shell find $(ODIR) -name '*.o')
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $< $(shell find $(ODIR) -name '*.o') $(PKG_LIBS) $(LIBS)


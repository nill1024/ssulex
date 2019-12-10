
CC = g++
CFLAGS = -std=c++17 -g

BDIR = bin
ODIR = ../obj
IDIR = .. ../third-party

INCLUDE = $(patsubst %,-I%,$(IDIR))

_PKG_LIBS = libavutil libavcodec libavformat libavfilter gumbo
PKG_LIBS = $(shell pkg-config --libs --cflags $(_PKG_LIBS))

LIBS = -lgq -lpthread -lstdc++fs

_OBJ = common.o media.o metadata_parser.o make_index.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_BIN = indexing seek_remux parse_imdb
BIN = $(patsubst %,$(BDIR)/%,$(_BIN))

all : $(BDIR) $(BIN)

$(BDIR):
	mkdir $@

$(BDIR)/% : %.cpp $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $< $(OBJ) $(PKG_LIBS) $(LIBS)

clean :
	bash -c "if [[ -d $(BDIR) ]]; then rm -r $(BDIR); fi"


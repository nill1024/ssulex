
CC = g++
CFLAGS = -std=c++17 -g

BDIR = bin
ODIR = obj
IDIR = . ./third-party

INCLUDE = $(patsubst %,-I%,$(IDIR))

_OBJ = common.o media.o metadata_parser.o main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_PKG_LIBS = libavutil libavcodec libavformat libavfilter gumbo
PKG_LIBS = $(shell pkg-config --libs --cflags $(_PKG_LIBS))

LIBS = -lgq -lpthread

$(BDIR)/ssulex : $(ODIR) $(OBJ) $(BDIR)
	$(CC) -o $@ $(OBJ) $(PKG_LIBS) $(LIBS)

$(ODIR) :
	mkdir $@

$(BDIR) :
	mkdir $@

$(ODIR)/%.o : %.cpp %.hpp
	$(CC) $(CFLAGS) -c -o $@ $< $(INCLUDE)

clean :
	bash -c "if [[ -d $(ODIR) ]]; then rm -r $(ODIR); fi"
	bash -c "if [[ -d $(BDIR) ]]; then rm -r $(BDIR); fi"


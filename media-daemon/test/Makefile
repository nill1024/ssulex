
CC = gcc

BDER = ./bin
DDER = ../obj
IDER = ../
LIBS = -lavformat -lavcodec

_DEP = indexing.o
DEP = $(patsubst %,$(DDER)/%,$(_DEP))

all :

% : %.c $(DEP)
	$(CC) -o $(BDER)/$@ $< $(DEP) -I$(IDER) $(LIBS)


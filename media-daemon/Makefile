
CC = gcc

ODIR = obj

_OBJ = indexing.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all : $(OBJ)

$(ODIR)/%.o : %.c %.h
	$(CC) -c -o $@ $<


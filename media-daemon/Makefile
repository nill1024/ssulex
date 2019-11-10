
CC = g++
CFLAGS = -std=c++17 -g

ODIR = obj
IDIR = .

_OBJ = indexing.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all : $(ODIR) $(OBJ)

$(ODIR) :
	mkdir $@

$(ODIR)/%.o : %.cpp %.hpp
	$(CC) $(CFLAGS) -c -o $@ $< -I$(IDIR)


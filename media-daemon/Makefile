
CC = g++
CFLAGS = -std=c++17

ODIR = obj
IDIR = .

_OBJ = indexing.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all : $(OBJ)

$(ODIR)/%.o : %.cpp %.hpp
	$(CC) $(CFLAGS) -c -o $@ $< -I$(IDIR)


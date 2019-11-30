
CC = g++
CFLAGS = -std=c++17 -g

ODIR = obj
IDIR = . ./third-party
INCLUDE = $(patsubst %,-I%,$(IDIR))

_OBJ = common.o indexing.o metadata_parser.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all : $(ODIR) $(OBJ)

$(ODIR) :
	mkdir $@

$(ODIR)/%.o : %.cpp %.hpp
	$(CC) $(CFLAGS) -c -o $@ $< $(INCLUDE)

clean :
	bash -c "if [[ -d $(ODIR) ]]; then rm -r $(ODIR); fi"


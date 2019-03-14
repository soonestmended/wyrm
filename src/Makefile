IDIR =../include
CC=g++
CFLAGS=-I$(IDIR) -g

ODIR =../obj
LDIR =../lib
BDIR =../bin

LIBS=

_DEPS = Light.hpp Material.hpp Mesh.hpp Primitive.hpp Ray.hpp Scene.hpp
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = Scene.o test.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

wyrm: $(OBJ)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
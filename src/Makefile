IDIR =../include
CC=g++
CFLAGS=-I$(IDIR) -g

ODIR =../obj
LDIR =../lib
BDIR =../bin

LIBS=

_DEPS = BBox.hpp Camera.hpp Color.hpp Film.hpp IntersectRec.hpp Light.hpp Material.hpp Mesh.hpp Primitive.hpp Ray.hpp Scene.hpp Tracer.hpp
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = Mesh.o Primitive.o Ray.o Scene.o Tracer.o test.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

wyrm: $(OBJ)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
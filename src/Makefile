IDIR =../include
CC=g++
CFLAGS=-I$(IDIR) -g

ODIR =../obj
LDIR =../lib
BDIR =../bin

LIBS=boost_program_options

_DEPS = BBox.hpp Camera.hpp Color.hpp IntersectRec.hpp Light.hpp Material.hpp Mesh.hpp Primitive.hpp Ray.hpp Renderer.hpp Scene.hpp Tracer.hpp
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = Film.o Mesh.o Primitive.o Ray.o Scene.o Tracer.o test.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -l $(LIBS) -o $@ $< $(CFLAGS)

wyrm: $(OBJ)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS) -l $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
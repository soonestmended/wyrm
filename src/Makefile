IDIR =../include
CC=g++ -std=c++17
CFLAGS=-I$(IDIR) -I/usr/local/include -g

ODIR =../obj
LDIR =../lib
BDIR =../bin

LIBS=-lboost_program_options -lpng

_DEPS = Accelerator.hpp BBox.hpp BVH.hpp Camera.hpp Color.hpp Image.hpp IntersectRec.hpp Light.hpp Material.hpp Mesh.hpp Primitive.hpp Ray.hpp Renderer.hpp Scene.hpp Shader.hpp Tracer.hpp
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = BBox.o BVH.o Image.o Material.o Mesh.o Primitive.o Ray.o Renderer.o Scene.o Shader.o Tracer.o test.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c $(LIBS) -o $@ $< $(CFLAGS)

wyrm: $(OBJ)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 

IDIR =../include
CC=g++ -std=c++17
CFLAGS=-I$(IDIR) -I/usr/local/include -g

ODIR =../obj
LDIR =../lib
BDIR =../bin

LIBS=-lboost_program_options -lpng -lpthread

_DEPS = Accelerator.hpp BBox.hpp BVH.hpp BxDF.hpp Camera.hpp Color.hpp common.hpp Image.hpp IntersectRec.hpp Light.hpp Material.hpp Mesh.hpp ONB.hpp Parser.hpp Primitive.hpp Ray.hpp Renderer.hpp Scene.hpp Tracer.hpp Utils.hpp VisibilityTester.hpp XForm.hpp
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = ADMaterial.o BBox.o BVH.o BxDF.o Image.o Light.o Material.o Mesh.o Parser.o Primitive.o Ray.o Renderer.o Scene.o Tracer.o test.o Utils.o VisibilityTester.o pugixml.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c $(LIBS) -o $@ $< $(CFLAGS)

wyrm: $(OBJ)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 

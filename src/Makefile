IDIR =../include
CC=g++ -std=c++17
CFLAGS=-I$(IDIR) -I/usr/local/include -g -O3

ODIR =../obj
LDIR =../lib
BDIR =../bin

LIBS=-lpng -lpthread

_DEPS = Accelerator.hpp BBox.hpp BVH.hpp BxDF.hpp Camera.hpp Color.hpp common.hpp Distribution.hpp Image.hpp IntersectRec.hpp Light.hpp Material.hpp Mesh.hpp ONB.hpp Parser.hpp PBRTParser.hpp Primitive.hpp Ray.hpp Renderer.hpp RenderPackage.hpp SampleGenerator.hpp Scene.hpp Texture.hpp tinyexr.h Tracer.hpp Utils.hpp VisibilityTester.hpp XForm.hpp
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = ADMaterial.o BBox.o BVH.o BxDF.o Image.o Light.o Material.o Mesh.o Parser.o PBRTParser.o Primitive.o Ray.o Renderer.o RenderPackage.o SampleGenerator.o Scene.o Texture.o Tracer.o test.o Utils.o VisibilityTester.o pugixml.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c $(LIBS) -o $@ $< $(CFLAGS)

wyrm: $(OBJ)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 

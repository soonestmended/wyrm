#pragma once

/* GLM */
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdlib.h>
#include <vector>

#include "Accelerator.hpp"
#include "BBox.hpp"
#include "Primitive.hpp"
class Scene;
/*
typedef struct _clBVHNode {
	union {
		cl_uint4 ptr;
		cl_Real4 v0;
	};
	union {
		cl_uint4 numFaces;
		cl_Real4 v1;
	};
} clBVHNode;
*/
typedef struct _XSP {
	Real pos;
	int axis;
} SP;	

class BVHStats {
public:
	BVHStats() {
		numNodes = 0;
	}

	int numNodes;
	
};


class BVHNode {
	
public:
	BVHNode() {
		bbox = BBox();
		ptr = 0;
		flags = numPrimitives = 0;
	}
	
	BVHNode(const BBox &_bbox) : bbox (_bbox) {
		ptr = 0;
		flags = numPrimitives = 0;
	}

	BVHNode(const unsigned int _faces, const unsigned short _numPrimitives) : ptr (_faces), numPrimitives (_numPrimitives) {
		flags = 0;
	}
	
	void setLeaf() {
		flags = 1;
	}
	
	bool isLeaf() const {
		return flags == 1;
	}
	
	BBox bbox;
	unsigned int ptr; // or index array offset
	unsigned int flags;
	unsigned int numPrimitives;

};

class BVH : public Accelerator {
public:
	
	BVH(const Scene &_s) : Accelerator (_s) {
		built = false;
		nextAllocedNode = 1;
        if (!build()) {
          std::cout << "Error building BVH." << std::endl;
          exit(1);
        }
        else {
          std::cout << "BVH build complete." << std::endl;
        }
	}
  
  bool build();
  bool closestIntersection(const Ray& ray, const Real tmin, const Real tmax, IntersectRec& ans) const;
  bool intersectionYN(const Ray& ray, const Real tmin, const Real tmax) const;
  void print() const;
    
private:
	unsigned int nextNewNode();
	Vec3 getCentroid(const unsigned int face);
	SP findSplitPlane(const BVHNode &node);
	void buildBelow(BVHNode &node, int depth);
	//void makeCLNodes(clBVHNode *&vec);
	
	std::vector<BVHNode> nodes;
	std::vector <std::shared_ptr<Primitive>> primitives;
	std::vector <int> primitiveIndices;
	unsigned int nextAllocedNode;
	std::vector <BBox> bboxes;
	std::vector <Vec3> centroids;
	BBox bboxOfCentroids;
	BVHStats bvhstats;
	bool built;
};

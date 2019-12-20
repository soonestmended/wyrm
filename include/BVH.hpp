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
		cl_float4 v0;
	};
	union {
		cl_uint4 numFaces;
		cl_float4 v1;
	};
} clBVHNode;
*/
typedef struct _XSP {
	float pos;
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
	
	const bool isLeaf() const {
		return flags == 1;
	}
	
	BBox bbox;
	uint ptr; // or index array offset
	unsigned int flags;
	unsigned int numPrimitives;

};

class BVH : public Accelerator {
public:
	
	BVH(const Scene &_s) : Accelerator (_s) {
		built = false;
		nextAllocedNode = 1;
	}

	const bool build();
	const bool closestIntersection(const Ray& ray, const float tmin, const float tmax, IntersectRec& ans) const;
    const bool intersectionYN(const Ray& ray, const float tmin, const float tmax) const;
    
private:
	uint nextNewNode();
	glm::vec3 getCentroid(const unsigned int face);
	SP findSplitPlane(const BVHNode &node);
	void buildBelow(BVHNode &node, int depth);
	//void makeCLNodes(clBVHNode *&vec);
	
	std::vector<BVHNode> nodes;
	std::vector <std::shared_ptr<Primitive>> primitives;
	std::vector <int> primitiveIndices;
	uint nextAllocedNode;
	std::vector <BBox> bboxes;
	std::vector <glm::vec3> centroids;
	BBox bboxOfCentroids;
	BVHStats bvhstats;
	bool built;
};
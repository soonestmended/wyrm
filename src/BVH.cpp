#include "BVH.hpp"
#include "common.hpp"
#include "Scene.hpp"
#include <stdlib.h>
#include <vector>

using namespace std;

/*
void BVH::makeCLNodes(clBVHNode *&vec) {
	vec = new clBVHNode[nextAllocedNode];
	for (int i = 0; i < nextAllocedNode; i++) {
		BVHNode *node = &(root[i]);
		
		vec[i].v0.x = node->bbox.v0[0];
		vec[i].v0.y = node->bbox.v0[1];
		vec[i].v0.z = node->bbox.v0[2];
		vec[i].v1.x = node->bbox.v1[0];
		vec[i].v1.y = node->bbox.v1[1];
		vec[i].v1.z = node->bbox.v1[2];

		vec[i].ptr.w = node->ptr;
		vec[i].numFaces.w = node->numFaces;
		vec[i].numFaces.w |= (node->isLeaf()) << 31;
		
	}
}
*/	

const bool BVH::build() {

	primitives = scene.getPrimitives();
	int numPrimitives = primitives.size();
	nodes.reserve(2*numPrimitives-1);
	primitiveIndices.reserve(numPrimitives);
	//root = new BVHNode[2*numFaces-1]; // max possible nodes needed
	
	//bboxes = new BBox[numFaces];
	bboxes.reserve(numPrimitives);

	// centroids = new glm::vec3[numFaces];
	centroids.reserve(numPrimitives);

	for (int i = 0; i < numPrimitives; i++) {
/*
		glm::vec3 p, q, r;
		int v0ei = i*3; // v0 element index
		p = s->vertices[s->elements[v0ei]];
		q = s->vertices[s->elements[v0ei+1]];
		r = s->vertices[s->elements[v0ei+2]];
		bboxes[i].enclose(p);
		bboxes[i].enclose(q);
		bboxes[i].enclose(r);
		*/
		primitiveIndices[i] = i;
		bboxes.push_back(primitives[i]->getBBox());
		centroids.push_back(primitives[i]->getBBox().getCentroid());
	}
	
	for (int i = 0; i < numPrimitives; i++) {
		bboxOfCentroids.enclose(centroids[i]);
	}
	
	nodes[0].setLeaf();
	nodes[0].ptr = 0; // index to primitives array
	nodes[0].numPrimitives = numPrimitives;
	nodes[0].bbox = scene.getBBox();
	
	buildBelow(nodes[0], 0);
	
	if (!nodes[0].isLeaf())
		nodes[0].ptr = nextAllocedNode;
	
//	delete[] bboxes;
//	delete[] centroids;
	return true;
}

uint BVH::nextNewNode() {
	return nextAllocedNode++;
}

float surfaceArea(const BBox &bbox) {
	float width = bbox.max[0] - bbox.min[0];
	float height = bbox.max[1] - bbox.min[1];
	float depth = bbox.max[2] - bbox.min[2];
	return 2.0*width*height + 2.0*width*depth + 2.0* height*depth;
}

SP BVH::findSplitPlane(const BVHNode& node) {
	// test N bins
#define N 8

	// find axis
	int axis;

// FROM HERE
	
	// get bounding box of centroids for this node
	BBox cb;
	for (int i = 0; i < node.numPrimitives; i++) {
		//cb.enclose(centroids[node->faces[i]]);
		cb.enclose(centroids[primitiveIndices[node.ptr+i]]);
	}

	glm::vec3 t = cb.max - cb.min;
	/*	
	float t0, t1, t2; // sizes of centroid bounds in diff axes
	t0 = cb.v1[0] - cb.v0[0];
	t1 = cb.v1[1] - cb.v0[1];
	t2 = cb.v1[2] - cb.v0[2];
	*/
	if (t[0] > t[1]) {
		if (t[0] > t[2]) {
			axis = 0;
		}
		else {
			axis = 2;
		}
	} else {
		if (t[1] > t[2]) {
			axis = 1;
		}
		else {
			axis = 2;
		}
	}
#define LOCAL_EPSILON .0000001f
	// precompute 
	if (t[axis] < LOCAL_EPSILON) {
		SP sp;
		sp.axis = -1;
		return sp;
	}
	
	float k1 = (N*(1.0f-LOCAL_EPSILON)) / (t[axis]);
	float k0 = cb.min[axis];
	

	int numIn[N]; // number of primitives in bin i
	for (int i = 0; i < N; i++) {
		numIn[i] = 0;
	}	
	
	// evaluate N different splitting planes
	BBox binBounds[N]; 
	BBox leftBounds, rightBounds;
	int numLeft[N-1];
	float SAleft[N-1];
	
		// compute bin IDs
	for (int i = 0; i < node.numPrimitives; i++) {
		int v0ei = primitiveIndices[node.ptr+i];
		int v0eiBin = (int) (k1 * (centroids[v0ei][axis] - k0 ));
		numIn[v0eiBin]++;
		binBounds[v0eiBin].enclose(bboxes[v0ei]);
	}
		
	numLeft[0] = numIn[0];
	SAleft[0] = surfaceArea(binBounds[0]);
	leftBounds.enclose(binBounds[0]);
	
	// test each plane
	for (int i = 1; i < N; i++) {
		// bin i is the highest numbered bin to the left of plane i
		numLeft[i] = numLeft[i-1] + numIn[i];
		leftBounds.enclose(binBounds[i]);
		SAleft[i] = surfaceArea(leftBounds);
	}
	
	// compute costs
	float cost[N-1];
	rightBounds.enclose(binBounds[N-1]);
	int numRight = numIn[N-1];
	cost[N-2] = numLeft[N-2]*SAleft[N-2] + (numRight*surfaceArea(rightBounds));
	
	for (int i = N-3; i >= 0; i--) { // loop over N-1 planes
	//	Cost j = ALj*NLj + ARj*NRj
		rightBounds.enclose(binBounds[i+1]);
		numRight += numIn[i+1];
		cost[i] = numLeft[i]*SAleft[i] + numRight*surfaceArea(rightBounds);
	}
	
	SP sp;
	sp.axis = axis;
	float bestCost = POS_INF;
	// find minimum cost
	int bestPlane = -1;
	for (int i = 0; i < N-1; i++) {
		if (cost[i] < bestCost) {
			bestCost = cost[i];
			bestPlane = i;
			sp.pos = cb.min[axis] + ((float)(i+1)*(t[axis]) / N);
		}
	}
	
	//printf("Cost: ");
	//for (int i = 0; i < N-1; i++) printf("%d, ", (int) cost[i]);
	//printf("\nNumIn: ");
	//for (int i = 0; i < N; i++) printf("%d, ", numIn[i]);
	//printf("\nBest cost: %f", bestCost);
	//printf("\tsp.pos: %f", sp.pos);
	//printf("\nBest plane: %d\tnumLeft: %d / %d\n", bestPlane, numLeft[bestPlane], node->numFaces);
	
	return sp;
}	

void advance_cursor() {
  static int pos=0;
  char cursor[4]={'/','-','\\','|'};
  printf("%c\b", cursor[pos]);
  fflush(stdout);
  pos = (pos+1) % 4;
}

void BVH::buildBelow(BVHNode &node, int depth) {
	
	//if (node->numFaces == 0)	
		//cout << "atDepth: " << depth << "\tnumFaces: " << node->numFaces << endl;
	
	if (!node.isLeaf()) {
		printf("Error - splitting internal node\n");
		exit(1);
	}
	//advance_cursor();
	/*
	float xSize = node->bbox.v1[0] - node->bbox.v0[0];
	float ySize = node->bbox.v1[1] - node->bbox.v0[1];
	float zSize = node->bbox.v1[2] - node->bbox.v0[2];
	*/
	glm::vec3 bboxSize = node.bbox.max - node.bbox.min;
	if (depth > 25 || node.numPrimitives < 4 || glm::length(bboxSize) < .0001) {
		//cout << "Leaf node with " << node.numPrimitives << " prims. \n";
		fflush(stdout);
		advance_cursor();
		return;
	}
				
	SP sp = findSplitPlane(node);
	//cout << node->bbox.v0[sp.axis] << " - " << node->bbox.v1[sp.axis] << endl;
	//cout << sp.pos << endl;
	
	if (sp.axis == -1) {
		// split failed.
		return;
	}
	
	node.flags = 0; // set not a leaf
	int le = node.ptr; int re = le + node.numPrimitives-1;
	int lp = le; int rp = re;

	//printf("Begin in place sort of faces[%d - %d]...\n", le, re);
	do {
    while (bboxes[primitiveIndices[lp]].getCentroid()[sp.axis] <= sp.pos && lp!=rp) lp++;
    while (bboxes[primitiveIndices[rp]].getCentroid()[sp.axis] >= sp.pos && lp!=rp) rp--;
    if (lp!=rp) {
			uint tmp = primitiveIndices[lp];
			primitiveIndices[lp] = primitiveIndices[rp];
			primitiveIndices[rp] = tmp;
		}    
		else {
			//printf("in place sort done with lp = %d, rp = %d\n", lp, rp);
			break;
		}
	} while (true);
	
	
	
	//printf("Checking sort... ");
	//for (int i = le; i < re+1; i++) {
//		if (bboxes[faces[i]].center()[sp.axis] < sp.pos) printf("-");
//		else printf("+");
//	}
	
	// now this subarray is sorted
	
	// make left child, which is node+1
	BVHNode &leftChild = nodes[nextNewNode()];

	unsigned int nodePrimitives = node.ptr; // first face of this node, will become first face of left child node
	node.numPrimitives = 0; // mark internal node
//	node->leftChild = nextNewNode();
	
//	root[node->leftChild].leftChild = nodeFaces;
//	root[node->leftChild].numFaces = lp - nodeFaces-1;
	
	leftChild.ptr = nodePrimitives; // leftChild is a leaf and has faces on the left side of this node's faces
	leftChild.numPrimitives = lp-nodePrimitives;
	leftChild.setLeaf();
	for (int i = leftChild.ptr; i < leftChild.ptr+leftChild.numPrimitives; i++) {
		leftChild.bbox.enclose(bboxes[primitiveIndices[i]]);
	}
	
		
	node.ptr = 999999;
	
	buildBelow(leftChild, depth+1);

	BVHNode &rightChild = nodes[nextNewNode()];
	
	if (!leftChild.isLeaf()) leftChild.ptr = nextAllocedNode-1; // "skip" value
	
	rightChild.ptr = lp;
	rightChild.numPrimitives = re - lp + 1; 
	rightChild.setLeaf();
	for (int i = rightChild.ptr; i < rightChild.ptr+rightChild.numPrimitives; i++) {
		rightChild.bbox.enclose(bboxes[primitiveIndices[i]]);
	}
	buildBelow(rightChild, depth+1);
	if (!rightChild.isLeaf()) {
		rightChild.ptr = nextAllocedNode;
	}
}

const bool BVH::closestIntersection(const Ray& ray, const float tmin, const float tmax, IntersectRec &ans) const {
	ans.t = POS_INF; // ans->u = -1.0f;
	bool hit = false;
	IntersectRec tempIr;
	int curPos = 0;
	while (curPos < nextAllocedNode) {
		
		//if (rayIntersectsBBox(ray, nodes[curPos].v0, nodes[curPos].v1)) {
		if (nodes[curPos].bbox.intersectYN(ray, tmin, tmax)) {		
			//if (nodes[curPos].numFaces > 0) { // if so, this is a leaf
			//if (nodes[curPos].numFaces.w >> 31 == 1) {
			if (nodes[curPos].isLeaf()) {
				//return true;
				uint ind;
				float p, q, r;
				for (int i = 0; i < nodes[curPos].numPrimitives; ++i) {
				//for (int i = 0; i < settings->numIndices/3; i++) {
					/*
					ind = vload3(((nodes[curPos].ptr.w)+i), indices);
					//ind = vload3(i, indices);
					p = vload8(ind.x, vertices);
					q = vload8(ind.y, vertices);
					r = vload8(ind.z, vertices);
					*/
					if (primitives[primitiveIndices[nodes[curPos].ptr+i]]->intersect(ray, tmin, tmax, tempIr)) {
//					if (rayIntersectsTriangle2(ray, p.s012, q.s012, r.s012, &tempIr)) {
						
						if (tempIr.t < ans.t) {
							hit = true;
							ans = tempIr;
							//ans->u = tempIr.u;
							//ans->v = tempIr.v;
							//ans->index = (nodes[curPos].ptr.w)+i;
							//ans->point = ans->v*r + ans->u*q + (1.0f-ans->u-ans->v)*p;
							
						}			
					}
				}				
				//return false;
			}
			curPos++;
		}
		else {
			if (!nodes[curPos].isLeaf()) { // if !isLeaf()
				curPos = nodes[curPos].ptr;
			} else {
				curPos++;
			}
		}
	}
	return hit;
}
    
	
	
const bool BVH::intersectionYN(const Ray& ray, const float tmin, const float tmax) const {
	int curPos = 0;
	while (curPos < nextAllocedNode) {
		
		//if (rayIntersectsBBox(ray, nodes[curPos].v0, nodes[curPos].v1)) {
		if (nodes[curPos].bbox.intersectYN(ray, tmin, tmax)) {		
			//if (nodes[curPos].numFaces > 0) { // if so, this is a leaf
			//if (nodes[curPos].numFaces.w >> 31 == 1) {
			if (nodes[curPos].isLeaf()) {
				//return true;
				uint ind;
				float p, q, r;
				for (int i = 0; i < nodes[curPos].numPrimitives; ++i) {
				//for (int i = 0; i < settings->numIndices/3; i++) {
					/*
					ind = vload3(((nodes[curPos].ptr.w)+i), indices);
					//ind = vload3(i, indices);
					p = vload8(ind.x, vertices);
					q = vload8(ind.y, vertices);
					r = vload8(ind.z, vertices);
					*/
					if (primitives[primitiveIndices[nodes[curPos].ptr]+i]->intersectYN(ray, tmin, tmax)) {
//					if (rayIntersectsTriangle2(ray, p.s012, q.s012, r.s012, &tempIr)) {
						return true;
					}
				}				
				//return false;
			}
			curPos++;
		}
		else {
			if (!nodes[curPos].isLeaf()) { // if !isLeaf()
				curPos = nodes[curPos].ptr;
			} else {
				curPos++;
			}
		}
	}
	return false;
}

/*
typedef struct _face_pos {
	uint face;
	float pos;
} FP;

int compare (const void * a, const void * b)
{
  return ( (*(FP*)a).pos - (*(FP*)b).pos );
}


void BVH::splitNode(BVHNode *node) {
	//cout << "entering split with " << this->numFaces << " faces\n";
	if (this->numFaces == 2) {
		this->left = new BVHNode(this->faces[0]);
		this->right = new BVHNode(this->faces[1]);
		return;
	}
		
	int leftFaces = 0, rightFaces = 0;

	// determine split position: sort centroids along axis

	FP *fp = new FP[numFaces];
	for (int i = 0; i < numFaces; i++)  {
		//fp[i].pos = tmb->faceBBoxes[nodeFaces[i]].center().cell[axis];
		fp[i].pos = 
		fp[i].face = faces[i];
	}
	
  qsort (fp, numFaces, sizeof(FP), compare);
	
	left = new BVHNode(tmb, BBox(Vector3(POS_INF, POS_INF, POS_INF), Vector3(NEG_INF, NEG_INF, NEG_INF)));
	right = new BVHNode(tmb, BBox(Vector3(POS_INF, POS_INF, POS_INF), Vector3(NEG_INF, NEG_INF, NEG_INF)));
	
	left->numFaces = numFaces/2;
	right->numFaces = numFaces-numFaces/2;
	
	left->nodeFaces = new int [left->numFaces];
	right->nodeFaces = new int [right->numFaces];

	for (int i = 0; i < numFaces/2; i++) {
		left->nodeFaces[i] = fp[i].face;
		left->bbox.enclose(tmb->faceBBoxes[left->nodeFaces[i]]);
	}
	int rind=0;
	for (int i = numFaces/2; i < numFaces; i++) {
		right->nodeFaces[rind++] = fp[i].face;
		right->bbox.enclose(tmb->faceBBoxes[fp[i].face]);
	}

	
	delete[] fp;
	delete[] this->nodeFaces;
	this->numFaces = 0;

	if (left->numFaces > 1)
		left->split((axis+1) % 3);
	if (right->numFaces > 1)
		right->split((axis+1) % 3);
	
}

*/

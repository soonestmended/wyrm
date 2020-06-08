#include "BBox.hpp"

void BBox::enclose(const Vec3& v) {
    min.x = min.x < v.x ? min.x : v.x;
    min.y = min.y < v.y ? min.y : v.y;
    min.z = min.z < v.z ? min.z : v.z;

    max.x = max.x > v.x ? max.x : v.x;
    max.y = max.y > v.y ? max.y : v.y;
    max.z = max.z > v.z ? max.z : v.z;
    
}

void BBox::enclose(const BBox& bbox) {
    min.x = min.x < bbox.min.x ? min.x : bbox.min.x;
    min.y = min.y < bbox.min.y ? min.y : bbox.min.y;
    min.z = min.z < bbox.min.z ? min.z : bbox.min.z;

    max.x = max.x > bbox.max.x ? max.x : bbox.max.x;
    max.y = max.y > bbox.max.y ? max.y : bbox.max.y;
    max.z = max.z > bbox.max.z ? max.z : bbox.max.z;
}

// Intersect BBox, store tmin and tmax of intersection in a and b.
bool BBox::intersect(const Ray &r, Real &a, Real &b) const {
	Real tymin, tymax, tzmin, tzmax;
	
	const Vec3 *bounds[2] = {&min, &max};
    /*
	a = ((*bounds[r.sign[0]])[0] - r.o[0]) * r.inv_d[0];
	b = ((*bounds[1-r.sign[0]])[0] - r.o[0]) * r.inv_d[0];

	tymin = ((*bounds[r.sign[1]])[1] - r.o[1]) * r.inv_d[1];
	tymax = ((*bounds[1-r.sign[1]])[1] - r.o[1]) * r.inv_d[1];
    */

    a = ((*bounds[r.sign[0]])[0] - r.o[0]) / r.d[0];
	b = ((*bounds[1-r.sign[0]])[0] - r.o[0]) / r.d[0];

	tymin = ((*bounds[r.sign[1]])[1] - r.o[1]) / r.d[1];
	tymax = ((*bounds[1-r.sign[1]])[1] - r.o[1]) / r.d[1];


    if ( (a > tymax) || (tymin > b) )
		return false;

	if (tymin > a)
		a = tymin;

	if (tymax < b) 
		b = tymax;
    /*
	tzmin = ((*bounds[r.sign[2]])[2] - r.o[2]) * r.inv_d[2];
	tzmax = ((*bounds[1-r.sign[2]])[2] - r.o[2]) * r.inv_d[2];
    */

    tzmin = ((*bounds[r.sign[2]])[2] - r.o[2]) / r.d[2];
	tzmax = ((*bounds[1-r.sign[2]])[2] - r.o[2]) / r.d[2];


    if ( (a > tzmax) || (tzmin > b) )
		return false;

	if (tzmin > a)
		a = tzmin;

	if (tzmax < b)
		b = tzmax;
	
	return true;
}

// Intersect BBox, store nothing
bool BBox::intersectYN(const Ray &r, Real t0, Real t1) const {
	Real tmin, tmax, tymin, tymax, tzmin, tzmax;
	
	const Vec3 *bounds[2] = {&min, &max};
    /*
	tmin = ((*bounds[r.sign[0]])[0] - r.o[0]) * r.inv_d[0];
	tmax = ((*bounds[1-r.sign[0]])[0] - r.o[0]) * r.inv_d[0];

	tymin = ((*bounds[r.sign[1]])[1] - r.o[1]) * r.inv_d[1];
	tymax = ((*bounds[1-r.sign[1]])[1] - r.o[1]) * r.inv_d[1];
    */
	tmin = ((*bounds[r.sign[0]])[0] - r.o[0]) / r.d[0];
	tmax = ((*bounds[1-r.sign[0]])[0] - r.o[0]) / r.d[0];

	tymin = ((*bounds[r.sign[1]])[1] - r.o[1]) / r.d[1];
	tymax = ((*bounds[1-r.sign[1]])[1] - r.o[1]) / r.d[1];


    if ( (tmin > tymax) || (tymin > tmax) )
		return false;

	if (tymin > tmin)
		tmin = tymin;

	if (tymax < tmax) 
		tmax = tymax;
    /*
	tzmin = ((*bounds[r.sign[2]])[2] - r.o[2]) * r.inv_d[2];
	tzmax = ((*bounds[1-r.sign[2]])[2] - r.o[2]) * r.inv_d[2];
    */

    tzmin = ((*bounds[r.sign[2]])[2] - r.o[2]) / r.d[2];
	tzmax = ((*bounds[1-r.sign[2]])[2] - r.o[2]) / r.d[2];


    if ( (tmin > tzmax) || (tzmin > tmax) )
		return false;

	if (tzmin > tmin)
		tmin = tzmin;

	if (tzmax < tmax)
		tmax = tzmax;

	return ( (tmin < t1) && (tmax > t0) );
}

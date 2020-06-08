#include "common.hpp"
#include "Primitive.hpp"
#include "Utils.hpp"

using namespace std;

const shared_ptr <Material> Primitive::getMaterial() const {
    return material;
}

const BBox& Primitive::getBBox() const {
    return bbox;
}

const bool Triangle::intersect(const Ray &ray, Real tmin, Real tmax, IntersectRec& ir) const {
    Vec3 P, Q, T;
    Real det, inv_det, u, v;
    Real t;

    //Begin calculating determinant - also used to calculate u parameter
    P = glm::cross(ray.d, e1);
 
    //if determinant is near zero, ray lies in plane of triangle
    det = glm::dot(e0, P);

    //NOT CULLING
    if((det > -EPSILON) && (det < EPSILON)) return false;
    inv_det = 1 / det;
 
    //calculate distance from V0 to ray origin
    T = ray.o - this->v[0];
 
    //Calculate u parameter and test bound
    u = glm::dot(T, P) * inv_det;
  
    //The intersection lies outside of the triangle
    if(u <= 0 || u >= 1) return false;
 
    //Prepare to test v parameter
    Q = glm::cross(T, e0);
 
    //Calculate V parameter and test bound
    v = glm::dot(ray.d, Q) * inv_det;

    //The intersection lies outside of the triangle
    if(v <= 0 || u + v  >= 1) return false;
 
    t = glm::dot(e1, Q) * inv_det;
 
    if(t >= tmin && t <= tmax) { //ray intersection
        ir.t = t;
        ir.primitive = (Primitive *) this;
        ir.normal = this->N;
        ir.material = this->getMaterial();
        ir.isectPoint = ray.o + t * ray.d;
        ir.uvw[1] = u;
        ir.uvw[2] = v;
        ir.uvw[0] = 1 - u - v;
        //std::cout << "hit";
        return true;
    }
 
    // No hit, no win
    return false;
}

const bool Triangle::intersectYN(const Ray &ray, Real tmin, Real tmax) const  {
    Vec3 P, Q, T;
    Real det, inv_det, u, v;
    Real t;

    //Begin calculating determinant - also used to calculate u parameter
    P = glm::cross(ray.d, e1);
 
    //if determinant is near zero, ray lies in plane of triangle
    det = glm::dot(e0, P);

    //NOT CULLING
    if((det > -EPSILON) && (det < EPSILON)) return false;
    inv_det = 1 / det;
 
    //calculate distance from V0 to ray origin
    T = ray.o - this->v[0];
 
    //Calculate u parameter and test bound
    u = glm::dot(T, P) * inv_det;
  
    //The intersection lies outside of the triangle
    if(u < 0 || u > 1) return false;
 
    //Prepare to test v parameter
    Q = glm::cross(T, e0);
 
    //Calculate V parameter and test bound
    v = glm::dot(ray.d, Q) * inv_det;

    //The intersection lies outside of the triangle
    if(v < 0 || u + v  > 1) return false;
 
    t = glm::dot(e1, Q) * inv_det;
 
    if(t >= tmin && t <= tmax) { //ray intersection
        return true;
    }
 
    // No hit, no win
    return false;
}

void Triangle::getRandomPoint(const Vec2& uv, Vec3& p, Real* pdf) const {
    Vec2 bg;
	utils::uniformSampleTriangle(uv, bg);
	p = bg[0] * v[0] + bg[1] * v[1] + (1 - bg[0] - bg[1]) * v[2];
    //cout << "\t" << utils::v2s(v[0]) << "\t" << utils::v2s(v[1]) << "\t" << utils::v2s(v[2]) << endl;
    //cout << "[" << bg[0] << ", " << bg[1] << "]: " << (Color) p << endl;
    *pdf = 1.0f / getSurfaceArea();
}
    
void Triangle::getRandomPointAndDirection(const Vec2& uv, Vec3& p, Vec3& d, Real* pdf) const {
    this->getRandomPoint(uv, p, pdf);
    d = this->N;
}

void TriangleWarp::getRandomPoint(const Vec2& uv, Vec3& p, Real* pdf) const {
    return Triangle::getRandomPoint(uv, p, pdf);
}
    
void TriangleWarp::getRandomPointAndDirection(const Vec2& uv, Vec3& p, Vec3& d, Real* pdf) const {
    return Triangle::getRandomPointAndDirection(uv, p, d, pdf);
}

const Real Triangle::getSurfaceArea() const {
	// Heron's method
  Real a = glm::length(e0);
  Real b = glm::length(e1);
  Real c = glm::length(v[2]-v[1]);
  Real s = 0.5 * (a+b+c);
  return sqrtf(s*(s-a)*(s-b)*(s-c));
}

const bool Box::intersectYN(const Ray& ray, const Real tmin, const Real tmax) const {
    return bbox.intersectYN(ray, tmin, tmax);
}

const bool Box::intersect(const Ray& r, Real t0, Real t1, IntersectRec& ir) const {
	Real tmin, tmax, tymin, tymax, tzmin, tzmax;
	enum Side {TOP, BOTTOM, LEFT, RIGHT, NEAR, FAR};
    Side hitSide = r.sign[0] ? RIGHT : LEFT;

	const Vec3 *bounds[2] = {&bbox.min, &bbox.max};
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

	if (tymin > tmin) {
		tmin = tymin;
        hitSide = r.sign[1] ? BOTTOM : TOP;
    }

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

	if (tzmin > tmin) {
		tmin = tzmin;
        hitSide = r.sign[2] ? FAR : NEAR;
    }

	if (tzmax < tmax)
		tmax = tzmax;

	if ( (tmin < t1) && (tmax > t0) ) {
        if (tmin >= t0 && tmin <= t1) {
            ir.t = tmin;
        }
        else if (tmax >= tmin && tmax <= t1) {
            ir.t = tmax;
        }
        else {
            return false;
        }
        // hit
        ir.material = this->material;
        ir.isectPoint = r.o + ir.t * r.d;
        //cout << "Box isectP: " << ir.isectPoint << "   t: " << ir.t << endl;
        ir.primitive = (Primitive *) this;
        if (hitSide == LEFT)
            ir.normal = Vec3(-1, 0, 0);
        else if (hitSide == RIGHT)
            ir.normal = Vec3(1, 0, 0);
        else if (hitSide == BOTTOM)
            ir.normal = Vec3(0, -1, 0);
        else if (hitSide == TOP)
            ir.normal = Vec3(0, 1, 0);
        else if (hitSide == NEAR) 
            ir.normal = Vec3(0, 0, -1);
        else
            ir.normal = Vec3(0, 0, 1);
        return true;
    }
    else {
        return false;
    }
}

void Box::getRandomPoint(const Vec2& uv, Vec3& p, Real* pdf) const {
}

void Box::getRandomPointAndDirection(const Vec2& uv, Vec3& p, Vec3& d, Real* pdf) const {
}

const Real Box::getSurfaceArea() const {
    return 2.0f * (
        (bbox.max[0] - bbox.min[0]) * (bbox.max[1] - bbox.min[1]) +
        (bbox.max[2] - bbox.min[2]) * (bbox.max[1] - bbox.min[1]) +
        (bbox.max[2] - bbox.min[2]) * (bbox.max[0] - bbox.min[0]));
}

void swap(Real& a, Real& b) {
    Real tmp = a;
    a = b;
    b = tmp;
}

const bool Cone::intersect(const Ray& ray, const Real tmin, const Real tmax, IntersectRec& ir) const {
    
    Vec3 C = Vec3(0, height/2.0f, 0);
    Vec3 CO = ray.o - C;
    Vec3 V = Vec3(0, -1, 0);
    Real cosTheta2 = height * height / (height*height + bottomRadius*bottomRadius);
    Real a = ray.d[1] * ray.d[1] - cosTheta2;
    Real b = 2.0f * ((-ray.d[1] * -CO[1]) - glm::dot(ray.d, CO) * cosTheta2);
    Real c = (CO[1] * CO[1]) - (glm::dot(CO, CO) * cosTheta2);
    Real det = b*b - 4.0f*a*c;
    if (det <= 0.0f) return false;
    
    Real tClose = (-b - sqrt(det)) / (2.0f * a);
    Real tFar = (-b + sqrt(det)) / (2.0f * a);
    if (tFar < tClose) swap(tClose, tFar);
    Real tCap = (-height/2.0f - ray.o.y) / ray.d.y; 

    Vec3 P = ray.o + ray.d * tClose;
    //Real pcv = -(P-C)[1];
    if ( P.y > -height/2.0f && P.y < height/2.0f && tClose >= tmin && tClose <= tmax) {// && pcv > 0.0f) {
        ir.isectPoint = P;
        ir.t = tClose;
        ir.primitive = (Primitive *) this;
        // FILL IN NORMAL
        Vec3 cp = P - C;
        ir.normal = glm::normalize(cp * -cp[1] / glm::dot(cp, cp) - Vec3(0, -1, 0));
        ir.material = this->getMaterial();
        // FILL IN UVW (maybe)
        return true;
    }

    if (tClose <= tCap && tCap <= tFar && tCap >= tmin && tCap <= tmax) { // as with a cylinder, if the ray hits the end cap between hitting either side of the cone, then there's an end cap intersection
        ir.isectPoint = ray.o + ray.d*tCap;
        ir.t = tCap;
        ir.primitive = (Primitive *) this;
        // FILL IN NORMAL
        ir.normal = Vec3(0, -1, 0);
        ir.material = this->getMaterial();
        // FILL IN UVW (maybe)
        return true;
    }
    
    P = ray.o + ray.d * tFar;
    //pcv = -(P-C)[1];
    if (P.y > -height/2.0f && P.y < height/2.0f && tFar >= tmin && tFar <= tmax) {// && pcv > 0.0f) {
        ir.isectPoint = P;
        ir.t = tFar;
        ir.primitive = (Primitive *) this;
        // FILL IN NORMAL
        Vec3 cp = P - C;//Vec3(0, height/2.0f, 0);
        ir.normal = glm::normalize(cp * -cp[1] / glm::dot(cp, cp) - Vec3(0, -1, 0));

        ir.material = this->getMaterial();
        // FILL IN UVW (maybe)
        return true;
    }
    
    return false;
}

const bool Cone::intersectYN(const Ray& ray, const Real tmin, const Real tmax) const {
    Vec3 C = Vec3(0, height/2.0f, 0);
    Vec3 CO = ray.o - C;
    Vec3 V = Vec3(0, -1, 0);
    Real cosTheta2 = height * height / (height*height + bottomRadius*bottomRadius);
    Real a = ray.d[1] * ray.d[1] - cosTheta2;
    Real b = 2.0f * ((-ray.d[1] * -CO[1]) - glm::dot(ray.d, CO) * cosTheta2);
    Real c = (CO[1] * CO[1]) - (glm::dot(CO, CO) * cosTheta2);
    Real det = b*b - 4.0f*a*c;
    if (det <= 0.0f) return false;
    Real tClose = (-b - sqrt(det)) / (2.0f * a);
    Real tFar = (-b + sqrt(det)) / (2.0f * a);
    if (tFar < tClose) swap(tClose, tFar);
    Vec3 P = ray.o + ray.d * tClose;
    if (P.y > -height/2.0f && P.y < height/2.0f && tClose >= tmin && tClose <= tmax) return true;
    Real tCap = (-height/2.0f - ray.o.y) / ray.d.y; 
    if (tClose <= tCap && tCap <= tFar && tCap >= tmin && tCap <= tmax) return true; // as with a cylinder, if the ray hits the end cap between hitting either side of the cone, then there's an end cap intersection
    P = ray.o + ray.d * tFar;
    if (P.y > -height/2.0f && P.y < height/2.0f && tFar >= tmin && tFar <= tmax) return true;

    return false;
}

void Cone::getRandomPoint(const Vec2& uv, Vec3& p, Real* pdf) const {

}

void Cone::getRandomPointAndDirection(const Vec2& uv, Vec3& p, Vec3& d, Real* pdf) const {

}

const Real Cone::getSurfaceArea() const {
    // pi * r^2 + pi * L * r
    Real L = sqrt(bottomRadius*bottomRadius + height*height);
    return M_PI * bottomRadius * (bottomRadius + L); 
}

const bool Cylinder::intersect(const Ray& ray, const Real tmin, const Real tmax, IntersectRec& ir) const {
    Real a = ray.d.x*ray.d.x + ray.d.z*ray.d.z;
    Real b = 2.0f * (ray.d.x*ray.o.x + ray.d.z*ray.o.z);
    Real c = ray.o.x*ray.o.x + ray.o.z*ray.o.z - radius*radius;
    Real det = b*b - 4.0f*a*c;
    if (det <= 0.0f) return false;
    Real ymin = -height/2.0f;
    Real ymax = height/2.0f;
    Real t[4] = {(-b - sqrt(det)) / (2.0f * a), (-b + sqrt(det)) / (2.0f * a), (ymin - ray.o.y) / ray.d.y, (ymax - ray.o.y) / ray.d.y};
    // here we know that t[0] < t[1] because a must be positive
    bool valid[4] = {true, true, true, true};

    // rule out intersections outside desired interval
    for (int i = 0; i < 4; ++i)
        if (t[i] < tmin || t[i] > tmax) 
            valid[i] = false;
        
    Real yvals[4];
    for (int i = 0; i < 4; ++i)
        yvals[i] = ray.o.y + ray.d.y * t[i];
    
    if (yvals[0] < ymin || yvals[0] > ymax)
        valid[0] = false;
    
    if (yvals[1] < ymin || yvals[1] > ymax)
        valid[1] = false;

    // possibly exclude end cap intersections
    if (ray.d.y > 0.0f) {
        if (!(yvals[0] <=  ymin && ymin <= yvals[1])) {
            valid[2] = false;
        }
        if (!(yvals[0] <=  ymax && ymax <= yvals[1])) {
            valid[3] = false;
        }
    }
    else {
        if (!(yvals[0] > ymin && ymin > yvals[1])) {
            valid[2] = false;
        }

        if (!(yvals[0] > ymax && ymax > yvals[1])) {
            valid[3] = false;
        }
    }

    int mindex;
    ir.t = POS_INF;
    for (int i = 0; i < 4; ++i) {
        if (t[i] < ir.t && valid[i]) {
            ir.t = t[i];
            mindex = i;
        }
    }

    ir.isectPoint = ray.o + ray.d * ir.t;
    ir.primitive = (Primitive *) this;
    // FILL IN NORMAL
    if (mindex < 2) {
        ir.normal = glm::normalize(Vec3(ir.isectPoint.x, 0.0f, ir.isectPoint.z));
    }
    else if (mindex == 2) {
        ir.normal = Vec3(0, -1, 0);
    }
    else {
        ir.normal = Vec3(0, 1, 0);
    }
    ir.material = this->getMaterial();
    // FILL IN UVW (maybe)
    return true;
}

const bool Cylinder::intersectYN(const Ray& ray, const Real tmin, const Real tmax) const {
    Real a = ray.d.x*ray.d.x + ray.d.z*ray.d.z;
    Real b = 2.0f * (ray.d.x*ray.o.x + ray.d.z*ray.o.z);
    Real c = ray.o.x*ray.o.x + ray.o.z*ray.o.z - radius*radius;
    Real det = b*b - 4.0f*a*c;
    if (det <= 0.0f) return false;
    Real ymin = -height/2.0f;
    Real ymax = height/2.0f;
    Real t[4] = {(-b - sqrt(det)) / (2.0f * a), (-b + sqrt(det)) / (2.0f * a), (ymin - ray.o.y) / ray.d.y, (ymax - ray.o.y) / ray.d.y};
    bool valid[4] = {true, true, true, true};

    // rule out intersections outside desired interval
    for (int i = 0; i < 4; ++i)
        if (t[i] < tmin || t[i] > tmax) 
            valid[i] = false;
        
    Real yvals[4];
    for (int i = 0; i < 4; ++i)
        yvals[i] = ray.o.y + ray.d.y * t[i];
    
    if (yvals[0] < ymin || yvals[0] > ymax)
        valid[0] = false;
    
    if (yvals[1] < ymin || yvals[1] > ymax)
        valid[1] = false;

    if (valid[0] || valid[1]) return true;

    // possibly exclude end cap intersections
    if (ray.d.y > 0.0f) {
        if (!(yvals[0] <=  ymin && ymin <= yvals[1])) {
            valid[2] = false;
        }
        if (!(yvals[0] <=  ymax && ymax <= yvals[1])) {
            valid[3] = false;
        }
    }
    else {
        if (!(yvals[0] > ymin && ymin > yvals[1])) {
            valid[2] = false;
        }

        if (!(yvals[0] > ymax && ymax > yvals[1])) {
            valid[3] = false;
        }
    }
    return (valid[2] || valid[3]);
}

void Cylinder::getRandomPoint(const Vec2& uv, Vec3& p, Real* pdf) const {
    // TODO
}
void Cylinder::getRandomPointAndDirection(const Vec2& uv, Vec3& p, Vec3& d, Real* pdf) const {
    // TODO
}
const Real Cylinder::getSurfaceArea() const {
    return 2.0f * M_PI * radius * (radius + height);
}

const bool Sphere::intersect(const Ray& ray, const Real tmin, const Real tmax, IntersectRec& ir) const {
    Real a = glm::dot(ray.d, ray.d);
    Real b = 2.0 * glm::dot(ray.o, ray.d);
    Real c = glm::dot(ray.o, ray.o) - radius * radius;
    Real det = b*b - 4.0f*a*c;
    if (det < 0.0f) return false;
    Real t = (-b - sqrt(det)) / (2.0f * a);
    if (t >= tmin && t <= tmax) {
        ir.t = t;
        ir.material = this->getMaterial();
        ir.primitive = (Primitive *) this;
        ir.isectPoint = ray.o + t*ray.d;
        ir.normal = glm::normalize(ir.isectPoint);
        //cout << "\t\tSphere isect normal: " << utils::v2s(ir.normal) << endl;
        return true;
    }
    t = (-b + sqrt(det)) / (2.0f * a);
    if (t >= tmin && t <= tmax) {
        ir.t = t;
        ir.material = this->getMaterial();
        ir.primitive = (Primitive *) this;
        ir.isectPoint = ray.o + t*ray.d;
        ir.normal = glm::normalize(ir.isectPoint);
        //cout << "\t\tSphere isect normal: " << utils::v2s(ir.normal) << endl;

        return true;
    }
    return false;
}

const bool Sphere::intersectYN(const Ray& ray, const Real tmin, const Real tmax) const {
    Real a = glm::dot(ray.d, ray.d);
    Real b = 2.0 * glm::dot(ray.o, ray.d);
    Real c = glm::dot(ray.o, ray.o) - radius * radius;
    Real det = b*b - 4.0f*a*c;
    if (det <= 0.0f) return false;
    Real t = (-b - sqrt(det)) / (2.0f * a);
    if (t >= tmin && t <= tmax) return true;
    t = (-b + sqrt(det)) / (2.0f * a);
    if (t >= tmin && t <= tmax) return true;
    return false;
}

void Sphere::getRandomPoint(const Vec2& uv, Vec3& p, Real* pdf) const {

}

void Sphere::getRandomPointAndDirection(const Vec2& uv, Vec3& p, Vec3& d, Real* pdf) const {

}

const Real Sphere::getSurfaceArea() const {
    return 4.0 * M_PI * radius * radius;
}
 
const bool TransformedPrimitive::intersect(const Ray& ray, const Real tmin, const Real tmax, IntersectRec& ir) const {
    // transform ray to local coordinates
    // compute intersection
    // transform intersection components to world coordinates
    // put utility functions to transform normals, vectors, and points in Utils namespace
    Ray localRay = localToWorld.inverseTransformRay(ray);
    //std::cout << localRay.o.x << ", " << localRay.o.y << ", " << localRay.o.z << std::endl;
    if (!prim->intersect(localRay, tmin, tmax, ir)) return false;
    ir.isectPoint = localToWorld.transformPoint(ir.isectPoint);
    ir.normal = glm::normalize(localToWorld.transformNormal(ir.normal));
    //ir.normal = Vec3(0, 0, -1);
    return true;
}

const bool TransformedPrimitive::intersectYN(const Ray& ray, const Real tmin, const Real tmax) const {
    // transform ray to local coordinates
    // compute intersectYN
    Ray localRay = localToWorld.inverseTransformRay(ray);
    return prim->intersectYN(localRay, tmin, tmax); 
}

void TransformedPrimitive::getRandomPoint(const Vec2& uv, Vec3& p, Real* pdf) const {
    // get random point from primitive
    // transform to world coordinates
    prim->getRandomPoint(uv, p, pdf);
    p = localToWorld.transformPoint(p);
}

void TransformedPrimitive::getRandomPointAndDirection(const Vec2& uv, Vec3& p, Vec3& d, Real* pdf) const {
    // get random point and direction from primitive
    // transform both to world coordinates
    prim->getRandomPointAndDirection(uv, p, d, pdf);
    p = localToWorld.transformPoint(p);
    d = localToWorld.transformVector(d);
}

const Real TransformedPrimitive::getSurfaceArea() const {
    // need to scale this by ... xf determinant?
    // No; this is harder than I thought. For now, just return underlying primitive surface area
    return prim->getSurfaceArea();

}

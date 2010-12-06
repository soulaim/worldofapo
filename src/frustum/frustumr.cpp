#include "frustumr.h"
#include <math.h>
#include <stdio.h>

//#include <GL/glut.h>

#define ANG2RAD 3.14159265358979323846/360.0 



FrustumR::FrustumR() {}

FrustumR::~FrustumR() {}

#define m(col,row)  m[row*4+col]


void FrustumR::setFrustum(float *m) {

	pl[NEARP].setCoefficients(m(2,0) + m(3,0),
	                          m(2,1) + m(3,1),
	                          m(2,2) + m(3,2),
	                          m(2,3) + m(3,3));
	pl[FARP].setCoefficients( -m(2,0) + m(3,0),
	                          -m(2,1) + m(3,1),
	                          -m(2,2) + m(3,2),
	                          -m(2,3) + m(3,3));
	pl[BOTTOM].setCoefficients(m(1,0) + m(3,0),
	                           m(1,1) + m(3,1),
	                           m(1,2) + m(3,2),
	                           m(1,3) + m(3,3));
	pl[TOP].setCoefficients(  -m(1,0) + m(3,0),
	                          -m(1,1) + m(3,1),
	                          -m(1,2) + m(3,2),
	                          -m(1,3) + m(3,3));
	pl[LEFT].setCoefficients(  m(0,0) + m(3,0),
	                           m(0,1) + m(3,1),
	                           m(0,2) + m(3,2),
	                           m(0,3) + m(3,3));
	pl[RIGHT].setCoefficients(-m(0,0) + m(3,0),
	                          -m(0,1) + m(3,1),
	                          -m(0,2) + m(3,2),
	                          -m(0,3) + m(3,3));
}

#undef M


void FrustumR::setCamInternals(float angle, float ratio, float nearD, float farD) {

	// store the information
	this->ratio = ratio;
	this->angle = angle * ANG2RAD;
	this->nearD = nearD;
	this->farD = farD;

	// compute width and height of the near and far plane sections
	tang = tan(this->angle);
	sphereFactorY = 1.0/cos(this->angle);//tang * sin(this->angle) + cos(this->angle);

	float anglex = atan(tang*ratio);
	sphereFactorX = 1.0/cos(anglex); //tang*ratio * sin(anglex) + cos(anglex);

	nh = nearD * tang;
	nw = nh * ratio; 

	fh = farD * tang;
	fw = fh * ratio;

}


void FrustumR::setCamDef(Vec3 &p, Vec3 &l, Vec3 &u) {

	Vec3 dir,nc,fc;

	camPos = p;

	// compute the Z axis of camera
	Z = p - l;
	Z.normalize();

	// X axis of camera of given "up" vector and Z axis
	X = u * Z;
	X.normalize();

	// the real "up" vector is the cross product of Z and X
	Y = Z * X;

	// compute the center of the near and far planes
	nc = p - Z * nearD;
	fc = p - Z * farD;

	// compute the 8 corners of the frustum
	ntl = nc + Y * nh - X * nw;
	ntr = nc + Y * nh + X * nw;
	nbl = nc - Y * nh - X * nw;
	nbr = nc - Y * nh + X * nw;

	ftl = fc + Y * fh - X * fw;
	fbr = fc - Y * fh + X * fw;
	ftr = fc + Y * fh + X * fw;
	fbl = fc - Y * fh - X * fw;

	// compute the six planes
	// the function set3Points asssumes that the points
	// are given in counter clockwise order
	pl[TOP].set3Points(ntr,ntl,ftl);
	pl[BOTTOM].set3Points(nbl,nbr,fbr);
	pl[LEFT].set3Points(ntl,nbl,fbl);
	pl[RIGHT].set3Points(nbr,ntr,fbr);
//	pl[NEARP].set3Points(ntl,ntr,nbr);
//	pl[FARP].set3Points(ftr,ftl,fbl);

	pl[NEARP].setNormalAndPoint(-Z,nc);
	pl[FARP].setNormalAndPoint(Z,fc);

	Vec3 aux,normal;

	aux = (nc + Y*nh) - p;
	normal = aux * X;
	pl[TOP].setNormalAndPoint(normal,nc+Y*nh);

	aux = (nc - Y*nh) - p;
	normal = X * aux;
	pl[BOTTOM].setNormalAndPoint(normal,nc-Y*nh);
	
	aux = (nc - X*nw) - p;
	normal = aux * Y;
	pl[LEFT].setNormalAndPoint(normal,nc-X*nw);

	aux = (nc + X*nw) - p;
	normal = Y * aux;
	pl[RIGHT].setNormalAndPoint(normal,nc+X*nw);
}



int FrustumR::pointInFrustum(const Vec3 &p) const {

	float pcz,pcx,pcy,aux;

	// compute vector from camera position to p
	Vec3 v = p-camPos;

	// compute and test the Z coordinate
	pcz = v.innerProduct(-Z);
	if (pcz > farD || pcz < nearD)
		return(OUTSIDE);

	// compute and test the Y coordinate
	pcy = v.innerProduct(Y);
	aux = pcz * tang;
	if (pcy > aux || pcy < -aux)
		return(OUTSIDE);
		
	// compute and test the X coordinate
	pcx = v.innerProduct(X);
	aux = aux * ratio;
	if (pcx > aux || pcx < -aux)
		return(OUTSIDE);


	return(INSIDE);

	
}


int FrustumR::sphereInFrustum(const Vec3 &p, float radius) const {


	float d1,d2;
	float az,ax,ay,zz1,zz2;
	int result = INSIDE;

	Vec3 v = p-camPos;

	az = v.innerProduct(-Z);
	if (az > farD + radius || az < nearD-radius)
		return(OUTSIDE);

	ax = v.innerProduct(X);
	zz1 = az * tang * ratio;
	d1 = sphereFactorX * radius;
	if (ax > zz1+d1 || ax < -zz1-d1)
		return(OUTSIDE);

	ay = v.innerProduct(Y);
	zz2 = az * tang;
	d2 = sphereFactorY * radius;
	if (ay > zz2+d2 || ay < -zz2-d2)
		return(OUTSIDE);



	if (az > farD - radius || az < nearD+radius)
		result = INTERSECT;
	if (ay > zz2-d2 || ay < -zz2+d2)
		result = INTERSECT;
	if (ax > zz1-d1 || ax < -zz1+d1)
		result = INTERSECT;


	return(result);

}

/*
void FrustumR::drawPoints() {


	glBegin(GL_POINTS);

		glVertex3f(ntl.x,ntl.y,ntl.z);
		glVertex3f(ntr.x,ntr.y,ntr.z);
		glVertex3f(nbl.x,nbl.y,nbl.z);
		glVertex3f(nbr.x,nbr.y,nbr.z);

		glVertex3f(ftl.x,ftl.y,ftl.z);
		glVertex3f(ftr.x,ftr.y,ftr.z);
		glVertex3f(fbl.x,fbl.y,fbl.z);
		glVertex3f(fbr.x,fbr.y,fbr.z);

	glEnd();
}


void FrustumR::drawLines() {

	glBegin(GL_LINE_LOOP);
	//near plane
		glVertex3f(ntl.x,ntl.y,ntl.z);
		glVertex3f(ntr.x,ntr.y,ntr.z);
		glVertex3f(nbr.x,nbr.y,nbr.z);
		glVertex3f(nbl.x,nbl.y,nbl.z);
	glEnd();

	glBegin(GL_LINE_LOOP);
	//far plane
		glVertex3f(ftr.x,ftr.y,ftr.z);
		glVertex3f(ftl.x,ftl.y,ftl.z);
		glVertex3f(fbl.x,fbl.y,fbl.z);
		glVertex3f(fbr.x,fbr.y,fbr.z);
	glEnd();

	glBegin(GL_LINE_LOOP);
	//bottom plane
		glVertex3f(nbl.x,nbl.y,nbl.z);
		glVertex3f(nbr.x,nbr.y,nbr.z);
		glVertex3f(fbr.x,fbr.y,fbr.z);
		glVertex3f(fbl.x,fbl.y,fbl.z);
	glEnd();

	glBegin(GL_LINE_LOOP);
	//top plane
		glVertex3f(ntr.x,ntr.y,ntr.z);
		glVertex3f(ntl.x,ntl.y,ntl.z);
		glVertex3f(ftl.x,ftl.y,ftl.z);
		glVertex3f(ftr.x,ftr.y,ftr.z);
	glEnd();

	glBegin(GL_LINE_LOOP);
	//left plane
		glVertex3f(ntl.x,ntl.y,ntl.z);
		glVertex3f(nbl.x,nbl.y,nbl.z);
		glVertex3f(fbl.x,fbl.y,fbl.z);
		glVertex3f(ftl.x,ftl.y,ftl.z);
	glEnd();

	glBegin(GL_LINE_LOOP);
	// right plane
		glVertex3f(nbr.x,nbr.y,nbr.z);
		glVertex3f(ntr.x,ntr.y,ntr.z);
		glVertex3f(ftr.x,ftr.y,ftr.z);
		glVertex3f(fbr.x,fbr.y,fbr.z);

	glEnd();
}


void FrustumR::drawPlanes() {

	glBegin(GL_QUADS);

	//near plane
		glVertex3f(ntl.x,ntl.y,ntl.z);
		glVertex3f(ntr.x,ntr.y,ntr.z);
		glVertex3f(nbr.x,nbr.y,nbr.z);
		glVertex3f(nbl.x,nbl.y,nbl.z);

	//far plane
		glVertex3f(ftr.x,ftr.y,ftr.z);
		glVertex3f(ftl.x,ftl.y,ftl.z);
		glVertex3f(fbl.x,fbl.y,fbl.z);
		glVertex3f(fbr.x,fbr.y,fbr.z);

	//bottom plane
		glVertex3f(nbl.x,nbl.y,nbl.z);
		glVertex3f(nbr.x,nbr.y,nbr.z);
		glVertex3f(fbr.x,fbr.y,fbr.z);
		glVertex3f(fbl.x,fbl.y,fbl.z);

	//top plane
		glVertex3f(ntr.x,ntr.y,ntr.z);
		glVertex3f(ntl.x,ntl.y,ntl.z);
		glVertex3f(ftl.x,ftl.y,ftl.z);
		glVertex3f(ftr.x,ftr.y,ftr.z);

	//left plane

		glVertex3f(ntl.x,ntl.y,ntl.z);
		glVertex3f(nbl.x,nbl.y,nbl.z);
		glVertex3f(fbl.x,fbl.y,fbl.z);
		glVertex3f(ftl.x,ftl.y,ftl.z);

	// right plane
		glVertex3f(nbr.x,nbr.y,nbr.z);
		glVertex3f(ntr.x,ntr.y,ntr.z);
		glVertex3f(ftr.x,ftr.y,ftr.z);
		glVertex3f(fbr.x,fbr.y,fbr.z);

	glEnd();

}

void FrustumR::drawNormals() {

	Vec3 a,b;

	glBegin(GL_LINES);

		// near
		a = (ntr + ntl + nbr + nbl) * 0.25;
		b = a + pl[NEARP].normal;
		glVertex3f(a.x,a.y,a.z);
		glVertex3f(b.x,b.y,b.z);

		// far
		a = (ftr + ftl + fbr + fbl) * 0.25;
		b = a + pl[FARP].normal;
		glVertex3f(a.x,a.y,a.z);
		glVertex3f(b.x,b.y,b.z);

		// left
		a = (ftl + fbl + nbl + ntl) * 0.25;
		b = a + pl[LEFT].normal;
		glVertex3f(a.x,a.y,a.z);
		glVertex3f(b.x,b.y,b.z);
		
		// right
		a = (ftr + nbr + fbr + ntr) * 0.25;
		b = a + pl[RIGHT].normal;
		glVertex3f(a.x,a.y,a.z);
		glVertex3f(b.x,b.y,b.z);
		
		// top
		a = (ftr + ftl + ntr + ntl) * 0.25;
		b = a + pl[TOP].normal;
		glVertex3f(a.x,a.y,a.z);
		glVertex3f(b.x,b.y,b.z);
		
		// bottom
		a = (fbr + fbl + nbr + nbl) * 0.25;
		b = a + pl[BOTTOM].normal;
		glVertex3f(a.x,a.y,a.z);
		glVertex3f(b.x,b.y,b.z);

	glEnd();


}

void FrustumR::printPlanes() {

	for (int i = 0; i < 6; i++) {

			pl[i].print();
			printf("\n");
	}
}
*/

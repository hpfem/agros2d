// Gmsh - Copyright (C) 1997-2014 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to the public mailing list <gmsh@geuz.org>.

#include "MPrism.h"
#include "Numeric.h"
#include "BasisFactory.h"
#include "Context.h"

int MPrism::getVolumeSign()
{
  double mat[3][3];
  mat[0][0] = _v[1]->x() - _v[0]->x();
  mat[0][1] = _v[2]->x() - _v[0]->x();
  mat[0][2] = _v[3]->x() - _v[0]->x();
  mat[1][0] = _v[1]->y() - _v[0]->y();
  mat[1][1] = _v[2]->y() - _v[0]->y();
  mat[1][2] = _v[3]->y() - _v[0]->y();
  mat[2][0] = _v[1]->z() - _v[0]->z();
  mat[2][1] = _v[2]->z() - _v[0]->z();
  mat[2][2] = _v[3]->z() - _v[0]->z();
  double d = det3x3(mat);
  if(d < 0.) return -1;
  else if(d > 0.) return 1;
  else return 0;
}

void MPrism::getIntegrationPoints(int pOrder, int *npts, IntPt **pts)
{
  *npts = getNGQPriPts(pOrder);
  *pts = getGQPriPts(pOrder);
}

const nodalBasis* MPrism::getFunctionSpace(int order) const
{
  if (order == -1) return BasisFactory::getNodalBasis(getTypeForMSH());

  int tag = ElementType::getTag(TYPE_PRI, order);
  return tag ? BasisFactory::getNodalBasis(tag) : NULL;
}

const JacobianBasis* MPrism::getJacobianFuncSpace(int order) const
{
  if (order == -1) return BasisFactory::getJacobianBasis(getTypeForMSH());

  int tag = ElementType::getTag(TYPE_PRI, order);
  return tag ? BasisFactory::getJacobianBasis(tag) : NULL;
}

double MPrism::getInnerRadius()
{
  double dist[3], k = 0.;
  int triEdges[3] = {0,1,3};
  for (int i = 0; i < 3; i++){
    MEdge e = getEdge(triEdges[i]);
    dist[i] = e.getVertex(0)->distance(e.getVertex(1));
    k += 0.5 * dist[i];
  }
  double radTri = sqrt(k * (k - dist[0]) * (k - dist[1]) * (k - dist[2])) / k;
  double radVert = 0.5*getVertex(0)->distance(getVertex(3));
  return std::min(radTri,radVert);
}

void MPrism::getFaceInfo(const MFace &face, int &ithFace, int &sign, int &rot) const
{
  for (ithFace = 0; ithFace < 5; ithFace++){
    MVertex *v0 = _v[faces_prism(ithFace, 0)];
    MVertex *v1 = _v[faces_prism(ithFace, 1)];
    MVertex *v2 = _v[faces_prism(ithFace, 2)];

    if (face.getNumVertices()==3) {
      if (v0 == face.getVertex(0) && v1 == face.getVertex(1) && v2 == face.getVertex(2)){
        sign = 1; rot = 0; return;
      }
      if (v0 == face.getVertex(1) && v1 == face.getVertex(2) && v2 == face.getVertex(0)){
        sign = 1; rot = 1; return;
      }
      if (v0 == face.getVertex(2) && v1 == face.getVertex(0) && v2 == face.getVertex(1)){
        sign = 1; rot = 2; return;
      }
      if (v0 == face.getVertex(0) && v1 == face.getVertex(2) && v2 == face.getVertex(1)){
        sign = -1; rot = 0; return;
      }
      if (v0 == face.getVertex(1) && v1 == face.getVertex(0) && v2 == face.getVertex(2)){
        sign = -1; rot = 1; return;
      }
      if (v0 == face.getVertex(2) && v1 == face.getVertex(1) && v2 == face.getVertex(0)){
        sign = -1; rot = 2; return;
      }
    }
    else {
      MVertex *v3 = _v[faces_prism(ithFace, 3)];
      if (v0 == face.getVertex(0) && v1 == face.getVertex(1) &&
          v2 == face.getVertex(2) && v3 == face.getVertex(3)){
        sign = 1; rot = 0; return;
      }
      if (v0 == face.getVertex(1) && v1 == face.getVertex(2) &&
          v2 == face.getVertex(3) && v3 == face.getVertex(0)){
        sign = 1; rot = 1; return;
      }
      if (v0 == face.getVertex(2) && v1 == face.getVertex(3) &&
          v2 == face.getVertex(0) && v3 == face.getVertex(1)){
        sign = 1; rot = 2; return;
      }
      if (v0 == face.getVertex(3) && v1 == face.getVertex(0) &&
          v2 == face.getVertex(1) && v3 == face.getVertex(2)){
        sign = 1; rot = 3; return;
      }
      if (v0 == face.getVertex(0) && v1 == face.getVertex(3) &&
          v2 == face.getVertex(2) && v3 == face.getVertex(1)){
        sign = -1; rot = 0; return;
      }
      if (v0 == face.getVertex(1) && v1 == face.getVertex(0) &&
          v2 == face.getVertex(3) && v3 == face.getVertex(2)){
        sign = -1; rot = 1; return;
      }
      if (v0 == face.getVertex(2) && v1 == face.getVertex(1) &&
          v2 == face.getVertex(0) && v3 == face.getVertex(3)){
        sign = -1; rot = 2; return;
      }
      if (v0 == face.getVertex(3) && v1 == face.getVertex(2) &&
          v2 == face.getVertex(1) && v3 == face.getVertex(0)){
        sign = -1; rot = 3; return;
      }
    }

  }
  Msg::Error("Could not get face information for prism %d", getNum());
}

static void _myGetEdgeRep(MPrism *pri, int num, double *x, double *y, double *z,
                          SVector3 *n, int numSubEdges) {

  //const int numSubEdges = CTX::instance()->mesh.numSubEdges;
  static double pp[6][3] = {
    {0,0,-1},{1,0,-1},{0,1,-1},
    {0,0,1},{1,0,1},{0,1,1} };
  static int ed [9][2] = {
    {0,1},{0,2},{0,3},{1,2},{1,4},{2,5},
    {3,4},{3,5},{4,5}
  };
  int iEdge = num / numSubEdges;
  int iSubEdge = num % numSubEdges;

  int iVertex1 = ed [iEdge][0];
  int iVertex2 = ed [iEdge][1];
  double t1 = (double) iSubEdge / (double) numSubEdges;
  double u1 = pp[iVertex1][0] * (1.-t1) + pp[iVertex2][0] * t1;
  double v1 = pp[iVertex1][1] * (1.-t1) + pp[iVertex2][1] * t1;
  double w1 = pp[iVertex1][2] * (1.-t1) + pp[iVertex2][2] * t1;

  double t2 = (double) (iSubEdge+1) / (double) numSubEdges;
  double u2 = pp[iVertex1][0] * (1.-t2) + pp[iVertex2][0] * t2;
  double v2 = pp[iVertex1][1] * (1.-t2) + pp[iVertex2][1] * t2;
  double w2 = pp[iVertex1][2] * (1.-t2) + pp[iVertex2][2] * t2;

  SPoint3 pnt1, pnt2;
  pri->pnt(u1, v1, w1, pnt1);
  pri->pnt(u2, v2, w2, pnt2);
  x[0] = pnt1.x(); x[1] = pnt2.x();
  y[0] = pnt1.y(); y[1] = pnt2.y();
  z[0] = pnt1.z(); z[1] = pnt2.z();

  // not great, but better than nothing
  //static const int f[6] = {0, 0, 0, 1, 2, 3};
  n[0] = n[1] = 1 ;
}


void MPrism15::getEdgeRep(bool curved, int num, double *x, double *y, double *z, SVector3 *n) {
  if (curved) _myGetEdgeRep(this, num, x, y, z, n, CTX::instance()->mesh.numSubEdges);
  else MPrism::getEdgeRep(false, num, x, y, z, n);
}

void MPrism18::getEdgeRep(bool curved, int num, double *x, double *y, double *z, SVector3 *n) {
  if (curved) _myGetEdgeRep(this, num, x, y, z, n, CTX::instance()->mesh.numSubEdges);
  else MPrism::getEdgeRep(false, num, x, y, z, n);
}

void MPrismN::getEdgeRep(bool curved, int num, double *x, double *y, double *z, SVector3 *n) {
  if (curved) _myGetEdgeRep(this, num, x, y, z, n, CTX::instance()->mesh.numSubEdges);
  else MPrism::getEdgeRep(false, num, x, y, z, n);
}

int MPrism15::getNumEdgesRep(bool curved) {
  return curved ? 9 * CTX::instance()->mesh.numSubEdges : 9;
}

int MPrism18::getNumEdgesRep(bool curved) {
  return curved ? 9 * CTX::instance()->mesh.numSubEdges : 9;
}

int MPrismN::getNumEdgesRep(bool curved) {
  return curved ? 9 * CTX::instance()->mesh.numSubEdges : 9;
}


void _myGetFaceRep(MPrism *pri, int num, double *x, double *y, double *z,
                          SVector3 *n, int numSubEdges)
{
  static double pp[6][3] = {
    {0,0,-1},{1,0,-1},{0,1,-1},
    {0,0,1},{1,0,1},{0,1,1} };

  int iFace    = num / (numSubEdges * numSubEdges);
  int iSubFace = num % (numSubEdges * numSubEdges);

  if (iFace > 1) {
    iFace = num / (2*numSubEdges * numSubEdges) + 1;
    iSubFace = num % (2*numSubEdges * numSubEdges);
  }

  int iVertex1 = pri->faces_prism(iFace,0);
  int iVertex2 = pri->faces_prism(iFace,1);
  int iVertex3 = pri->faces_prism(iFace,2);
  int iVertex4 = pri->faces_prism(iFace,3);

  SPoint3 pnt1, pnt2, pnt3;
 // double J1[3][3], J2[3][3], J3[3][3];

  /*
    0
    0 1
    0 1 2
    0 1 2 3
    0 1 2 3 4
    0 1 2 3 4 5
  */

  // on each layer, we have (numSubEdges) * 2 triangles
  // ix and iy are the coordinates of the sub-quadrangle

  if (iFace > 1) {
    int io = iSubFace%2;
    int ix = (iSubFace/2)/numSubEdges;
    int iy = (iSubFace/2)%numSubEdges;

    const double d = 2. / numSubEdges;
    double ox = -1. + d*ix;
    double oy = -1. + d*iy;

    if (io == 0){
      double U1 =
        pp[iVertex1][0] * (1.-ox)*(1-oy)*.25 +
        pp[iVertex2][0] * (1.+ox)*(1-oy)*.25 +
        pp[iVertex3][0] * (1.+ox)*(1+oy)*.25 +
        pp[iVertex4][0] * (1.-ox)*(1+oy)*.25;
      double V1 =
        pp[iVertex1][1] * (1.-ox)*(1-oy)*.25 +
        pp[iVertex2][1] * (1.+ox)*(1-oy)*.25 +
        pp[iVertex3][1] * (1.+ox)*(1+oy)*.25 +
        pp[iVertex4][1] * (1.-ox)*(1+oy)*.25;
      double W1 =
        pp[iVertex1][2] * (1.-ox)*(1-oy)*.25 +
        pp[iVertex2][2] * (1.+ox)*(1-oy)*.25 +
        pp[iVertex3][2] * (1.+ox)*(1+oy)*.25 +
        pp[iVertex4][2] * (1.-ox)*(1+oy)*.25;

      ox += d;

      double U2 =
        pp[iVertex1][0] * (1.-ox)*(1-oy)*.25 +
        pp[iVertex2][0] * (1.+ox)*(1-oy)*.25 +
        pp[iVertex3][0] * (1.+ox)*(1+oy)*.25 +
        pp[iVertex4][0] * (1.-ox)*(1+oy)*.25;
      double V2 =
        pp[iVertex1][1] * (1.-ox)*(1-oy)*.25 +
        pp[iVertex2][1] * (1.+ox)*(1-oy)*.25 +
        pp[iVertex3][1] * (1.+ox)*(1+oy)*.25 +
        pp[iVertex4][1] * (1.-ox)*(1+oy)*.25;
      double W2 =
        pp[iVertex1][2] * (1.-ox)*(1-oy)*.25 +
        pp[iVertex2][2] * (1.+ox)*(1-oy)*.25 +
        pp[iVertex3][2] * (1.+ox)*(1+oy)*.25 +
        pp[iVertex4][2] * (1.-ox)*(1+oy)*.25;

      oy += d;

      double U3 =
        pp[iVertex1][0] * (1.-ox)*(1-oy)*.25 +
        pp[iVertex2][0] * (1.+ox)*(1-oy)*.25 +
        pp[iVertex3][0] * (1.+ox)*(1+oy)*.25 +
        pp[iVertex4][0] * (1.-ox)*(1+oy)*.25;
      double V3 =
        pp[iVertex1][1] * (1.-ox)*(1-oy)*.25 +
        pp[iVertex2][1] * (1.+ox)*(1-oy)*.25 +
        pp[iVertex3][1] * (1.+ox)*(1+oy)*.25 +
        pp[iVertex4][1] * (1.-ox)*(1+oy)*.25;
      double W3 =
        pp[iVertex1][2] * (1.-ox)*(1-oy)*.25 +
        pp[iVertex2][2] * (1.+ox)*(1-oy)*.25 +
        pp[iVertex3][2] * (1.+ox)*(1+oy)*.25 +
        pp[iVertex4][2] * (1.-ox)*(1+oy)*.25;

      pri->pnt(U1, V1, W1, pnt1);
      pri->pnt(U2, V2, W2, pnt2);
      pri->pnt(U3, V3, W3, pnt3);
    }
    else{
      double U1 =
        pp[iVertex1][0] * (1.-ox)*(1-oy)*.25 +
        pp[iVertex2][0] * (1.+ox)*(1-oy)*.25 +
        pp[iVertex3][0] * (1.+ox)*(1+oy)*.25 +
        pp[iVertex4][0] * (1.-ox)*(1+oy)*.25;
      double V1 =
        pp[iVertex1][1] * (1.-ox)*(1-oy)*.25 +
        pp[iVertex2][1] * (1.+ox)*(1-oy)*.25 +
        pp[iVertex3][1] * (1.+ox)*(1+oy)*.25 +
        pp[iVertex4][1] * (1.-ox)*(1+oy)*.25;
      double W1 =
        pp[iVertex1][2] * (1.-ox)*(1-oy)*.25 +
        pp[iVertex2][2] * (1.+ox)*(1-oy)*.25 +
        pp[iVertex3][2] * (1.+ox)*(1+oy)*.25 +
        pp[iVertex4][2] * (1.-ox)*(1+oy)*.25;

      ox += d;
      oy += d;

      double U2 =
        pp[iVertex1][0] * (1.-ox)*(1-oy)*.25 +
        pp[iVertex2][0] * (1.+ox)*(1-oy)*.25 +
        pp[iVertex3][0] * (1.+ox)*(1+oy)*.25 +
        pp[iVertex4][0] * (1.-ox)*(1+oy)*.25;
      double V2 =
        pp[iVertex1][1] * (1.-ox)*(1-oy)*.25 +
        pp[iVertex2][1] * (1.+ox)*(1-oy)*.25 +
        pp[iVertex3][1] * (1.+ox)*(1+oy)*.25 +
        pp[iVertex4][1] * (1.-ox)*(1+oy)*.25;
      double W2 =
        pp[iVertex1][2] * (1.-ox)*(1-oy)*.25 +
        pp[iVertex2][2] * (1.+ox)*(1-oy)*.25 +
        pp[iVertex3][2] * (1.+ox)*(1+oy)*.25 +
        pp[iVertex4][2] * (1.-ox)*(1+oy)*.25;

      ox -= d;

      double U3 =
        pp[iVertex1][0] * (1.-ox)*(1-oy)*.25 +
        pp[iVertex2][0] * (1.+ox)*(1-oy)*.25 +
        pp[iVertex3][0] * (1.+ox)*(1+oy)*.25 +
        pp[iVertex4][0] * (1.-ox)*(1+oy)*.25;
      double V3 =
        pp[iVertex1][1] * (1.-ox)*(1-oy)*.25 +
        pp[iVertex2][1] * (1.+ox)*(1-oy)*.25 +
        pp[iVertex3][1] * (1.+ox)*(1+oy)*.25 +
        pp[iVertex4][1] * (1.-ox)*(1+oy)*.25;
      double W3 =
        pp[iVertex1][2] * (1.-ox)*(1-oy)*.25 +
        pp[iVertex2][2] * (1.+ox)*(1-oy)*.25 +
        pp[iVertex3][2] * (1.+ox)*(1+oy)*.25 +
        pp[iVertex4][2] * (1.-ox)*(1+oy)*.25;

      pri->pnt(U1, V1, W1, pnt1);
      pri->pnt(U2, V2, W2, pnt2);
      pri->pnt(U3, V3, W3, pnt3);
    }

  } else {
    int ix = 0, iy = 0;
    int nbt = 0;
    for (int i = 0; i < numSubEdges; i++){
      int nbl = (numSubEdges - i - 1) * 2 + 1;
      nbt += nbl;
      if (nbt > iSubFace){
        iy = i;
        ix = nbl - (nbt - iSubFace);
        break;
      }
    }

    const double d = 1. / numSubEdges;

    double u1, v1, u2, v2, u3, v3;
    if (ix % 2 == 0){
      u1 = ix / 2 * d; v1= iy*d;
      u2 = (ix / 2 + 1) * d ; v2 =  iy * d;
      u3 = ix / 2 * d ; v3 =  (iy+1) * d;
    }
    else{
      u1 = (ix / 2 + 1) * d; v1= iy * d;
      u2 = (ix / 2 + 1) * d; v2= (iy + 1) * d;
      u3 = ix / 2 * d ; v3 =  (iy + 1) * d;
    }

    double U1 = pp[iVertex1][0] * (1.-u1-v1) + pp[iVertex2][0] * u1 + pp[iVertex3][0] * v1;
    double U2 = pp[iVertex1][0] * (1.-u2-v2) + pp[iVertex2][0] * u2 + pp[iVertex3][0] * v2;
    double U3 = pp[iVertex1][0] * (1.-u3-v3) + pp[iVertex2][0] * u3 + pp[iVertex3][0] * v3;

    double V1 = pp[iVertex1][1] * (1.-u1-v1) + pp[iVertex2][1] * u1 + pp[iVertex3][1] * v1;
    double V2 = pp[iVertex1][1] * (1.-u2-v2) + pp[iVertex2][1] * u2 + pp[iVertex3][1] * v2;
    double V3 = pp[iVertex1][1] * (1.-u3-v3) + pp[iVertex2][1] * u3 + pp[iVertex3][1] * v3;

    double W1 = pp[iVertex1][2] * (1.-u1-v1) + pp[iVertex2][2] * u1 + pp[iVertex3][2] * v1;
    double W2 = pp[iVertex1][2] * (1.-u2-v2) + pp[iVertex2][2] * u2 + pp[iVertex3][2] * v2;
    double W3 = pp[iVertex1][2] * (1.-u3-v3) + pp[iVertex2][2] * u3 + pp[iVertex3][2] * v3;

    pri->pnt(U1, V1, W1, pnt1);
    pri->pnt(U2, V2, W2, pnt2);
    pri->pnt(U3, V3, W3, pnt3);
  }



  x[0] = pnt1.x(); x[1] = pnt2.x(); x[2] = pnt3.x();
  y[0] = pnt1.y(); y[1] = pnt2.y(); y[2] = pnt3.y();
  z[0] = pnt1.z(); z[1] = pnt2.z(); z[2] = pnt3.z();

  SVector3 d1(x[1] - x[0], y[1] - y[0], z[1] - z[0]);
  SVector3 d2(x[2] - x[0], y[2] - y[0], z[2] - z[0]);
  n[0] = crossprod(d1, d2);
  n[0].normalize();
  n[1] = n[0];
  n[2] = n[0];
}

void MPrism15::getFaceRep(bool curved, int num, double *x, double *y, double *z, SVector3 *n)
{
  if (curved) _myGetFaceRep(this, num, x, y, z, n, CTX::instance()->mesh.numSubEdges);
  else MPrism::getFaceRep(false, num, x, y, z, n);
}

void MPrism18::getFaceRep(bool curved, int num, double *x, double *y, double *z, SVector3 *n)
{
  if (curved) _myGetFaceRep(this, num, x, y, z, n, CTX::instance()->mesh.numSubEdges);
  else MPrism::getFaceRep(false, num, x, y, z, n);
}

void MPrismN::getFaceRep(bool curved, int num, double *x, double *y, double *z, SVector3 *n)
{
  if (curved) _myGetFaceRep(this, num, x, y, z, n, CTX::instance()->mesh.numSubEdges);
  else MPrism::getFaceRep(false, num, x, y, z, n);
}

int MPrism15::getNumFacesRep(bool curved) {
  return curved ? 4 * (CTX::instance()->mesh.numSubEdges * CTX::instance()->mesh.numSubEdges * 2) : 8;
}

int MPrism18::getNumFacesRep(bool curved) {
  return curved ? 4 * (CTX::instance()->mesh.numSubEdges * CTX::instance()->mesh.numSubEdges * 2) : 8;
}

int MPrismN::getNumFacesRep(bool curved) {
  return curved ? 4 * (CTX::instance()->mesh.numSubEdges * CTX::instance()->mesh.numSubEdges * 2) : 8;
}

namespace {

void _addEdgeNodes(int num, bool reverse, int order, const std::vector<MVertex*> &vs,
                   int &ind, std::vector<MVertex*> &v)
{

  const int nNode = order-1, startNode = num*nNode, endNode = startNode+nNode-1;

  if (reverse)
    for (int i=endNode; i>=startNode; i--, ind++) v[ind] = vs[i];
  else
    for (int i=startNode; i<=endNode; i++, ind++) v[ind] = vs[i];

}

void _addFaceNodes(int num, int order, const std::vector<MVertex*> &vs,
                   int &ind, std::vector<MVertex*> &v)
{

  const int nNodeEd = order-1, nNodeTri = (order-2)*(order-1)/2;

  int startNode, endNode;
  if (num < 2) {
    startNode = 9*nNodeEd+num*nNodeTri;
    endNode = startNode+nNodeTri;
  }
  else {
    const int nNodeQuad = (order-1)*(order-1);
    startNode = 9*nNodeEd+2*nNodeTri+(num-2)*nNodeQuad;
    endNode = startNode+nNodeQuad;
  }

  for (int i=startNode; i<endNode; i++, ind++) v[ind] = vs[i];

}

}

// To be tested
void MPrismN::getFaceVertices(const int num, std::vector<MVertex*> &v) const
{
  // FIXME serendipity case
  static const int edge[5][4] = {
      {1, 3, 0, -1},
      {6, 8, 7, -1},
      {0, 4, 6, 2},
      {2, 7, 5, 1},
      {3, 5, 8, 4}
  };
  static const bool reverse[5][4] = {
      {false, true, true, false},
      {false, false, true, false},
      {false, false, true, true},
      {false, false, true, true},
      {false, false, true, true}
  };

  int nNodeTotal, nEdge;
  if (num < 2) {                            // Triangular face
    nNodeTotal = (_order+1)*(_order+2)/2;
    nEdge = 3;
  }
  else {                                    // Quad face
    nNodeTotal = (_order+1)*(_order+1);
    nEdge = 4;
  }

  v.resize(nNodeTotal);

  // Add corner nodes (there are nEdge corner nodes)
  MPrism::_getFaceVertices(num, v);
  int ind = nEdge;

  // Add edge nodes
  for (int iE=0; iE<nEdge; iE++) _addEdgeNodes(edge[num][iE],reverse[num][iE],_order,_vs,ind,v);

  // Add face nodes
  _addFaceNodes(num,_order,_vs,ind,v);

}
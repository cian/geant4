//
// ********************************************************************
// * DISCLAIMER                                                       *
// *                                                                  *
// * The following disclaimer summarizes all the specific disclaimers *
// * of contributors to this software. The specific disclaimers,which *
// * govern, are listed with their locations in:                      *
// *   http://cern.ch/geant4/license                                  *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.                                                             *
// *                                                                  *
// * This  code  implementation is the  intellectual property  of the *
// * GEANT4 collaboration.                                            *
// * By copying,  distributing  or modifying the Program (or any work *
// * based  on  the Program)  you indicate  your  acceptance of  this *
// * statement, and all its terms.                                    *
// ********************************************************************
//
//
// $Id: HepPolyhedron.cc,v 1.12 2002/11/20 14:18:34 gcosmo Exp $
// GEANT4 tag $Name: geant4-05-02 $
//
// 
//
// G4 Polyhedron library
//
// History:
// 23.07.96 E.Chernyaev <Evgueni.Tcherniaev@cern.ch> - initial version
//
// 30.09.96 E.Chernyaev
// - added GetNextVertexIndex, GetVertex by Yasuhide Sawada
// - added GetNextUnitNormal, GetNextEdgeIndeces, GetNextEdge
//
// 15.12.96 E.Chernyaev
// - added GetNumberOfRotationSteps, RotateEdge, RotateAroundZ, SetReferences
// - rewritten G4PolyhedronCons;
// - added G4PolyhedronPara, ...Trap, ...Pgon, ...Pcon, ...Sphere, ...Torus
//
// 01.06.97 E.Chernyaev
// - modified RotateAroundZ, added SetSideFacets
//
// 19.03.00 E.Chernyaev
// - implemented boolean operations (add, subtract, intersect) on polyhedra;
//
// 25.05.01 E.Chernyaev
// - added GetSurfaceArea() and GetVolume();
//
// 05.11.02 E.Chernyaev
// - added createTwistedTrap() and createPolyhedron();
//
  
#include "HepPolyhedron.h"
#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/config/TemplateFunctions.h>

/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron operator <<                   Date:    09.05.96 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: Print contents of G4 polyhedron                           *
 *                                                                     *
 ***********************************************************************/
HepStd::ostream & operator<<(HepStd::ostream & ostr, const G4Facet & facet) {
  for (int k=0; k<4; k++) {
    ostr << " " << facet.edge[k].v << "/" << facet.edge[k].f;
  }
  return ostr;
}

HepStd::ostream & operator<<(HepStd::ostream & ostr, const HepPolyhedron & ph) {
  ostr << HepStd::endl;
  ostr << "Nverteces=" << ph.nvert << ", Nfacets=" << ph.nface << HepStd::endl;
  int i;
  for (i=1; i<=ph.nvert; i++) {
     ostr << "xyz(" << i << ")="
	  << ph.pV[i].x() << ' ' << ph.pV[i].y() << ' ' << ph.pV[i].z()
	  << HepStd::endl;
  }
  for (i=1; i<=ph.nface; i++) {
    ostr << "face(" << i << ")=" << ph.pF[i] << HepStd::endl;
  }
  return ostr;
}

HepPolyhedron::HepPolyhedron(const HepPolyhedron &from)
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron copy constructor             Date:    23.07.96  *
 * Author: E.Chernyaev (IHEP/Protvino)              Revised:           *
 *                                                                     *
 ***********************************************************************/
: nvert(0), nface(0), pV(0), pF(0)
{
  AllocateMemory(from.nvert, from.nface);
  for (int i=1; i<=nvert; i++) pV[i] = from.pV[i];
  for (int k=1; k<=nface; k++) pF[k] = from.pF[k];
}

HepPolyhedron & HepPolyhedron::operator=(const HepPolyhedron &from)
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron operator =                   Date:    23.07.96  *
 * Author: E.Chernyaev (IHEP/Protvino)              Revised:           *
 *                                                                     *
 * Function: Copy contents of one polyhedron to another                *
 *                                                                     *
 ***********************************************************************/
{
  if (this != &from) {
    AllocateMemory(from.nvert, from.nface);
    for (int i=1; i<=nvert; i++) pV[i] = from.pV[i];
    for (int k=1; k<=nface; k++) pF[k] = from.pF[k];
  }
  return *this;
}

int
HepPolyhedron::FindNeighbour(int iFace, int iNode, int iOrder) const
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::FindNeighbour                Date:    22.11.99 *
 * Author: E.Chernyaev                               Revised:          *
 *                                                                     *
 * Function: Find neighbouring face                                    *
 *                                                                     *
 ***********************************************************************/
{
  int i;
  for (i=0; i<4; i++) {
    if (iNode == abs(pF[iFace].edge[i].v)) break;
  }
  if (i == 4) {
    HepStd::cerr
      << "HepPolyhedron::FindNeighbour: face " << iFace
      << " has no node " << iNode
      << HepStd::endl; 
    return 0;
  }
  if (iOrder < 0) {
    if ( --i < 0) i = 3;
    if (pF[iFace].edge[i].v == 0) i = 2;
  }
  return (pF[iFace].edge[i].v > 0) ? 0 : pF[iFace].edge[i].f;
}

HepNormal3D HepPolyhedron::FindNodeNormal(int iFace, int iNode) const
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::FindNodeNormal               Date:    22.11.99 *
 * Author: E.Chernyaev                               Revised:          *
 *                                                                     *
 * Function: Find normal at given node                                 *
 *                                                                     *
 ***********************************************************************/
{
  HepNormal3D  normal = GetUnitNormal(iFace);
  int          k = iFace, iOrder = 1, n = 1;

  for(;;) {
    k = FindNeighbour(k, iNode, iOrder);
    if (k == iFace) break; 
    if (k > 0) {
      n++;
      normal += GetUnitNormal(k);
    }else{
      if (iOrder < 0) break;
      k = iFace;
      iOrder = -iOrder;
    }
  }
  return normal.unit();
}

int HepPolyhedron::GetNumberOfRotationSteps()
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::GetNumberOfRotationSteps     Date:    24.06.97 *
 * Author: J.Allison (Manchester University)         Revised:          *
 *                                                                     *
 * Function: Get number of steps for whole circle                      *
 *                                                                     *
 ***********************************************************************/
{
  return fNumberOfRotationSteps;
}

void HepPolyhedron::SetNumberOfRotationSteps(int n)
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::SetNumberOfRotationSteps     Date:    24.06.97 *
 * Author: J.Allison (Manchester University)         Revised:          *
 *                                                                     *
 * Function: Set number of steps for whole circle                      *
 *                                                                     *
 ***********************************************************************/
{
  const int nMin = 3;
  if (n < nMin) {
    HepStd::cerr 
      << "HepPolyhedron::SetNumberOfRotationSteps: attempt to set the\n"
      << "number of steps per circle < " << nMin << "; forced to " << nMin
      << HepStd::endl;
    fNumberOfRotationSteps = nMin;
  }else{
    fNumberOfRotationSteps = n;
  }    
}

void HepPolyhedron::ResetNumberOfRotationSteps()
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::GetNumberOfRotationSteps     Date:    24.06.97 *
 * Author: J.Allison (Manchester University)         Revised:          *
 *                                                                     *
 * Function: Reset number of steps for whole circle to default value   *
 *                                                                     *
 ***********************************************************************/
{
  fNumberOfRotationSteps = DEFAULT_NUMBER_OF_STEPS;
}

void HepPolyhedron::AllocateMemory(int Nvert, int Nface)
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::AllocateMemory               Date:    19.06.96 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised: 05.11.02 *
 *                                                                     *
 * Function: Allocate memory for GEANT4 polyhedron                     *
 *                                                                     *
 * Input: Nvert - number of nodes                                      *
 *        Nface - number of faces                                      *
 *                                                                     *
 ***********************************************************************/
{
  if (nvert == Nvert && nface == Nface) return;
  if (pV != 0) delete [] pV;
  if (pF != 0) delete [] pF;
  if (Nvert > 0 && Nface > 0) {
    nvert = Nvert;
    nface = Nface;
    pV    = new HepPoint3D[nvert+1];
    pF    = new G4Facet[nface+1];
  }else{
    nvert = 0; nface = 0; pV = 0; pF = 0;
  }
}

void HepPolyhedron::CreatePrism()
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::CreatePrism                  Date:    15.07.96 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: Set facets for a prism                                    *
 *                                                                     *
 ***********************************************************************/
{
  enum {DUMMY, BOTTOM, LEFT, BACK, RIGHT, FRONT, TOP};

  pF[1] = G4Facet(1,LEFT,  4,BACK,  3,RIGHT,  2,FRONT);
  pF[2] = G4Facet(5,TOP,   8,BACK,  4,BOTTOM, 1,FRONT);
  pF[3] = G4Facet(8,TOP,   7,RIGHT, 3,BOTTOM, 4,LEFT);
  pF[4] = G4Facet(7,TOP,   6,FRONT, 2,BOTTOM, 3,BACK);
  pF[5] = G4Facet(6,TOP,   5,LEFT,  1,BOTTOM, 2,RIGHT);
  pF[6] = G4Facet(5,FRONT, 6,RIGHT, 7,BACK,   8,LEFT);
}

void HepPolyhedron::RotateEdge(int k1, int k2, double r1, double r2,
			      int v1, int v2, int vEdge,
                              bool ifWholeCircle, int ns, int &kface)
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::RotateEdge                   Date:    05.12.96 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: Create set of facets by rotation of an edge around Z-axis *
 *                                                                     *
 * Input: k1, k2 - end vertices of the edge                            *
 *        r1, r2 - radiuses of the end vertices                        *
 *        v1, v2 - visibility of edges produced by rotation of the end *
 *                 vertices                                            *
 *        vEdge  - visibility of the edge                              *
 *        ifWholeCircle - is true in case of whole circle rotation     *
 *        ns     - number of discrete steps                            *
 *        r[]    - r-coordinates                                       *
 *        kface  - current free cell in the pF array                   *
 *                                                                     *
 ***********************************************************************/
{
  if (r1 == 0. && r2 == 0) return;

  int i;
  int i1  = k1;
  int i2  = k2;
  int ii1 = ifWholeCircle ? i1 : i1+ns;
  int ii2 = ifWholeCircle ? i2 : i2+ns;
  int vv  = ifWholeCircle ? vEdge : 1;

  if (ns == 1) {
    if (r1 == 0.) {
      pF[kface++]   = G4Facet(i1,0,    v2*i2,0, (i2+1),0);
    }else if (r2 == 0.) {
      pF[kface++]   = G4Facet(i1,0,    i2,0,    v1*(i1+1),0);
    }else{
      pF[kface++]   = G4Facet(i1,0,    v2*i2,0, (i2+1),0, v1*(i1+1),0);
    }
  }else{
    if (r1 == 0.) {
      pF[kface++]   = G4Facet(vv*i1,0,    v2*i2,0, vEdge*(i2+1),0);
      for (i2++,i=1; i<ns-1; i2++,i++) {
	pF[kface++] = G4Facet(vEdge*i1,0, v2*i2,0, vEdge*(i2+1),0);
      }
      pF[kface++]   = G4Facet(vEdge*i1,0, v2*i2,0, vv*ii2,0);
    }else if (r2 == 0.) {
      pF[kface++]   = G4Facet(vv*i1,0,    vEdge*i2,0, v1*(i1+1),0);
      for (i1++,i=1; i<ns-1; i1++,i++) {
	pF[kface++] = G4Facet(vEdge*i1,0, vEdge*i2,0, v1*(i1+1),0);
      }
      pF[kface++]   = G4Facet(vEdge*i1,0, vv*i2,0,    v1*ii1,0);
    }else{
      pF[kface++]   = G4Facet(vv*i1,0,    v2*i2,0, vEdge*(i2+1),0,v1*(i1+1),0);
      for (i1++,i2++,i=1; i<ns-1; i1++,i2++,i++) {
	pF[kface++] = G4Facet(vEdge*i1,0, v2*i2,0, vEdge*(i2+1),0,v1*(i1+1),0);
      }  
      pF[kface++]   = G4Facet(vEdge*i1,0, v2*i2,0, vv*ii2,0,      v1*ii1,0);
    }
  }
}

void HepPolyhedron::SetSideFacets(int ii[4], int vv[4],
				 int *kk, double *r,
                                 double dphi, int ns, int &kface)
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::SetSideFacets                Date:    20.05.97 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: Set side facets for the case of incomplete rotation       *
 *                                                                     *
 * Input: ii[4] - indeces of original verteces                         *
 *        vv[4] - visibility of edges                                  *
 *        kk[]  - indeces of nodes                                     *
 *        r[]   - radiuses                                             *
 *        dphi  - delta phi                                            *
 *        ns     - number of discrete steps                            *
 *        kface  - current free cell in the pF array                   *
 *                                                                     *
 ***********************************************************************/
{
  int k1, k2, k3, k4;
  
  if (abs((double)(dphi-M_PI)) < perMillion) {          // half a circle
    for (int i=0; i<4; i++) {
      k1 = ii[i];
      k2 = (i == 3) ? ii[0] : ii[i+1];
      if (r[k1] == 0. && r[k2] == 0.) vv[i] = -1;      
    }
  }

  if (ii[1] == ii[2]) {
    k1 = kk[ii[0]];
    k2 = kk[ii[2]];
    k3 = kk[ii[3]];
    pF[kface++] = G4Facet(vv[0]*k1,0, vv[2]*k2,0, vv[3]*k3,0);
    if (r[ii[0]] != 0.) k1 += ns;
    if (r[ii[2]] != 0.) k2 += ns;
    if (r[ii[3]] != 0.) k3 += ns;
    pF[kface++] = G4Facet(vv[2]*k3,0, vv[0]*k2,0, vv[3]*k1,0);
  }else if (kk[ii[0]] == kk[ii[1]]) {
    k1 = kk[ii[0]];
    k2 = kk[ii[2]];
    k3 = kk[ii[3]];
    pF[kface++] = G4Facet(vv[1]*k1,0, vv[2]*k2,0, vv[3]*k3,0);
    if (r[ii[0]] != 0.) k1 += ns;
    if (r[ii[2]] != 0.) k2 += ns;
    if (r[ii[3]] != 0.) k3 += ns;
    pF[kface++] = G4Facet(vv[2]*k3,0, vv[1]*k2,0, vv[3]*k1,0);
  }else if (kk[ii[2]] == kk[ii[3]]) {
    k1 = kk[ii[0]];
    k2 = kk[ii[1]];
    k3 = kk[ii[2]];
    pF[kface++] = G4Facet(vv[0]*k1,0, vv[1]*k2,0, vv[3]*k3,0);
    if (r[ii[0]] != 0.) k1 += ns;
    if (r[ii[1]] != 0.) k2 += ns;
    if (r[ii[2]] != 0.) k3 += ns;
    pF[kface++] = G4Facet(vv[1]*k3,0, vv[0]*k2,0, vv[3]*k1,0);
  }else{
    k1 = kk[ii[0]];
    k2 = kk[ii[1]];
    k3 = kk[ii[2]];
    k4 = kk[ii[3]];
    pF[kface++] = G4Facet(vv[0]*k1,0, vv[1]*k2,0, vv[2]*k3,0, vv[3]*k4,0);
    if (r[ii[0]] != 0.) k1 += ns;
    if (r[ii[1]] != 0.) k2 += ns;
    if (r[ii[2]] != 0.) k3 += ns;
    if (r[ii[3]] != 0.) k4 += ns;
    pF[kface++] = G4Facet(vv[2]*k4,0, vv[1]*k3,0, vv[0]*k2,0, vv[3]*k1,0);
  }
}

void HepPolyhedron::RotateAroundZ(int nstep, double phi, double dphi,
                                 int np1, int np2,
				 const double *z, double *r,
				 int nodeVis, int edgeVis)
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::RotateAroundZ                Date:    27.11.96 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: Create HepPolyhedron for a solid produced by rotation of  *
 *           two polylines around Z-axis                               *
 *                                                                     *
 * Input: nstep - number of discrete steps, if 0 then default          *
 *        phi   - starting phi angle                                   *
 *        dphi  - delta phi                                            *
 *        np1   - number of points in external polyline                *
 *                (must be negative in case of closed polyline)        *
 *        np2   - number of points in internal polyline (may be 1)     *
 *        z[]   - z-coordinates (+z >>> -z for both polylines)         *
 *        r[]   - r-coordinates                                        *
 *        nodeVis - how to Draw edges joing consecutive positions of   *
 *                  node during rotation                               *
 *        edgeVis - how to Draw edges                                  *
 *                                                                     *
 ***********************************************************************/
{
  static double wholeCircle   = 2*M_PI;
    
  //   S E T   R O T A T I O N   P A R A M E T E R S

  bool ifWholeCircle = (abs(dphi-wholeCircle) < perMillion) ? true : false;
  double   delPhi  = ifWholeCircle ? wholeCircle : dphi;  
  int        nSphi    = (nstep > 0) ?
    nstep : int(delPhi*GetNumberOfRotationSteps()/wholeCircle+.5);
  if (nSphi == 0) nSphi = 1;
  int        nVphi    = ifWholeCircle ? nSphi : nSphi+1;
  bool ifClosed = np1 > 0 ? false : true;
  
  //   C O U N T   V E R T E C E S

  int absNp1 = abs(np1);
  int absNp2 = abs(np2);
  int i1beg = 0;
  int i1end = absNp1-1;
  int i2beg = absNp1;
  int i2end = absNp1+absNp2-1; 
  int i, j, k;

  for(i=i1beg; i<=i2end; i++) {
    if (abs(r[i]) < perMillion) r[i] = 0.;
  }

  j = 0;                                                // external nodes
  for (i=i1beg; i<=i1end; i++) {
    j += (r[i] == 0.) ? 1 : nVphi;
  }

  bool ifSide1 = false;                           // internal nodes
  bool ifSide2 = false;

  if (r[i2beg] != r[i1beg] || z[i2beg] != z[i1beg]) {
    j += (r[i2beg] == 0.) ? 1 : nVphi;
    ifSide1 = true;
  }

  for(i=i2beg+1; i<i2end; i++) {
    j += (r[i] == 0.) ? 1 : nVphi;
  }
  
  if (r[i2end] != r[i1end] || z[i2end] != z[i1end]) {
    if (absNp2 > 1) j += (r[i2end] == 0.) ? 1 : nVphi;
    ifSide2 = true;
  }

  //   C O U N T   F A C E S

  k = ifClosed ? absNp1*nSphi : (absNp1-1)*nSphi;       // external faces

  if (absNp2 > 1) {                                     // internal faces
    for(i=i2beg; i<i2end; i++) {
      if (r[i] > 0. || r[i+1] > 0.)       k += nSphi;
    }

    if (ifClosed) {
      if (r[i2end] > 0. || r[i2beg] > 0.) k += nSphi;
    }
  }

  if (!ifClosed) {                                      // side faces
    if (ifSide1 && (r[i1beg] > 0. || r[i2beg] > 0.)) k += nSphi;
    if (ifSide2 && (r[i1end] > 0. || r[i2end] > 0.)) k += nSphi;
  }

  if (!ifWholeCircle) {                                 // phi_side faces
    k += ifClosed ? 2*absNp1 : 2*(absNp1-1);
  }

  //   A L L O C A T E   M E M O R Y

  AllocateMemory(j, k);

  //   G E N E R A T E   V E R T E C E S

  int *kk;
  kk = new int[absNp1+absNp2];

  k = 1;
  for(i=i1beg; i<=i1end; i++) {
    kk[i] = k;
    if (r[i] == 0.) { pV[k++] = HepPoint3D(0, 0, z[i]); } else { k += nVphi; }
  }

  i = i2beg;
  if (ifSide1) {
    kk[i] = k;
    if (r[i] == 0.) { pV[k++] = HepPoint3D(0, 0, z[i]); } else { k += nVphi; }
  }else{
    kk[i] = kk[i1beg];
  }

  for(i=i2beg+1; i<i2end; i++) {
    kk[i] = k;
    if (r[i] == 0.) { pV[k++] = HepPoint3D(0, 0, z[i]); } else { k += nVphi; }
  }

  if (absNp2 > 1) {
    i = i2end;
    if (ifSide2) {
      kk[i] = k;
      if (r[i] == 0.) pV[k] = HepPoint3D(0, 0, z[i]);
    }else{
      kk[i] = kk[i1end];
    }
  }

  double cosPhi, sinPhi;

  for(j=0; j<nVphi; j++) {
    cosPhi = cos(phi+j*delPhi/nSphi);
    sinPhi = sin(phi+j*delPhi/nSphi);
    for(i=i1beg; i<=i2end; i++) {
      if (r[i] != 0.) pV[kk[i]+j] = HepPoint3D(r[i]*cosPhi,r[i]*sinPhi,z[i]);
    }
  }

  //   G E N E R A T E   E X T E R N A L   F A C E S

  int v1,v2;

  k = 1;
  v2 = ifClosed ? nodeVis : 1;
  for(i=i1beg; i<i1end; i++) {
    v1 = v2;
    if (!ifClosed && i == i1end-1) {
      v2 = 1;
    }else{
      v2 = (r[i] == r[i+1] && r[i+1] == r[i+2]) ? -1 : nodeVis;
    }
    RotateEdge(kk[i], kk[i+1], r[i], r[i+1], v1, v2,
	       edgeVis, ifWholeCircle, nSphi, k);
  }
  if (ifClosed) {
    RotateEdge(kk[i1end], kk[i1beg], r[i1end],r[i1beg], nodeVis, nodeVis,
	       edgeVis, ifWholeCircle, nSphi, k);
  }

  //   G E N E R A T E   I N T E R N A L   F A C E S

  if (absNp2 > 1) {
    v2 = ifClosed ? nodeVis : 1;
    for(i=i2beg; i<i2end; i++) {
      v1 = v2;
      if (!ifClosed && i==i2end-1) {
	v2 = 1;
      }else{
	v2 = (r[i] == r[i+1] && r[i+1] == r[i+2]) ? -1 :  nodeVis;
      }
      RotateEdge(kk[i+1], kk[i], r[i+1], r[i], v2, v1,
		 edgeVis, ifWholeCircle, nSphi, k);
    }
    if (ifClosed) {
      RotateEdge(kk[i2beg], kk[i2end], r[i2beg], r[i2end], nodeVis, nodeVis,
		 edgeVis, ifWholeCircle, nSphi, k);
    }
  }

  //   G E N E R A T E   S I D E   F A C E S

  if (!ifClosed) {
    if (ifSide1) {
      RotateEdge(kk[i2beg], kk[i1beg], r[i2beg], r[i1beg], 1, 1,
		 -1, ifWholeCircle, nSphi, k);
    }
    if (ifSide2) {
      RotateEdge(kk[i1end], kk[i2end], r[i1end], r[i2end], 1, 1,
		 -1, ifWholeCircle, nSphi, k);
    }
  }

  //   G E N E R A T E   S I D E   F A C E S  for the case of incomplete circle

  if (!ifWholeCircle) {

    int  ii[4], vv[4];

    if (ifClosed) {
      for (i=i1beg; i<=i1end; i++) {
	ii[0] = i;
	ii[3] = (i == i1end) ? i1beg : i+1;
	ii[1] = (absNp2 == 1) ? i2beg : ii[0]+absNp1;
	ii[2] = (absNp2 == 1) ? i2beg : ii[3]+absNp1;
	vv[0] = -1;
	vv[1] = 1;
	vv[2] = -1;
	vv[3] = 1;
	SetSideFacets(ii, vv, kk, r, dphi, nSphi, k);
      }
    }else{
      for (i=i1beg; i<i1end; i++) {
	ii[0] = i;
	ii[3] = i+1;
	ii[1] = (absNp2 == 1) ? i2beg : ii[0]+absNp1;
	ii[2] = (absNp2 == 1) ? i2beg : ii[3]+absNp1;
	vv[0] = (i == i1beg)   ? 1 : -1;
	vv[1] = 1;
	vv[2] = (i == i1end-1) ? 1 : -1;
	vv[3] = 1;
	SetSideFacets(ii, vv, kk, r, dphi, nSphi, k);
      }
    }      
  }

  delete [] kk;

  if (k-1 != nface) {
    HepStd::cerr
      << "Polyhedron::RotateAroundZ: number of generated faces ("
      << k-1 << ") is not equal to the number of allocated faces ("
      << nface << ")"
      << HepStd::endl;
  }
}

void HepPolyhedron::SetReferences()
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::SetReferences                Date:    04.12.96 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: For each edge set reference to neighbouring facet         *
 *                                                                     *
 ***********************************************************************/
{
  if (nface <= 0) return;

  struct edgeListMember {
    edgeListMember *next;
    int v2;
    int iface;
    int iedge;
  } *edgeList, *freeList, **headList;

  
  //   A L L O C A T E   A N D   I N I T I A T E   L I S T S

  edgeList = new edgeListMember[2*nface];
  headList = new edgeListMember*[nvert];
  
  int i;
  for (i=0; i<nvert; i++) {
    headList[i] = 0;
  }
  freeList = edgeList;
  for (i=0; i<2*nface-1; i++) {
    edgeList[i].next = &edgeList[i+1];
  }
  edgeList[2*nface-1].next = 0;

  //   L O O P   A L O N G   E D G E S

  int iface, iedge, nedge, i1, i2, k1, k2;
  edgeListMember *prev, *cur;
  
  for(iface=1; iface<=nface; iface++) {
    nedge = (pF[iface].edge[3].v == 0) ? 3 : 4;
    for (iedge=0; iedge<nedge; iedge++) {
      i1 = iedge;
      i2 = (iedge < nedge-1) ? iedge+1 : 0;
      i1 = abs(pF[iface].edge[i1].v);
      i2 = abs(pF[iface].edge[i2].v);
      k1 = (i1 < i2) ? i1 : i2;          // k1 = ::min(i1,i2);
      k2 = (i1 > i2) ? i1 : i2;          // k2 = ::max(i1,i2);
      
      // check head of the List corresponding to k1
      cur = headList[k1];
      if (cur == 0) {
	headList[k1] = freeList;
	freeList = freeList->next;
        cur = headList[k1];
	cur->next = 0;
	cur->v2 = k2;
	cur->iface = iface;
	cur->iedge = iedge;
        continue;
      }	

      if (cur->v2 == k2) {
        headList[k1] = cur->next;
	cur->next = freeList;
        freeList = cur;      
        pF[iface].edge[iedge].f = cur->iface;
	pF[cur->iface].edge[cur->iedge].f = iface;
        i1 = (pF[iface].edge[iedge].v < 0) ? -1 : 1;
	i2 = (pF[cur->iface].edge[cur->iedge].v < 0) ? -1 : 1;
        if (i1 != i2) {
	  HepStd::cerr
	    << "Polyhedron::SetReferences: different edge visibility "
	    << iface << "/" << iedge << "/"
	    << pF[iface].edge[iedge].v << " and "
	    << cur->iface << "/" << cur->iedge << "/"
	    << pF[cur->iface].edge[cur->iedge].v
	    << HepStd::endl;
	}
	continue;
      }

      // check List itself
      for (;;) {
	prev = cur;
	cur = prev->next;
	if (cur == 0) {
	  prev->next = freeList;
	  freeList = freeList->next;
	  cur = prev->next;
	  cur->next = 0;
	  cur->v2 = k2;
	  cur->iface = iface;
	  cur->iedge = iedge;
	  break;
	}

        if (cur->v2 == k2) {
	  prev->next = cur->next;
	  cur->next = freeList;
	  freeList = cur;      
	  pF[iface].edge[iedge].f = cur->iface;
	  pF[cur->iface].edge[cur->iedge].f = iface;
	  i1 = (pF[iface].edge[iedge].v < 0) ? -1 : 1;
	  i2 = (pF[cur->iface].edge[cur->iedge].v < 0) ? -1 : 1;
	    if (i1 != i2) {
	      HepStd::cerr
		<< "Polyhedron::SetReferences: different edge visibility "
		<< iface << "/" << iedge << "/"
		<< pF[iface].edge[iedge].v << " and "
		<< cur->iface << "/" << cur->iedge << "/"
		<< pF[cur->iface].edge[cur->iedge].v
		<< HepStd::endl;
	    }
	  break;
	}
      }
    }
  }

  //  C H E C K   T H A T   A L L   L I S T S   A R E   E M P T Y

  for (i=0; i<nvert; i++) {
    if (headList[i] != 0) {
      HepStd::cerr
	<< "Polyhedron::SetReferences: List " << i << " is not empty"
	<< HepStd::endl;
    }
  }

  //   F R E E   M E M O R Y

  delete [] edgeList;
  delete [] headList;
}

void HepPolyhedron::InvertFacets()
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::InvertFacets                Date:    01.12.99  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Invert the order of the nodes in the facets               *
 *                                                                     *
 ***********************************************************************/
{
  if (nface <= 0) return;
  int i, k, nnode, v[4],f[4];
  for (i=1; i<=nface; i++) {
    nnode =  (pF[i].edge[3].v == 0) ? 3 : 4;
    for (k=0; k<nnode; k++) {
      v[k] = (k+1 == nnode) ? pF[i].edge[0].v : pF[i].edge[k+1].v;
      if (v[k] * pF[i].edge[k].v < 0) v[k] = -v[k];
      f[k] = pF[i].edge[k].f;
    }
    for (k=0; k<nnode; k++) {
      pF[i].edge[nnode-1-k].v = v[k];
      pF[i].edge[nnode-1-k].f = f[k];
    }
  }
}

HepPolyhedron & HepPolyhedron::Transform(const HepTransform3D &t)
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::Transform                    Date:    01.12.99  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Make transformation of the polyhedron                     *
 *                                                                     *
 ***********************************************************************/
{
  if (nvert > 0) {
    for (int i=1; i<=nvert; i++) { pV[i] = t * pV[i]; }

    //  C H E C K   D E T E R M I N A N T   A N D
    //  I N V E R T   F A C E T S   I F   I T   I S   N E G A T I V E

    HepVector3D d = t * HepVector3D(0,0,0);
    HepVector3D x = t * HepVector3D(1,0,0) - d;
    HepVector3D y = t * HepVector3D(0,1,0) - d;
    HepVector3D z = t * HepVector3D(0,0,1) - d;
    if ((x.cross(y))*z < 0) InvertFacets();
  }
  return *this;
}

bool HepPolyhedron::GetNextVertexIndex(int &index, int &edgeFlag) const
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::GetNextVertexIndex          Date:    03.09.96  *
 * Author: Yasuhide Sawada                          Revised:           *
 *                                                                     *
 * Function:                                                           *
 *                                                                     *
 ***********************************************************************/
{
  static int iFace = 1;
  static int iQVertex = 0;
  int vIndex = pF[iFace].edge[iQVertex].v;

  edgeFlag = (vIndex > 0) ? 1 : 0;
  index = abs(vIndex);

  if (iQVertex >= 3 || pF[iFace].edge[iQVertex+1].v == 0) {
    iQVertex = 0;
    if (++iFace > nface) iFace = 1;
    return false;  // Last Edge
  }else{
    ++iQVertex;
    return true;  // not Last Edge
  }
}

HepPoint3D HepPolyhedron::GetVertex(int index) const
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::GetVertex                   Date:    03.09.96  *
 * Author: Yasuhide Sawada                          Revised: 17.11.99  *
 *                                                                     *
 * Function: Get vertex of the index.                                  *
 *                                                                     *
 ***********************************************************************/
{
  if (index <= 0 || index > nvert) {
    HepStd::cerr
      << "HepPolyhedron::GetVertex: irrelevant index " << index
      << HepStd::endl;
    return HepPoint3D();
  }
  return pV[index];
}

bool
HepPolyhedron::GetNextVertex(HepPoint3D &vertex, int &edgeFlag) const
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::GetNextVertex               Date:    22.07.96  *
 * Author: John Allison                             Revised:           *
 *                                                                     *
 * Function: Get vertices of the quadrilaterals in order for each      *
 *           face in face order.  Returns false when finished each     *
 *           face.                                                     *
 *                                                                     *
 ***********************************************************************/
{
  int index;
  bool rep = GetNextVertexIndex(index, edgeFlag);
  vertex = pV[index];
  return rep;
}

bool HepPolyhedron::GetNextVertex(HepPoint3D &vertex, int &edgeFlag,
				       HepNormal3D &normal) const
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::GetNextVertex               Date:    26.11.99  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Get vertices with normals of the quadrilaterals in order  *
 *           for each face in face order.                              *
 *           Returns false when finished each face.                    *
 *                                                                     *
 ***********************************************************************/
{
  static int iFace = 1;
  static int iNode = 0;

  if (nface == 0) return false;  // empty polyhedron

  int k = pF[iFace].edge[iNode].v;
  if (k > 0) { edgeFlag = 1; } else { edgeFlag = -1; k = -k; }
  vertex = pV[k];
  normal = FindNodeNormal(iFace,k);
  if (iNode >= 3 || pF[iFace].edge[iNode+1].v == 0) {
    iNode = 0;
    if (++iFace > nface) iFace = 1;
    return false;                // last node
  }else{
    ++iNode;
    return true;                 // not last node
  }
}

bool HepPolyhedron::GetNextEdgeIndeces(int &i1, int &i2, int &edgeFlag,
					    int &iface1, int &iface2) const
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::GetNextEdgeIndeces          Date:    30.09.96  *
 * Author: E.Chernyaev                              Revised: 17.11.99  *
 *                                                                     *
 * Function: Get indeces of the next edge together with indeces of     *
 *           of the faces which share the edge.                        *
 *           Returns false when the last edge.                         *
 *                                                                     *
 ***********************************************************************/
{
  static int iFace    = 1;
  static int iQVertex = 0;
  static int iOrder   = 1;
  int  k1, k2, kflag, kface1, kface2;

  if (iFace == 1 && iQVertex == 0) {
    k2 = pF[nface].edge[0].v;
    k1 = pF[nface].edge[3].v;
    if (k1 == 0) k1 = pF[nface].edge[2].v;
    if (abs(k1) > abs(k2)) iOrder = -1;
  }

  do {
    k1     = pF[iFace].edge[iQVertex].v;
    kflag  = k1;
    k1     = abs(k1);
    kface1 = iFace; 
    kface2 = pF[iFace].edge[iQVertex].f;
    if (iQVertex >= 3 || pF[iFace].edge[iQVertex+1].v == 0) {
      iQVertex = 0;
      k2 = abs(pF[iFace].edge[iQVertex].v);
      iFace++;
    }else{
      iQVertex++;
      k2 = abs(pF[iFace].edge[iQVertex].v);
    }
  } while (iOrder*k1 > iOrder*k2);

  i1 = k1; i2 = k2; edgeFlag = (kflag > 0) ? 1 : 0;
  iface1 = kface1; iface2 = kface2; 

  if (iFace > nface) {
    iFace  = 1; iOrder = 1;
    return false;
  }else{
    return true;
  }
}

bool
HepPolyhedron::GetNextEdgeIndeces(int &i1, int &i2, int &edgeFlag) const
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::GetNextEdgeIndeces          Date:    17.11.99  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Get indeces of the next edge.                             *
 *           Returns false when the last edge.                         *
 *                                                                     *
 ***********************************************************************/
{
  int kface1, kface2;
  return GetNextEdgeIndeces(i1, i2, edgeFlag, kface1, kface2);
}

bool
HepPolyhedron::GetNextEdge(HepPoint3D &p1,
			   HepPoint3D &p2,
			   int &edgeFlag) const
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::GetNextEdge                 Date:    30.09.96  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Get next edge.                                            *
 *           Returns false when the last edge.                         *
 *                                                                     *
 ***********************************************************************/
{
  int i1,i2;
  bool rep = GetNextEdgeIndeces(i1,i2,edgeFlag);
  p1 = pV[i1];
  p2 = pV[i2];
  return rep;
}

bool
HepPolyhedron::GetNextEdge(HepPoint3D &p1, HepPoint3D &p2,
			  int &edgeFlag, int &iface1, int &iface2) const
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::GetNextEdge                 Date:    17.11.99  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Get next edge with indeces of the faces which share       *
 *           the edge.                                                 *
 *           Returns false when the last edge.                         *
 *                                                                     *
 ***********************************************************************/
{
  int i1,i2;
  bool rep = GetNextEdgeIndeces(i1,i2,edgeFlag,iface1,iface2);
  p1 = pV[i1];
  p2 = pV[i2];
  return rep;
}

void HepPolyhedron::GetFacet(int iFace, int &n, int *iNodes,
			    int *edgeFlags, int *iFaces) const
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::GetFacet                    Date:    15.12.99  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Get face by index                                         *
 *                                                                     *
 ***********************************************************************/
{
  if (iFace < 1 || iFace > nface) {
    HepStd::cerr 
      << "HepPolyhedron::GetFacet: irrelevant index " << iFace
      << HepStd::endl;
    n = 0;
  }else{
    int i, k;
    for (i=0; i<4; i++) { 
      k = pF[iFace].edge[i].v;
      if (k == 0) break;
      if (iFaces != 0) iFaces[i] = pF[iFace].edge[i].f;
      if (k > 0) { 
	iNodes[i] = k;
	if (edgeFlags != 0) edgeFlags[i] = 1;
      }else{
	iNodes[i] = -k;
	if (edgeFlags != 0) edgeFlags[i] = -1;
      }
    }
    n = i;
  }
}

void HepPolyhedron::GetFacet(int index, int &n, HepPoint3D *nodes,
			    int *edgeFlags, HepNormal3D *normals) const
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::GetFacet                    Date:    17.11.99  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Get face by index                                         *
 *                                                                     *
 ***********************************************************************/
{
  int iNodes[4];
  GetFacet(index, n, iNodes, edgeFlags);
  if (n != 0) {
    for (int i=0; i<n; i++) {
      nodes[i] = pV[iNodes[i]];
      if (normals != 0) normals[i] = FindNodeNormal(index,iNodes[i]);
    }
  }
}

bool
HepPolyhedron::GetNextFacet(int &n, HepPoint3D *nodes,
			   int *edgeFlags, HepNormal3D *normals) const
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::GetNextFacet                Date:    19.11.99  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Get next face with normals of unit length at the nodes.   *
 *           Returns false when finished all faces.                    *
 *                                                                     *
 ***********************************************************************/
{
  static int iFace = 1;

  if (edgeFlags == 0) {
    GetFacet(iFace, n, nodes);
  }else if (normals == 0) {
    GetFacet(iFace, n, nodes, edgeFlags);
  }else{
    GetFacet(iFace, n, nodes, edgeFlags, normals);
  }

  if (++iFace > nface) {
    iFace  = 1;
    return false;
  }else{
    return true;
  }
}

HepNormal3D HepPolyhedron::GetNormal(int iFace) const
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::GetNormal                    Date:    19.11.99 *
 * Author: E.Chernyaev                               Revised:          *
 *                                                                     *
 * Function: Get normal of the face given by index                     *
 *                                                                     *
 ***********************************************************************/
{
  if (iFace < 1 || iFace > nface) {
    HepStd::cerr 
      << "HepPolyhedron::GetNormal: irrelevant index " << iFace 
      << HepStd::endl;
    return HepNormal3D();
  }

  int i0  = abs(pF[iFace].edge[0].v);
  int i1  = abs(pF[iFace].edge[1].v);
  int i2  = abs(pF[iFace].edge[2].v);
  int i3  = abs(pF[iFace].edge[3].v);
  if (i3 == 0) i3 = i0;
  return (pV[i2] - pV[i0]).cross(pV[i3] - pV[i1]);
}

HepNormal3D HepPolyhedron::GetUnitNormal(int iFace) const
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::GetNormal                    Date:    19.11.99 *
 * Author: E.Chernyaev                               Revised:          *
 *                                                                     *
 * Function: Get unit normal of the face given by index                *
 *                                                                     *
 ***********************************************************************/
{
  if (iFace < 1 || iFace > nface) {
    HepStd::cerr 
      << "HepPolyhedron::GetUnitNormal: irrelevant index " << iFace
      << HepStd::endl;
    return HepNormal3D();
  }

  int i0  = abs(pF[iFace].edge[0].v);
  int i1  = abs(pF[iFace].edge[1].v);
  int i2  = abs(pF[iFace].edge[2].v);
  int i3  = abs(pF[iFace].edge[3].v);
  if (i3 == 0) i3 = i0;
  return ((pV[i2] - pV[i0]).cross(pV[i3] - pV[i1])).unit();
}

bool HepPolyhedron::GetNextNormal(HepNormal3D &normal) const
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::GetNextNormal               Date:    22.07.96  *
 * Author: John Allison                             Revised: 19.11.99  *
 *                                                                     *
 * Function: Get normals of each face in face order.  Returns false    *
 *           when finished all faces.                                  *
 *                                                                     *
 ***********************************************************************/
{
  static int iFace = 1;
  normal = GetNormal(iFace);
  if (++iFace > nface) {
    iFace = 1;
    return false;
  }else{
    return true;
  }
}

bool HepPolyhedron::GetNextUnitNormal(HepNormal3D &normal) const
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::GetNextUnitNormal           Date:    16.09.96  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Get normals of unit length of each face in face order.    *
 *           Returns false when finished all faces.                    *
 *                                                                     *
 ***********************************************************************/
{
  bool rep = GetNextNormal(normal);
  normal = normal.unit();
  return rep;
}

double HepPolyhedron::GetSurfaceArea() const
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::GetSurfaceArea              Date:    25.05.01  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Returns area of the surface of the polyhedron.            *
 *                                                                     *
 ***********************************************************************/
{
  double s = 0.;
  for (int iFace=1; iFace<=nface; iFace++) {
    int i0 = abs(pF[iFace].edge[0].v);
    int i1 = abs(pF[iFace].edge[1].v);
    int i2 = abs(pF[iFace].edge[2].v);
    int i3 = abs(pF[iFace].edge[3].v);
    if (i3 == 0) i3 = i0;
    s += ((pV[i2] - pV[i0]).cross(pV[i3] - pV[i1])).mag();
  }
  return s/2.;
}

double HepPolyhedron::GetVolume() const
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::GetVolume                   Date:    25.05.01  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Returns volume of the polyhedron.                         *
 *                                                                     *
 ***********************************************************************/
{
  double v = 0.;
  for (int iFace=1; iFace<=nface; iFace++) {
    int i0 = abs(pF[iFace].edge[0].v);
    int i1 = abs(pF[iFace].edge[1].v);
    int i2 = abs(pF[iFace].edge[2].v);
    int i3 = abs(pF[iFace].edge[3].v);
    HepPoint3D g;
    if (i3 == 0) {
      i3 = i0;
      g  = (pV[i0]+pV[i1]+pV[i2]) * (1./3.);
    }else{
      g  = (pV[i0]+pV[i1]+pV[i2]+pV[i3]) * 0.25;
    }
    v += ((pV[i2] - pV[i0]).cross(pV[i3] - pV[i1])).dot(g);
  }
  return v/6.;
}

int
HepPolyhedron::createTwistedTrap(double Dz,
				 const double xy1[][2],
				 const double xy2[][2])
/***********************************************************************
 *                                                                     *
 * Name: createTwistedTrap                           Date:    05.11.02 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: Creates polyhedron for twisted trapezoid                  *
 *                                                                     *
 * Input: Dz       - half-length along Z             8----7            *
 *        xy1[2,4] - quadrilateral at Z=-Dz       5----6  !            *
 *        xy2[2,4] - quadrilateral at Z=+Dz       !  4-!--3            *
 *                                                1----2               *
 *                                                                     *
 ***********************************************************************/
{
  AllocateMemory(12,18);

  pV[ 1] = HepPoint3D(xy1[0][0],xy1[0][1],-Dz);
  pV[ 2] = HepPoint3D(xy1[1][0],xy1[1][1],-Dz);
  pV[ 3] = HepPoint3D(xy1[2][0],xy1[2][1],-Dz);
  pV[ 4] = HepPoint3D(xy1[3][0],xy1[3][1],-Dz);

  pV[ 5] = HepPoint3D(xy2[0][0],xy2[0][1], Dz);
  pV[ 6] = HepPoint3D(xy2[1][0],xy2[1][1], Dz);
  pV[ 7] = HepPoint3D(xy2[2][0],xy2[2][1], Dz);
  pV[ 8] = HepPoint3D(xy2[3][0],xy2[3][1], Dz);

  pV[ 9] = (pV[1]+pV[2]+pV[5]+pV[6])/4.;
  pV[10] = (pV[2]+pV[3]+pV[6]+pV[7])/4.;
  pV[11] = (pV[3]+pV[4]+pV[7]+pV[8])/4.;
  pV[12] = (pV[4]+pV[1]+pV[8]+pV[5])/4.;

  enum {DUMMY, BOTTOM,
	LEFT_BOTTOM,  LEFT_FRONT,   LEFT_TOP,  LEFT_BACK,
	BACK_BOTTOM,  BACK_LEFT,    BACK_TOP,  BACK_RIGHT,
	RIGHT_BOTTOM, RIGHT_BACK,   RIGHT_TOP, RIGHT_FRONT,
        FRONT_BOTTOM, FRONT_RIGHT,  FRONT_TOP, FRONT_LEFT,
        TOP};

  pF[ 1]=G4Facet(1,LEFT_BOTTOM, 4,BACK_BOTTOM, 3,RIGHT_BOTTOM, 2,FRONT_BOTTOM);

  pF[ 2]=G4Facet(4,BOTTOM,     -1,LEFT_FRONT,  -12,LEFT_BACK,    0,0);
  pF[ 3]=G4Facet(1,FRONT_LEFT, -5,LEFT_TOP,    -12,LEFT_BOTTOM,  0,0);
  pF[ 4]=G4Facet(5,TOP,        -8,LEFT_BACK,   -12,LEFT_FRONT,   0,0);
  pF[ 5]=G4Facet(8,BACK_LEFT,  -4,LEFT_BOTTOM, -12,LEFT_TOP,     0,0);

  pF[ 6]=G4Facet(3,BOTTOM,     -4,BACK_LEFT,   -11,BACK_RIGHT,   0,0);
  pF[ 7]=G4Facet(4,LEFT_BACK,  -8,BACK_TOP,    -11,BACK_BOTTOM,  0,0);
  pF[ 8]=G4Facet(8,TOP,        -7,BACK_RIGHT,  -11,BACK_LEFT,    0,0);
  pF[ 9]=G4Facet(7,RIGHT_BACK, -3,BACK_BOTTOM, -11,BACK_TOP,     0,0);

  pF[10]=G4Facet(2,BOTTOM,     -3,RIGHT_BACK,  -10,RIGHT_FRONT,  0,0);
  pF[11]=G4Facet(3,BACK_RIGHT, -7,RIGHT_TOP,   -10,RIGHT_BOTTOM, 0,0);
  pF[12]=G4Facet(7,TOP,        -6,RIGHT_FRONT, -10,RIGHT_BACK,   0,0);
  pF[13]=G4Facet(6,FRONT_RIGHT,-2,RIGHT_BOTTOM,-10,RIGHT_TOP,    0,0);

  pF[14]=G4Facet(1,BOTTOM,     -2,FRONT_RIGHT,  -9,FRONT_LEFT,   0,0);
  pF[15]=G4Facet(2,RIGHT_FRONT,-6,FRONT_TOP,    -9,FRONT_BOTTOM, 0,0);
  pF[16]=G4Facet(6,TOP,        -5,FRONT_LEFT,   -9,FRONT_RIGHT,  0,0);
  pF[17]=G4Facet(5,LEFT_FRONT, -1,FRONT_BOTTOM, -9,FRONT_TOP,    0,0);
 
  pF[18]=G4Facet(5,FRONT_TOP, 6,RIGHT_TOP, 7,BACK_TOP, 8,LEFT_TOP);

  return 0;
}

int
HepPolyhedron::createPolyhedron(int Nnodes, int Nfaces,
				const double xyz[][3],
				const int  faces[][4])
/***********************************************************************
 *                                                                     *
 * Name: createPolyhedron                            Date:    05.11.02 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: Creates user defined polyhedron                           *
 *                                                                     *
 * Input: Nnodes  - number of nodes                                    *
 *        Nfaces  - number of faces                                    *
 *        nodes[][3] - node coordinates                                *
 *        faces[][4] - faces                                           *
 *                                                                     *
 ***********************************************************************/
{
  AllocateMemory(Nnodes, Nfaces);
  if (nvert == 0) return 1;

  for (int i=0; i<Nnodes; i++) {
    pV[i+1] = HepPoint3D(xyz[i][0], xyz[i][1], xyz[i][2]);
  }
  for (int k=0; k<Nfaces; k++) {
    pF[k+1] = G4Facet(faces[k][0],0,faces[k][1],0,faces[k][2],0,faces[k][3],0);
  }
  SetReferences();
  return 0;
}

HepPolyhedronTrd2::HepPolyhedronTrd2(double Dx1, double Dx2,
				     double Dy1, double Dy2,
				     double Dz)
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedronTrd2                           Date:    22.07.96 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: Create GEANT4 TRD2-trapezoid                              *
 *                                                                     *
 * Input: Dx1 - half-length along X at -Dz           8----7            *
 *        Dx2 - half-length along X ay +Dz        5----6  !            *
 *        Dy1 - half-length along Y ay -Dz        !  4-!--3            *
 *        Dy2 - half-length along Y ay +Dz        1----2               *
 *        Dz  - half-length along Z                                    *
 *                                                                     *
 ***********************************************************************/
{
  AllocateMemory(8,6);

  pV[1] = HepPoint3D(-Dx1,-Dy1,-Dz);
  pV[2] = HepPoint3D( Dx1,-Dy1,-Dz);
  pV[3] = HepPoint3D( Dx1, Dy1,-Dz);
  pV[4] = HepPoint3D(-Dx1, Dy1,-Dz);
  pV[5] = HepPoint3D(-Dx2,-Dy2, Dz);
  pV[6] = HepPoint3D( Dx2,-Dy2, Dz);
  pV[7] = HepPoint3D( Dx2, Dy2, Dz);
  pV[8] = HepPoint3D(-Dx2, Dy2, Dz);

  CreatePrism();
}

HepPolyhedronTrd2::~HepPolyhedronTrd2() {}

HepPolyhedronTrd1::HepPolyhedronTrd1(double Dx1, double Dx2,
				     double Dy, double Dz)
  : HepPolyhedronTrd2(Dx1, Dx2, Dy, Dy, Dz) {}

HepPolyhedronTrd1::~HepPolyhedronTrd1() {}

HepPolyhedronBox::HepPolyhedronBox(double Dx, double Dy, double Dz)
  : HepPolyhedronTrd2(Dx, Dx, Dy, Dy, Dz) {}

HepPolyhedronBox::~HepPolyhedronBox() {}

HepPolyhedronTrap::HepPolyhedronTrap(double Dz,
				     double Theta,
				     double Phi,
				     double Dy1,
				     double Dx1,
				     double Dx2,
				     double Alp1,
				     double Dy2,
				     double Dx3,
				     double Dx4,
				     double Alp2)
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedronTrap                           Date:    20.11.96 *
 * Author: E.Chernyaev                               Revised:          *
 *                                                                     *
 * Function: Create GEANT4 TRAP-trapezoid                              *
 *                                                                     *
 * Input: DZ   - half-length in Z                                      *
 *        Theta,Phi - polar angles of the line joining centres of the  *
 *                    faces at Z=-Dz and Z=+Dz                         *
 *        Dy1  - half-length in Y of the face at Z=-Dz                 *
 *        Dx1  - half-length in X of low edge of the face at Z=-Dz     *
 *        Dx2  - half-length in X of top edge of the face at Z=-Dz     *
 *        Alp1 - angle between Y-axis and the median joining top and   *
 *               low edges of the face at Z=-Dz                        *
 *        Dy2  - half-length in Y of the face at Z=+Dz                 *
 *        Dx3  - half-length in X of low edge of the face at Z=+Dz     *
 *        Dx4  - half-length in X of top edge of the face at Z=+Dz     *
 *        Alp2 - angle between Y-axis and the median joining top and   *
 *               low edges of the face at Z=+Dz                        *
 *                                                                     *
 ***********************************************************************/
{
  double DzTthetaCphi = Dz*tan(Theta)*cos(Phi);
  double DzTthetaSphi = Dz*tan(Theta)*sin(Phi);
  double Dy1Talp1 = Dy1*tan(Alp1);
  double Dy2Talp2 = Dy2*tan(Alp2);
  
  AllocateMemory(8,6);

  pV[1] = HepPoint3D(-DzTthetaCphi-Dy1Talp1-Dx1,-DzTthetaSphi-Dy1,-Dz);
  pV[2] = HepPoint3D(-DzTthetaCphi-Dy1Talp1+Dx1,-DzTthetaSphi-Dy1,-Dz);
  pV[3] = HepPoint3D(-DzTthetaCphi+Dy1Talp1+Dx2,-DzTthetaSphi+Dy1,-Dz);
  pV[4] = HepPoint3D(-DzTthetaCphi+Dy1Talp1-Dx2,-DzTthetaSphi+Dy1,-Dz);
  pV[5] = HepPoint3D( DzTthetaCphi-Dy2Talp2-Dx3, DzTthetaSphi-Dy2, Dz);
  pV[6] = HepPoint3D( DzTthetaCphi-Dy2Talp2+Dx3, DzTthetaSphi-Dy2, Dz);
  pV[7] = HepPoint3D( DzTthetaCphi+Dy2Talp2+Dx4, DzTthetaSphi+Dy2, Dz);
  pV[8] = HepPoint3D( DzTthetaCphi+Dy2Talp2-Dx4, DzTthetaSphi+Dy2, Dz);

  CreatePrism();
}

HepPolyhedronTrap::~HepPolyhedronTrap() {}

HepPolyhedronPara::HepPolyhedronPara(double Dx, double Dy, double Dz,
				     double Alpha, double Theta,
				     double Phi)
  : HepPolyhedronTrap(Dz, Theta, Phi, Dy, Dx, Dx, Alpha, Dy, Dx, Dx, Alpha) {}

HepPolyhedronPara::~HepPolyhedronPara() {}

HepPolyhedronCons::HepPolyhedronCons(double Rmn1,
				     double Rmx1,
				     double Rmn2,
				     double Rmx2, 
				     double Dz,
				     double Phi1,
				     double Dphi) 
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedronCons::HepPolyhedronCons        Date:    15.12.96 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised: 15.12.96 *
 *                                                                     *
 * Function: Constructor for CONS, TUBS, CONE, TUBE                    *
 *                                                                     *
 * Input: Rmn1, Rmx1 - inside and outside radiuses at -Dz              *
 *        Rmn2, Rmx2 - inside and outside radiuses at +Dz              *
 *        Dz         - half length in Z                                *
 *        Phi1       - starting angle of the segment                   *
 *        Dphi       - segment range                                   *
 *                                                                     *
 ***********************************************************************/
{
  static double wholeCircle=2*M_PI;

  //   C H E C K   I N P U T   P A R A M E T E R S

  int k = 0;
  if (Rmn1 < 0. || Rmx1 < 0. || Rmn2 < 0. || Rmx2 < 0.)        k = 1;
  if (Rmn1 > Rmx1 || Rmn2 > Rmx2)                              k = 1;
  if (Rmn1 == Rmx1 && Rmn2 == Rmx2)                            k = 1;

  if (Dz <= 0.) k += 2;
 
  double phi1, phi2, dphi;
  if (Dphi < 0.) {
    phi2 = Phi1; phi1 = phi2 - Dphi;
  }else if (Dphi == 0.) {
    phi1 = Phi1; phi2 = phi1 + wholeCircle;
  }else{
    phi1 = Phi1; phi2 = phi1 + Dphi;
  }
  dphi  = phi2 - phi1;
  if (abs(dphi-wholeCircle) < perMillion) dphi = wholeCircle;
  if (dphi > wholeCircle) k += 4; 

  if (k != 0) {
    HepStd::cerr << "HepPolyhedronCone(s)/Tube(s): error in input parameters";
    if ((k & 1) != 0) HepStd::cerr << " (radiuses)";
    if ((k & 2) != 0) HepStd::cerr << " (half-length)";
    if ((k & 4) != 0) HepStd::cerr << " (angles)";
    HepStd::cerr << HepStd::endl;
    HepStd::cerr << " Rmn1=" << Rmn1 << " Rmx1=" << Rmx1;
    HepStd::cerr << " Rmn2=" << Rmn2 << " Rmx2=" << Rmx2;
    HepStd::cerr << " Dz=" << Dz << " Phi1=" << Phi1 << " Dphi=" << Dphi
	      << HepStd::endl;
    return;
  }
  
  //   P R E P A R E   T W O   P O L Y L I N E S

  double zz[4], rr[4];
  zz[0] =  Dz; 
  zz[1] = -Dz; 
  zz[2] =  Dz; 
  zz[3] = -Dz; 
  rr[0] =  Rmx2;
  rr[1] =  Rmx1;
  rr[2] =  Rmn2;
  rr[3] =  Rmn1;

  //   R O T A T E    P O L Y L I N E S

  RotateAroundZ(0, phi1, dphi, 2, 2, zz, rr, -1, -1); 
  SetReferences();
}

HepPolyhedronCons::~HepPolyhedronCons() {}

HepPolyhedronCone::HepPolyhedronCone(double Rmn1, double Rmx1, 
				     double Rmn2, double Rmx2,
				     double Dz) :
  HepPolyhedronCons(Rmn1, Rmx1, Rmn2, Rmx2, Dz, 0*deg, 360*deg) {}

HepPolyhedronCone::~HepPolyhedronCone() {}

HepPolyhedronTubs::HepPolyhedronTubs(double Rmin, double Rmax,
				     double Dz, 
				     double Phi1, double Dphi)
  :   HepPolyhedronCons(Rmin, Rmax, Rmin, Rmax, Dz, Phi1, Dphi) {}

HepPolyhedronTubs::~HepPolyhedronTubs() {}

HepPolyhedronTube::HepPolyhedronTube (double Rmin, double Rmax,
				      double Dz)
  : HepPolyhedronCons(Rmin, Rmax, Rmin, Rmax, Dz, 0*deg, 360*deg) {}

HepPolyhedronTube::~HepPolyhedronTube () {}

HepPolyhedronPgon::HepPolyhedronPgon(double phi,
				     double dphi,
				     int    npdv,
				     int    nz,
				     const double *z,
				     const double *rmin,
				     const double *rmax)
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedronPgon                           Date:    09.12.96 *
 * Author: E.Chernyaev                               Revised:          *
 *                                                                     *
 * Function: Constructor of polyhedron for PGON, PCON                  *
 *                                                                     *
 * Input: phi  - initial phi                                           *
 *        dphi - delta phi                                             *
 *        npdv - number of steps along phi                             *
 *        nz   - number of z-planes (at least two)                     *
 *        z[]  - z coordinates of the slices                           *
 *        rmin[] - smaller r at the slices                             *
 *        rmax[] - bigger  r at the slices                             *
 *                                                                     *
 ***********************************************************************/
{
  //   C H E C K   I N P U T   P A R A M E T E R S

  if (dphi <= 0. || dphi > 2*M_PI) {
    HepStd::cerr
      << "HepPolyhedronPgon/Pcon: wrong delta phi = " << dphi
      << HepStd::endl;
    return;
  }    
    
  if (nz < 2) {
    HepStd::cerr
      << "HepPolyhedronPgon/Pcon: number of z-planes less than two = " << nz
      << HepStd::endl;
    return;
  }

  if (npdv < 0) {
    HepStd::cerr
      << "HepPolyhedronPgon/Pcon: error in number of phi-steps =" << npdv
      << HepStd::endl;
    return;
  }

  int i;
  for (i=0; i<nz; i++) {
    if (rmin[i] < 0. || rmax[i] < 0. || rmin[i] > rmax[i]) {
      HepStd::cerr
	<< "HepPolyhedronPgon: error in radiuses rmin[" << i << "]="
	<< rmin[i] << " rmax[" << i << "]=" << rmax[i]
	<< HepStd::endl;
      return;
    }
  }

  //   P R E P A R E   T W O   P O L Y L I N E S

  double *zz, *rr;
  zz = new double[2*nz];
  rr = new double[2*nz];

  if (z[0] > z[nz-1]) {
    for (i=0; i<nz; i++) {
      zz[i]    = z[i];
      rr[i]    = rmax[i];
      zz[i+nz] = z[i];
      rr[i+nz] = rmin[i];
    }
  }else{
    for (i=0; i<nz; i++) {
      zz[i]    = z[nz-i-1];
      rr[i]    = rmax[nz-i-1];
      zz[i+nz] = z[nz-i-1];
      rr[i+nz] = rmin[nz-i-1];
    }
  }

  //   R O T A T E    P O L Y L I N E S

  RotateAroundZ(npdv, phi, dphi, nz, nz, zz, rr, -1, (npdv == 0) ? -1 : 1); 
  SetReferences();
  
  delete [] zz;
  delete [] rr;
}

HepPolyhedronPgon::~HepPolyhedronPgon() {}

HepPolyhedronPcon::HepPolyhedronPcon(double phi, double dphi, int nz,
				     const double *z,
				     const double *rmin,
				     const double *rmax)
  : HepPolyhedronPgon(phi, dphi, 0, nz, z, rmin, rmax) {}

HepPolyhedronPcon::~HepPolyhedronPcon() {}

HepPolyhedronSphere::HepPolyhedronSphere(double rmin, double rmax,
				       double phi, double dphi,
				       double the, double dthe)
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedronSphere                         Date:    11.12.96 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: Constructor of polyhedron for SPHERE                      *
 *                                                                     *
 * Input: rmin - internal radius                                       *
 *        rmax - external radius                                       *
 *        phi  - initial phi                                           *
 *        dphi - delta phi                                             *
 *        the  - initial theta                                         *
 *        dthe - delta theta                                           *
 *                                                                     *
 ***********************************************************************/
{
  //   C H E C K   I N P U T   P A R A M E T E R S

  if (dphi <= 0. || dphi > 2*M_PI) {
    HepStd::cerr
      << "HepPolyhedronSphere: wrong delta phi = " << dphi
      << HepStd::endl;
    return;
  }    

  if (the < 0. || the > M_PI) {
    HepStd::cerr
      << "HepPolyhedronSphere: wrong theta = " << the
      << HepStd::endl;
    return;
  }    
  
  if (dthe <= 0. || dthe > M_PI) {
    HepStd::cerr
      << "HepPolyhedronSphere: wrong delta theta = " << dthe
      << HepStd::endl;
    return;
  }    

  if (the+dthe > M_PI) {
    HepStd::cerr
      << "HepPolyhedronSphere: wrong theta + delta theta = "
      << the << " " << dthe
      << HepStd::endl;
    return;
  }    
  
  if (rmin < 0. || rmin >= rmax) {
    HepStd::cerr
      << "HepPolyhedronSphere: error in radiuses"
      << " rmin=" << rmin << " rmax=" << rmax
      << HepStd::endl;
    return;
  }

  //   P R E P A R E   T W O   P O L Y L I N E S

  int ns = (GetNumberOfRotationSteps() + 1) / 2;
  int np1 = int(dthe*ns/M_PI+.5) + 1;
  if (np1 <= 1) np1 = 2;
  int np2 = rmin < perMillion ? 1 : np1;

  double *zz, *rr;
  zz = new double[np1+np2];
  rr = new double[np1+np2];

  double a = dthe/(np1-1);
  double cosa, sina;
  for (int i=0; i<np1; i++) {
    cosa  = cos(the+i*a);
    sina  = sin(the+i*a);
    zz[i] = rmax*cosa;
    rr[i] = rmax*sina;
    if (np2 > 1) {
      zz[i+np1] = rmin*cosa;
      rr[i+np1] = rmin*sina;
    }
  }
  if (np2 == 1) {
    zz[np1] = 0.;
    rr[np1] = 0.;
  }

  //   R O T A T E    P O L Y L I N E S

  RotateAroundZ(0, phi, dphi, np1, np2, zz, rr, -1, -1); 
  SetReferences();
  
  delete [] zz;
  delete [] rr;
}

HepPolyhedronSphere::~HepPolyhedronSphere() {}

HepPolyhedronTorus::HepPolyhedronTorus(double rmin,
				       double rmax,
				       double rtor,
				       double phi,
				       double dphi)
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedronTorus                          Date:    11.12.96 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: Constructor of polyhedron for TORUS                       *
 *                                                                     *
 * Input: rmin - internal radius                                       *
 *        rmax - external radius                                       *
 *        rtor - radius of torus                                       *
 *        phi  - initial phi                                           *
 *        dphi - delta phi                                             *
 *                                                                     *
 ***********************************************************************/
{
  //   C H E C K   I N P U T   P A R A M E T E R S

  if (dphi <= 0. || dphi > 2*M_PI) {
    HepStd::cerr
      << "HepPolyhedronTorus: wrong delta phi = " << dphi
      << HepStd::endl;
    return;
  }

  if (rmin < 0. || rmin >= rmax || rmax >= rtor) {
    HepStd::cerr
      << "HepPolyhedronTorus: error in radiuses"
      << " rmin=" << rmin << " rmax=" << rmax << " rtorus=" << rtor
      << HepStd::endl;
    return;
  }

  //   P R E P A R E   T W O   P O L Y L I N E S

  int np1 = GetNumberOfRotationSteps();
  int np2 = rmin < perMillion ? 1 : np1;

  double *zz, *rr;
  zz = new double[np1+np2];
  rr = new double[np1+np2];

  double a = 2*M_PI/np1;
  double cosa, sina;
  for (int i=0; i<np1; i++) {
    cosa  = cos(i*a);
    sina  = sin(i*a);
    zz[i] = rmax*cosa;
    rr[i] = rtor+rmax*sina;
    if (np2 > 1) {
      zz[i+np1] = rmin*cosa;
      rr[i+np1] = rtor+rmin*sina;
    }
  }
  if (np2 == 1) {
    zz[np1] = 0.;
    rr[np1] = rtor;
    np2 = -1;
  }

  //   R O T A T E    P O L Y L I N E S

  RotateAroundZ(0, phi, dphi, -np1, -np2, zz, rr, -1,-1); 
  SetReferences();
  
  delete [] zz;
  delete [] rr;
}

HepPolyhedronTorus::~HepPolyhedronTorus() {}

int HepPolyhedron::fNumberOfRotationSteps = DEFAULT_NUMBER_OF_STEPS;
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::fNumberOfRotationSteps       Date:    24.06.97 *
 * Author: J.Allison (Manchester University)         Revised:          *
 *                                                                     *
 * Function: Number of steps for whole circle                          *
 *                                                                     *
 ***********************************************************************/

#include "BooleanProcessor.src"
static BooleanProcessor processor;

HepPolyhedron HepPolyhedron::add(const HepPolyhedron & p) const 
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::add                          Date:    19.03.00 *
 * Author: E.Chernyaev                               Revised:          *
 *                                                                     *
 * Function: Boolean "union" of two polyhedra                          *
 *                                                                     *
 ***********************************************************************/
{
  return processor.execute(OP_UNION, *this, p);
}

HepPolyhedron HepPolyhedron::intersect(const HepPolyhedron & p) const 
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::intersect                    Date:    19.03.00 *
 * Author: E.Chernyaev                               Revised:          *
 *                                                                     *
 * Function: Boolean "intersection" of two polyhedra                   *
 *                                                                     *
 ***********************************************************************/
{
  return processor.execute(OP_INTERSECTION, *this, p);
}

HepPolyhedron HepPolyhedron::subtract(const HepPolyhedron & p) const 
/***********************************************************************
 *                                                                     *
 * Name: HepPolyhedron::add                          Date:    19.03.00 *
 * Author: E.Chernyaev                               Revised:          *
 *                                                                     *
 * Function: Boolean "subtraction" of "p" from "this"                  *
 *                                                                     *
 ***********************************************************************/
{
  return processor.execute(OP_SUBTRACTION, *this, p);
}
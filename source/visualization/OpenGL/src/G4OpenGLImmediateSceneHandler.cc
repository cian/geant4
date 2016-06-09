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
// $Id: G4OpenGLImmediateSceneHandler.cc,v 1.9 2003/06/16 17:13:39 gunter Exp $
// GEANT4 tag $Name: geant4-05-02 $
//
// 
// Andrew Walkden  10th February 1997
// OpenGL immediate scene - draws immediately to buffer
//                           (saving space on server).

#ifdef G4VIS_BUILD_OPENGL_DRIVER

// Included here - problems with HP compiler if not before other includes?
#include "G4NURBS.hh"

// Here follows a special for Mesa, the OpenGL emulator.  Does not affect
// other OpenGL's, as far as I'm aware.   John Allison 18/9/96.
#define CENTERLINE_CLPP  /* CenterLine C++ workaround: */
// Also seems to be required for HP's CC and AIX xlC, at least.

#include "G4OpenGLSceneHandler.hh"
#include "G4OpenGLViewer.hh"
#include "G4OpenGLTransform3D.hh"
#include "G4Point3D.hh"
#include "G4Normal3D.hh"
#include "G4Transform3D.hh"
#include "G4Polyline.hh"
#include "G4Text.hh"
#include "G4Circle.hh"
#include "G4Square.hh"
#include "G4Polyhedron.hh"
#include "G4VisAttributes.hh"

#include "G4OpenGLImmediateSceneHandler.hh"

G4OpenGLImmediateSceneHandler::G4OpenGLImmediateSceneHandler (G4VGraphicsSystem& system,
						const G4String& name):
G4OpenGLSceneHandler (system, fSceneIdCount++, name)
{
  fSceneCount++;
}

G4OpenGLImmediateSceneHandler::~G4OpenGLImmediateSceneHandler ()
{
  fSceneCount--;
}

#include <iomanip>

void G4OpenGLImmediateSceneHandler::BeginPrimitives
(const G4Transform3D& objectTransformation) {
  G4VSceneHandler::BeginPrimitives (objectTransformation);
  glPushMatrix();
  G4OpenGLTransform3D oglt (objectTransformation);

  /*************************** Check matrix.
  const GLdouble* m = oglt.GetGLMatrix ();
  G4cout << "G4OpenGLTransform3D matrix:";
  for (int i = 0; i < 16; i++) {
    if ((i % 4) == 0) G4cout << '\n';
    G4cout << std::setw (15) << m[i];
  }
  G4cout << G4endl;
  *****************************************/

  glMultMatrixd (oglt.GetGLMatrix ());
}

void G4OpenGLImmediateSceneHandler::EndPrimitives () {
  glPopMatrix();
  if (fReadyForTransients) {
    glFlush ();
  }
  G4VSceneHandler::EndPrimitives ();
}

void G4OpenGLImmediateSceneHandler::BeginModeling () {
  G4VSceneHandler::BeginModeling();
  if (fpViewer -> GetViewParameters ().GetDrawingStyle() == G4ViewParameters::hlr) {
    initialize_hlr = true;
  }
}

void G4OpenGLImmediateSceneHandler::EndModeling () {
  if (fpViewer -> GetViewParameters ().GetDrawingStyle() == G4ViewParameters::hlr) {
    initialize_hlr = true;
    //    glDisable (GL_POLYGON_OFFSET_FILL);
  }
  G4VSceneHandler::EndModeling ();
}

void G4OpenGLImmediateSceneHandler::ClearTransientStore () {

  G4VSceneHandler::ClearTransientStore ();

  // For immediate mode, clear screen and re-draw detector.
  if (fpViewer) {
    fpViewer -> SetView ();
    fpViewer -> ClearView ();
    fpViewer -> DrawView ();
  }
}

G4int G4OpenGLImmediateSceneHandler::GetSceneCount () {
  return fSceneCount;
}

G4int G4OpenGLImmediateSceneHandler::fSceneIdCount = 0;

G4int G4OpenGLImmediateSceneHandler::fSceneCount = 0;

#endif
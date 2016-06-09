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
// $Id: G4Scene.cc,v 1.13 2003/06/16 17:14:15 gunter Exp $
// GEANT4 tag $Name: geant4-05-02 $
//
// 
// Scene data  John Allison  19th July 1996.

#include "G4Scene.hh"

#include "G4Vector3D.hh"
#include "G4BoundingSphereScene.hh"
#include "G4VisAttributes.hh"
#include "G4PhysicalVolumeModel.hh"
#include "G4TransportationManager.hh"

G4Scene::G4Scene (const G4String& name):
  fName (name),
  fRefreshAtEndOfEvent(true)
{} // Note all other data members have default initial values.

G4Scene::~G4Scene () {}

G4bool G4Scene::AddRunDurationModel (G4VModel* pModel, G4bool warn) {
  G4int i, nModels = fRunDurationModelList.size ();
  for (i = 0; i < nModels; i++) {
    if (pModel -> GetGlobalDescription () ==
	fRunDurationModelList [i] -> GetGlobalDescription ()) break;
  }
  if (i < nModels) {
    if (warn) {
      G4cout << "G4Scene::AddRunDurationModel: model \""
	     << pModel -> GetGlobalDescription ()
	     << "\"\n  is already in the run-duration list of scene \""
	     << fName
	     << "\"."
	     << G4endl;
    }
    return false;
  }
  fRunDurationModelList.push_back (pModel);
  CalculateExtent ();
  return true;
}

void G4Scene::CalculateExtent () {
  G4int nModels = fRunDurationModelList.size ();
  G4BoundingSphereScene boundingSphereScene;
  for (G4int i = 0; i < nModels; i++) {
    const G4VisExtent& thisExtent =
      fRunDurationModelList[i] -> GetExtent ();
    G4Point3D thisCentre = thisExtent.GetExtentCentre ();
    G4double thisRadius = thisExtent.GetExtentRadius ();
    thisCentre.transform (fRunDurationModelList[i] -> GetTransformation ());
    boundingSphereScene.AccrueBoundingSphere (thisCentre, thisRadius);
  }
  fExtent = boundingSphereScene.GetBoundingSphereExtent ();
  fStandardTargetPoint = fExtent.GetExtentCentre ();
}

G4bool G4Scene::AddWorldIfEmpty (G4bool warn) {
  G4bool successful = true;
  if (IsEmpty ()) {
    successful = false;
    G4VPhysicalVolume* pWorld =
      G4TransportationManager::GetTransportationManager ()
      -> GetNavigatorForTracking () -> GetWorldVolume ();
    if (pWorld) {
      const G4VisAttributes* pVisAttribs =
	pWorld -> GetLogicalVolume () -> GetVisAttributes ();
      if (!pVisAttribs || pVisAttribs -> IsVisible ()) {
	if (warn) {
	  G4cout << 
	    "Your \"world\" has no vis attributes or is marked as visible."
	    "\n  For a better view of the contents, mark the world as"
	    " invisible, e.g.,"
	    "\n  myWorldLogicalVol ->"
	    " SetVisAttributes (G4VisAttributes::Invisible);"
		 << G4endl;
	}
      }
      successful = AddRunDurationModel (new G4PhysicalVolumeModel (pWorld));
      // Note: default depth and no modeling parameters.
      if (successful) {
	if (warn) {
	  G4cout <<
	    "G4Scene::AddWorldIfEmpty: The scene was empty,"
	    "\n   \"world\" has been added.";
	  G4cout << G4endl;
	}
      }
    }
  }
  return successful;
}

G4bool G4Scene::AddEndOfEventModel (G4VModel* pModel, G4bool warn) {
  G4int i, nModels = fEndOfEventModelList.size ();
  for (i = 0; i < nModels; i++) {
    if (pModel -> GetGlobalDescription () ==
	fEndOfEventModelList [i] -> GetGlobalDescription ()) break;
  }
  if (i < nModels) {
    delete fEndOfEventModelList[i];
    fEndOfEventModelList[i] = pModel;
    if (warn) {
      G4cout << "G4Scene::AddEndOfEventModel: a model \""
	     << pModel -> GetGlobalDescription ()
	     << "\"\n  is already in the run-duration list of scene \""
	     << fName <<
	"\".\n  The old model has been deleted; this new model replaces it."
	     << G4endl;
    }
    return false;
  }
  fEndOfEventModelList.push_back (pModel);
  return true;
}

void G4Scene::Clear () {
  size_t i;
  for (i = 0; i < fRunDurationModelList.size(); ++i) {
    delete fRunDurationModelList[i];
  }
  for (i = 0; i < fEndOfEventModelList.size(); ++i) {
    delete fEndOfEventModelList[i];
  }
}

std::ostream& operator << (std::ostream& os, const G4Scene& s) {

  size_t i;

  os << "Scene data:";

  os << "\n  Run-duration model list:";
  for (i = 0; i < s.fRunDurationModelList.size (); i++) {
    os << "\n  " << *(s.fRunDurationModelList[i]);
  }

  os << "\n  End-of-event model list:";
  for (i = 0; i < s.fEndOfEventModelList.size (); i++) {
    os << "\n  " << *(s.fEndOfEventModelList[i]);
  }

  os << "\n  Extent or bounding box: " << s.fExtent;

  os << "\n  Standard target point:  " << s.fStandardTargetPoint;

  os << "\n  End of event action set to \"";
  if (s.fRefreshAtEndOfEvent) os << "refresh";
  else os << "accumulate";
  os << "\"";

  return os;
}

G4bool G4Scene::operator != (const G4Scene& s) const {
  if (
      (fRunDurationModelList.size () !=
       s.fRunDurationModelList.size ())                 ||
      (fExtent               != s.fExtent)              ||
      !(fStandardTargetPoint == s.fStandardTargetPoint) ||
      fRefreshAtEndOfEvent != s.fRefreshAtEndOfEvent
      ) return true;

  for (size_t i = 0; i < fRunDurationModelList.size (); i++) {
    if (fRunDurationModelList[i] != s.fRunDurationModelList[i])
      return true;
  }

  return false;
}
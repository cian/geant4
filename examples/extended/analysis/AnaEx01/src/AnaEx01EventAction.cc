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
// $Id: AnaEx01EventAction.cc,v 1.5 2001/11/16 14:31:12 barrand Exp $
// GEANT4 tag $Name: geant4-05-02 $
//
// 

#ifdef G4ANALYSIS_USE
#include "AnaEx01AnalysisManager.hh"
#endif

#include "AnaEx01EventAction.hh"

AnaEx01EventAction::AnaEx01EventAction(
 AnaEx01AnalysisManager* aAnalysisManager
):fAnalysisManager(aAnalysisManager){}

AnaEx01EventAction::~AnaEx01EventAction(){}

void AnaEx01EventAction::BeginOfEventAction(const G4Event* aEvent){
#ifdef G4ANALYSIS_USE
  if(fAnalysisManager) fAnalysisManager->BeginOfEvent(aEvent);
#endif
}

void AnaEx01EventAction::EndOfEventAction(const G4Event* aEvent) {
#ifdef G4ANALYSIS_USE
  if(fAnalysisManager) fAnalysisManager->EndOfEvent(aEvent);
#endif
}

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
// $Id: Em8DetectorConstruction.cc,v 1.9 2003/04/17 13:05:19 grichine Exp $
// GEANT4 tag $Name: geant4-05-02 $
//
// 

#include "Em8DetectorConstruction.hh"
#include "Em8DetectorMessenger.hh"

#include "Em8CalorimeterSD.hh"
#include "G4VXrayTRmodel.hh"
#include "G4IrregularXrayTRmodel.hh"
#include "G4FoamXrayTRmodel.hh"
#include "G4RegularXrayTRmodel.hh"
#include "G4GamDistrXrayTRmodel.hh"
#include "G4PlateIrrGasXrayTRmodel.hh"

#include "G4Material.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4UniformMagField.hh"
#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"
#include "G4SDManager.hh"
#include "G4RunManager.hh"

#include "G4ios.hh"

/////////////////////////////////////////////////////////////////////////////
//
//

Em8DetectorConstruction::Em8DetectorConstruction()
:solidWorld(NULL),logicWorld(NULL),physiWorld(NULL),
 solidAbsorber(NULL),logicAbsorber(NULL),physiAbsorber(NULL),
 AbsorberMaterial(NULL),WorldMaterial(NULL),fRadiatorMat(NULL),
 magField(NULL),calorimeterSD(NULL),worldchanged(false)
{
  // default parameter values of the calorimeter

  G4double inch = 2.54*cm ;
  G4double  mil = inch/1000.0 ;

  WorldSizeZ = 80.*cm;
  WorldSizeR = 20.*cm;

  AbsorberThickness = 40.0*mm;

  AbsorberRadius   = 10.*cm;
  zAbsorber = 36.*cm ;

  fWindowThick = 51.0*micrometer ;
  fElectrodeThick = 10.0*micrometer ;
  fGapThick = 1.0*mm ;

  fRadThickness = 25*micrometer ;   // 0.5*mil ;   
  fGasGap       = 1500*micrometer  ;    // 30*mil ;    
  fFoilNumber   = 188 ;

  fDetThickness = 40.0*mm ;
  fDetLength    = 200.0*cm  ;
  fDetGap       = 1.0*mm ;

  fStartR       = 40*cm  ;
  fStartZ       = 10.0*mm  ;

  fModuleNumber = 1      ;  

  // create commands for interactive definition of the calorimeter  

  detectorMessenger = new Em8DetectorMessenger(this);
}

//////////////////////////////////////////////////////////////////////////
//
//

Em8DetectorConstruction::~Em8DetectorConstruction()
{ 
  delete detectorMessenger;
}

//////////////////////////////////////////////////////////////////////////
//
//

G4VPhysicalVolume* Em8DetectorConstruction::Construct()
{
  DefineMaterials();
  return ConstructCalorimeter();
}

//////////////////////////////////////////////////////////////////////////////
//
//

void Em8DetectorConstruction::DefineMaterials()
{ 
 //This function illustrates the possible ways to define materials
 
G4String name, symbol ;             //a=mass of a mole;
G4double a, z, density ;            //z=mean number of protons;  
G4int iz, n, nel ;                       //iz=number of protons  in an isotope; 
                                   // n=number of nucleons in an isotope;

G4int ncomponents, natoms;
G4double abundance, fractionmass;
G4double temperature, pressure;

//
// define Elements
//

  a = 1.01*g/mole;
  G4Element* elH  = new G4Element(name="Hydrogen",symbol="H" , z= 1., a);

  a = 12.01*g/mole;
  G4Element* elC = new G4Element(name="Carbon", symbol="C", z=6., a);

  a = 14.01*g/mole;
  G4Element* elN  = new G4Element(name="Nitrogen",symbol="N" , z= 7., a);

  a = 16.00*g/mole;
  G4Element* elO  = new G4Element(name="Oxygen"  ,symbol="O" , z= 8., a);

  a = 39.948*g/mole;
  G4Element* elAr = new G4Element(name="Argon", symbol="Ar", z=18., a);

  a = 131.29*g/mole;
  G4Element* elXe = new G4Element(name="Xenon", symbol="Xe", z=54., a);
  
  a = 19.00*g/mole;
  G4Element* elF  = new G4Element(name="Fluorine", symbol="F", z=9., a);


//
// define simple materials
//

     /* ******************************************************************

density = 1.848*g/cm3;
a = 9.01*g/mole;
G4Material* Be = new G4Material(name="Beryllium", z=4., a, density);


density = 1.390*g/cm3;
a = 39.95*g/mole;
G4Material* lAr = new G4Material(name="liquidArgon", z=18., a, density);

density = 7.870*g/cm3;
a = 55.85*g/mole;
G4Material* Fe = new G4Material(name="Iron"   , z=26., a, density);

density = 8.960*g/cm3;
a = 63.55*g/mole;
G4Material* Cu = new G4Material(name="Copper"   , z=29., a, density);

density = 19.32*g/cm3;
a =196.97*g/mole;
G4Material* Au = new G4Material(name="Gold"   , z=79., a, density);

density = 11.35*g/cm3;
a = 207.19*g/mole;
G4Material* Pb = new G4Material(name="Lead"     , z=82., a, density);

//
// define a material from elements.   case 1: chemical molecule
//

density = 1.000*g/cm3;
G4Material* H2O = new G4Material(name="Water", density, ncomponents=2);
H2O->AddElement(elH, natoms=2);
H2O->AddElement(elO, natoms=1);

  // Kapton (polyimide) ??? since = Mylar C5H4O2

  density = 1.39*g/cm3;
  G4Material* Kapton = new G4Material(name="Kapton", density, nel=3);
  Kapton->AddElement(elO,2);
  Kapton->AddElement(elC,5);
  Kapton->AddElement(elH,4);

  // Silicon as detector material

  density = 2.330*g/cm3;
  a = 28.09*g/mole;
  G4Material* Si = new G4Material(name="Silicon", z=14., a, density);

  // Carbon dioxide

  density = 1.977*mg/cm3;
  G4Material* CO2 = new G4Material(name="CO2", density, nel=2,
				       kStateGas,273.15*kelvin,1.*atmosphere);
  CO2->AddElement(elC,1);
  CO2->AddElement(elO,2);


  // TRT_CH2
      
  density = 0.935*g/cm3;
  G4Material* TRT_CH2 = new G4Material(name="TRT_CH2",density, nel=2);
  TRT_CH2->AddElement(elC,1);
  TRT_CH2->AddElement(elH,2);

  // Radiator

  density = 0.059*g/cm3;
  G4Material* Radiator = new G4Material(name="Radiator",density, nel=2);
  Radiator->AddElement(elC,1);
  Radiator->AddElement(elH,2);

  // Carbon Fiber

  density = 0.145*g/cm3;
  G4Material* CarbonFiber = new G4Material(name="CarbonFiber",density, nel=1);
  CarbonFiber->AddElement(elC,1);

  density = 1.290*mg/cm3;  // old air from elements
  G4Material* air = new G4Material(name="air"  , density, ncomponents=2);
  Air->AddElement(elN, fractionmass=0.7);
  Air->AddElement(elO, fractionmass=0.3);


  density = 1.25053*mg/cm3 ;       // STP
  a = 14.01*g/mole ;       // get atomic weight !!!
  //  a = 28.016*g/mole;
  G4Material* N2  = new G4Material(name="Nitrogen", z= 7.,a,density) ;

  density = 1.25053*mg/cm3 ;       // STP
  G4Material* anotherN2 = new G4Material(name="anotherN2", density,ncomponents=2);
  anotherN2->AddElement(elN, 1);
  anotherN2->AddElement(elN, 1);

************************ */

  // Al for electrodes

  density = 2.700*g/cm3;
  a = 26.98*g/mole;
  G4Material* Al = new G4Material(name="Aluminium", z=13., a, density);

  // Mylar

  density = 1.39*g/cm3;
  G4Material* Mylar = new G4Material(name="Mylar", density, nel=3);
  Mylar->AddElement(elO,2);
  Mylar->AddElement(elC,5);
  Mylar->AddElement(elH,4);

  // Polypropelene

  G4Material* CH2 = new G4Material ("Polypropelene" , 0.91*g/cm3, 2);
  CH2->AddElement(elH,2);
  CH2->AddElement(elC,1);



  // Krypton as detector gas, STP

  density = 3.700*mg/cm3 ;
  a = 83.80*g/mole ;
  G4Material* Kr  = new G4Material(name="Kr",z=36., a, density );

  // Metane, STP

  //  density = 0.7174*mg/cm3 ;
  //  G4Material* metane = new G4Material(name="CH4",density,nel=2) ;
  //  metane->AddElement(elC,1) ;
  //  metane->AddElement(elH,4) ;


  // Dry air (average composition)

  density = 1.7836*mg/cm3 ;       // STP
  G4Material* Argon = new G4Material(name="Argon"  , density, ncomponents=1);
  Argon->AddElement(elAr, 1);

  density = 1.25053*mg/cm3 ;       // STP
  G4Material* Nitrogen = new G4Material(name="N2"  , density, ncomponents=1);
  Nitrogen->AddElement(elN, 2);

  density = 1.4289*mg/cm3 ;       // STP
  G4Material* Oxygen = new G4Material(name="O2"  , density, ncomponents=1);
  Oxygen->AddElement(elO, 2);


  density = 1.2928*mg/cm3 ;       // STP
  G4Material* Air = new G4Material(name="Air"  , density, ncomponents=3);
  Air->AddMaterial( Nitrogen, fractionmass = 0.7557 ) ;
  Air->AddMaterial( Oxygen,   fractionmass = 0.2315 ) ;
  Air->AddMaterial( Argon,    fractionmass = 0.0128 ) ;

  // 93% Ar + 7% CH4, STP

  //  density = 1.709*mg/cm3 ;      
  //  G4Material* Ar7CH4 = new G4Material(name="Ar7CH4"  , density, 
  //                                                           ncomponents=2);
  //  Ar7CH4->AddMaterial( Argon,    fractionmass = 0.971 ) ;
  //  Ar7CH4->AddMaterial( metane,   fractionmass = 0.029 ) ;

  // 93% Kr + 7% CH4, STP

  //  density = 3.491*mg/cm3 ;      
  //  G4Material* Kr7CH4 = new G4Material(name="Kr7CH4"  , density, 
  //                                                   ncomponents=2);
  //  Kr7CH4->AddMaterial( Kr,       fractionmass = 0.986 ) ;
  //  Kr7CH4->AddMaterial( metane,   fractionmass = 0.014 ) ;

  /* **************

  G4double TRT_Xe_density = 5.485*mg/cm3;
  G4Material* TRT_Xe = new G4Material(name="TRT_Xe", TRT_Xe_density, nel=1,
				      kStateGas,293.15*kelvin,1.*atmosphere);
  TRT_Xe->AddElement(elXe,1);

  G4double TRT_CO2_density = 1.842*mg/cm3;
  G4Material* TRT_CO2 = new G4Material(name="TRT_CO2", TRT_CO2_density, nel=2,
				       kStateGas,293.15*kelvin,1.*atmosphere);
  TRT_CO2->AddElement(elC,1);
  TRT_CO2->AddElement(elO,2);

  G4double TRT_CF4_density = 3.9*mg/cm3;
  G4Material* TRT_CF4 = new G4Material(name="TRT_CF4", TRT_CF4_density, nel=2,
                                       kStateGas,293.15*kelvin,1.*atmosphere);
  TRT_CF4->AddElement(elC,1);
  TRT_CF4->AddElement(elF,4);

  // ATLAS TRT straw tube gas mixture (20 C, 1 atm)

  G4double XeCO2CF4_density = 4.76*mg/cm3;
  G4Material* XeCO2CF4 = new G4Material(name="XeCO2CF4", XeCO2CF4_density,
					ncomponents=3,
					kStateGas,293.15*kelvin,1.*atmosphere);
  XeCO2CF4->AddMaterial(TRT_Xe,0.807);
  XeCO2CF4->AddMaterial(TRT_CO2,0.039);
  XeCO2CF4->AddMaterial(TRT_CF4,0.154);

  *********** */

  // Xenon as detector gas, STP

  density = 5.858*mg/cm3 ;
  a = 131.29*g/mole ;
  G4Material* Xe  = new G4Material(name="Xenon",z=54., a, density );

  // Carbon dioxide, STP

  density = 1.977*mg/cm3;
  G4Material* CarbonDioxide = new G4Material(name="CO2", density, nel=2);
  CarbonDioxide->AddElement(elC,1);
  CarbonDioxide->AddElement(elO,2);

  // 80% Ar + 20% CO2, STP

//  density = 1.8223*mg/cm3 ;      
//  G4Material* Ar_80CO2_20 = new G4Material(name="ArCO2"  , density, 
//                                    ncomponents=2);
//  Ar_80CO2_20->AddMaterial( Argon,           fractionmass = 0.783 ) ;
//  Ar_80CO2_20->AddMaterial( CarbonDioxide,   fractionmass = 0.217 ) ;

  // 80% Xe + 20% CO2, STP

  density = 5.0818*mg/cm3 ;      
  G4Material* Xe20CO2 = new G4Material(name="Xe20CO2"  , density, ncomponents=2);
  Xe20CO2->AddMaterial( Xe,              fractionmass = 0.922 ) ;
  Xe20CO2->AddMaterial( CarbonDioxide,   fractionmass = 0.078 ) ;

  // 80% Kr + 20% CO2, STP

  density = 3.601*mg/cm3 ;      
  G4Material* Kr20CO2 = new G4Material(name="Kr20CO2"  , density, 
                                                             ncomponents=2);
  Kr20CO2->AddMaterial( Kr,              fractionmass = 0.89 ) ;
  Kr20CO2->AddMaterial( CarbonDioxide,   fractionmass = 0.11 ) ;


  G4cout << *(G4Material::GetMaterialTable()) << G4endl;

  //default materials of the calorimeter and TR radiator

  fRadiatorMat =  Mylar ; // CH2 ;   // Mylar ; 
  
  fWindowMat = Mylar ;
  fElectrodeMat = Al ;

  AbsorberMaterial = Kr20CO2 ;   // XeCO2CF4  ; 
  fGapMat          = Kr20CO2 ;

  WorldMaterial    = Air ;
}

/////////////////////////////////////////////////////////////////////////
//
//
  
G4VPhysicalVolume* Em8DetectorConstruction::ConstructCalorimeter()
{
  G4int i, j ; 
  G4double zModule, zRadiator, rModule, rRadiator ; 

  // complete the Calor parameters definition and Print 

  ComputeCalorParameters();
  PrintCalorParameters();
      
  // World
  
  if(solidWorld) delete solidWorld ;
  if(logicWorld) delete logicWorld ;
  if(physiWorld) delete physiWorld ;

  solidWorld = new G4Tubs("World",				//its name
                   0.,WorldSizeR,WorldSizeZ/2.,0.,twopi)       ;//its size
                         
  logicWorld = new G4LogicalVolume(solidWorld,		//its solid
                                   WorldMaterial,	//its material
                                   "World");		//its name
                                   
  physiWorld = new G4PVPlacement(0,			//no rotation
  				 G4ThreeVector(),	//at (0,0,0)
                                 "World",		//its name
                                 logicWorld,		//its logical volume
                                 NULL,			//its mother  volume
                                 false,			//no boolean operation
                                 0);			//copy number

  // TR radiator envelope

  G4double radThick = fFoilNumber*(fRadThickness + fGasGap) + fDetGap   ;

  G4double zRad = fStartZ + 0.5*radThick ;
  G4cout<<"zRad = "<<zRad/mm<<" mm"<<G4endl ;

  radThick *= 1.2 ;
  G4cout<<"radThick = "<<radThick/mm<<" mm"<<G4endl ;

  G4Tubs* solidRadiator = new G4Tubs("Radiator", 0.0, 1.1*AbsorberRadius, 
                                      0.5*radThick, 0.0, twopi  ) ; 
                         
  G4LogicalVolume* logicRadiator = new G4LogicalVolume(solidRadiator,	
                                                       WorldMaterial,      
                                                       "Radiator");	       
                                   
  G4VPhysicalVolume* physiRadiator = new G4PVPlacement(0,
                                     G4ThreeVector(0,0,zRad),	        
                                     "Radiator", logicRadiator,		
                                     physiWorld, false,	0       );  	

  

    fSolidRadSlice = new G4Tubs("RadSlice",0.0,
                                AbsorberRadius,0.5*fRadThickness,0.0,360*deg);

    fLogicRadSlice = new G4LogicalVolume(fSolidRadSlice,fRadiatorMat,
                                          "RadSlice",0,0,0);

    //   fPhysicRadSlice = new G4PVPlacement(0,
    //      G4ThreeVector(0.,0.,fStartZ+1.2*fDetThickness),
    //             "RadSlice",fLogicRadSlice,
    //               physiWorld,false,0);
    /*
  for(i=0;i<fModuleNumber;i++)
  {
    //   rModule = fStartR + fDetThickness + fDetGap + 
    //           (i-1)*(fFoilNumber*(fRadThickness + fGasGap) + 
    //           fDetThickness + fDetGap) ;

    zModule = fStartZ + fRadThickness + 
              i*( fFoilNumber*(fRadThickness + fGasGap) + 
              fDetThickness + fDetGap )  ;
    G4cout<<"zModule = "<<zModule/mm<<" mm"<<G4endl ;
    G4cout<<"i = "<<i<<"\t"<<G4endl ; 

    for(j=0;j<fFoilNumber;j++)
    {  
      //   rRadiator = rModule + j*(fRadThickness + fGasGap) ;

      zRadiator = zModule + j*(fRadThickness + fGasGap) ;
      G4cout<<zRadiator/mm<<" mm"<<"\t" ;
      //   G4cout<<"j = "<<j<<"\t" ;         
      // RadRing

            
      // fSolidRadRing = new G4Tubs("RadRing",rRadiator,
      //        rRadiator + fRadThickness,
      //     fDetLength,0.0,360*deg     ) ;

      //  fLogicRadRing = new G4LogicalVolume(fSolidRadRing,fRadiatorMat,
      //                         "radRing",0,0,0);
      
      //  fPhysicRadRing = new G4PVPlacement(0,G4ThreeVector(),
      //       "RadRing",fLogicRadRing,
      //               physiWorld,false,j)  ; 
                                                            
      // We put slice relatively of Radiator, so zRadiator-zRad
      
      fPhysicRadSlice = new G4PVPlacement(0,G4ThreeVector(0.,0.,zRadiator-zRad),
                                         "RadSlice",fLogicRadSlice,
                                          physiRadiator,false,j);
     }                                 
    //   fPhysicDetSlice = new G4PVPlacement(0,
    //          G4ThreeVector(0.,0.,zRadiator+
    //                        fDetGap +0.5*fDetThickness),"DetSlice",
    //                        fLogicDetSlice,physiWorld,false,i); 
  }                                            
  G4cout<<G4endl ;
    */
  G4Tubs* solidElectrode = new G4Tubs("Electrode",0.,AbsorberRadius,
                                       fElectrodeThick/2.,0.,twopi); 
                          
  G4LogicalVolume* logicElectrode = new G4LogicalVolume(solidElectrode,
                                        fElectrodeMat, "Electrode"); 

  G4double zElectrode = zAbsorber - AbsorberThickness/2. - 
                        fElectrodeThick/2. - 0.01*mm;    
      			                  
  G4VPhysicalVolume*    physiElectrode = new G4PVPlacement(0,		   
      		                       G4ThreeVector(0.,0.,zElectrode),        
                                        "Electrode",logicElectrode,
                                         physiWorld,false,0);    
  


  G4Tubs* solidGap = new G4Tubs("Gap",0.,AbsorberRadius,fGapThick/2.,0.,twopi); 
                          
  G4LogicalVolume* logicGap = new G4LogicalVolume(solidGap,fGapMat, "Gap"); 

  G4double zGap = zElectrode - fElectrodeThick/2. - fGapThick/2. - 0.01*mm ;    
      			                  
  G4VPhysicalVolume*    physiGap = new G4PVPlacement(0,		   
      		                       G4ThreeVector(0.,0.,zGap),        
                                        "Gap",logicGap,physiWorld,false,0); 

  G4Tubs* solidWindow = new G4Tubs("Window",0.,AbsorberRadius,
                                    fWindowThick/2.,0.,twopi); 
                          
  G4LogicalVolume* logicWindow = new G4LogicalVolume(solidWindow,
                                     fWindowMat, "Window"); 

  G4double zWindow = zGap - fGapThick/2. - fWindowThick/2. - 0.01*mm ;    
      			                  
  G4VPhysicalVolume*    physiWindow = new G4PVPlacement(0,		   
      		                       G4ThreeVector(0.,0.,zWindow),        
                                        "Window",logicWindow,physiWorld,false,0); 
                             
  // Absorber

  if (AbsorberThickness > 0.) 
  { 
      if(solidAbsorber) delete solidAbsorber ;
      if(logicAbsorber) delete logicAbsorber ;
      if(physiAbsorber) delete physiAbsorber ;

      solidAbsorber = new G4Tubs("Absorber",		
                          0.,AbsorberRadius,AbsorberThickness/2.,0.,twopi); 
                          
      logicAbsorber = new G4LogicalVolume(solidAbsorber,    
      			                  AbsorberMaterial, 
      			                  "Absorber");     
      			                  
      physiAbsorber = new G4PVPlacement(0,		   
      		    G4ThreeVector(0.,0.,zAbsorber),        
                                        "Absorber",        
                                        logicAbsorber,     
                                        physiWorld,       
                                        false,             
                                        0);                
                                        
  }
                                 
  // Sensitive Detectors: Absorber 
  
  G4SDManager* SDman = G4SDManager::GetSDMpointer();

  if(!calorimeterSD)
  {
    calorimeterSD = new Em8CalorimeterSD("CalorSD",this);
    SDman->AddNewDetector( calorimeterSD );
  }
  if (logicAbsorber)  logicAbsorber->SetSensitiveDetector(calorimeterSD);

  // Parameterisation

  //   G4VXrayTRmodel* pTRModel = new G4IrregularXrayTRmodel(logicRadiator,
  //        fRadThickness,fGasGap);

  //  G4VXrayTRmodel* pTRModel = new G4FoamXrayTRmodel(logicRadiator,
  //                                    fRadThickness,fGasGap);

  //   G4VXrayTRmodel* pTRModel = new G4RegularXrayTRmodel(logicRadiator,
  //                                                       fRadThickness,fGasGap);

  G4double alphaPlate = 160.0 ;
  G4double alphaGas   = 160.0 ;

  //  G4VXrayTRmodel* pTRModel = new G4GamDistrXrayTRmodel(logicRadiator,
  //					       fRadThickness,alphaPlate,
  //                                              fGasGap,alphaGas);

  //  G4VXrayTRmodel* pTRModel = new G4PlateIrrGasXrayTRmodel(logicRadiator,
  //       fRadThickness,fGasGap);

  //  pTRModel->GetPlateZmuProduct() ;
  //  pTRModel->GetGasZmuProduct() ;

  //  pTRModel->GetNumberOfPhotons() ;  
  
  // always return physics world

  return physiWorld;
}

////////////////////////////////////////////////////////////////////////////
//
//

void Em8DetectorConstruction::PrintCalorParameters()
{
  G4cout << "\n The  WORLD   is made of " 
       << WorldSizeZ/mm << "mm of " << WorldMaterial->GetName() ;
  G4cout << ", the transverse size (R) of the world is " << WorldSizeR/mm << " mm. " << G4endl;
  G4cout << " The ABSORBER is made of " 
       << AbsorberThickness/mm << "mm of " << AbsorberMaterial->GetName() ;
  G4cout << ", the transverse size (R) is " << AbsorberRadius/mm << " mm. " << G4endl;
  G4cout << " Z position of the (middle of the) absorber " << zAbsorber/mm << "  mm." << G4endl;
  G4cout << G4endl;
}

///////////////////////////////////////////////////////////////////////////
//
//

void Em8DetectorConstruction::SetAbsorberMaterial(G4String materialChoice)
{
  // get the pointer to the material table
  const G4MaterialTable* theMaterialTable = G4Material::GetMaterialTable();

  // search the material by its name   
  G4Material* pttoMaterial;
  for (size_t J=0 ; J<theMaterialTable->size() ; J++)
   { pttoMaterial = (*theMaterialTable)[J];     
     if(pttoMaterial->GetName() == materialChoice)
        {AbsorberMaterial = pttoMaterial;
         logicAbsorber->SetMaterial(pttoMaterial); 
        // PrintCalorParameters();
        }             
   }
}

////////////////////////////////////////////////////////////////////////////
//
//

void Em8DetectorConstruction::SetWorldMaterial(G4String materialChoice)
{
  // get the pointer to the material table
  const G4MaterialTable* theMaterialTable = G4Material::GetMaterialTable();

  // search the material by its name   
  G4Material* pttoMaterial;
  for (size_t J=0 ; J<theMaterialTable->size() ; J++)
   { pttoMaterial = (*theMaterialTable)[J];     
     if(pttoMaterial->GetName() == materialChoice)
        {WorldMaterial = pttoMaterial;
         logicWorld->SetMaterial(pttoMaterial); 
       //  PrintCalorParameters();
        }             
   }
}

///////////////////////////////////////////////////////////////////////////
//
//

void Em8DetectorConstruction::SetAbsorberThickness(G4double val)
{
  // change Absorber thickness and recompute the calorimeter parameters
  AbsorberThickness = val;
  ComputeCalorParameters();
}  

/////////////////////////////////////////////////////////////////////////////
//
//

void Em8DetectorConstruction::SetAbsorberRadius(G4double val)
{
  // change the transverse size and recompute the calorimeter parameters
  AbsorberRadius = val;
  ComputeCalorParameters();
}  

////////////////////////////////////////////////////////////////////////////
//
//

void Em8DetectorConstruction::SetWorldSizeZ(G4double val)
{
  worldchanged=true;
  WorldSizeZ = val;
  ComputeCalorParameters();
}  

///////////////////////////////////////////////////////////////////////////
//
//

void Em8DetectorConstruction::SetWorldSizeR(G4double val)
{
  worldchanged=true;
  WorldSizeR = val;
  ComputeCalorParameters();
}  

//////////////////////////////////////////////////////////////////////////////
//
//

void Em8DetectorConstruction::SetAbsorberZpos(G4double val)
{
  zAbsorber  = val;
  ComputeCalorParameters();
}  

//////////////////////////////////////////////////////////////////////////////
//
//

void Em8DetectorConstruction::SetMagField(G4double fieldValue)
{
  //apply a global uniform magnetic field along X axis

  /* *********************************************************

  G4FieldManager* fieldMgr 
   = G4TransportationManager::GetTransportationManager()->GetFieldManager();
    
  if(magField) delete magField;		//delete the existing magn field
  
  if(fieldValue!=0.)			// create a new one if non nul
  { 
    magField = new G4UniformMagField(G4ThreeVector(fieldValue,0.,0.));        
    fieldMgr->SetDetectorField(magField);
    fieldMgr->CreateChordFinder(magField);
  } 
  else 
  {
    magField = NULL;
    fieldMgr->SetDetectorField(magField);
  }

  *************************************************************** */

}

///////////////////////////////////////////////////////////////////////////////
//
//
  
void Em8DetectorConstruction::UpdateGeometry()
{
  G4RunManager::GetRunManager()->DefineWorldVolume(ConstructCalorimeter());
}

//
//
////////////////////////////////////////////////////////////////////////////

















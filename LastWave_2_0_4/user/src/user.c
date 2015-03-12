

#include "lastwave.h"



/*
 * This is the C-function that is called at initialization of LastWave
 * You should declare your packages here
 */


void UserInit(void) 
{ 
  extern void DeclareSignalPackage(void);
  extern void DeclareWtrans1dPackage(void);
  extern void DeclareExtrema1dPackage(void);
  extern void DeclareWtmm1dPackage(void);
  extern void DeclareImagePackage(void);
  extern void DeclareStftPackage(void);
  extern void DeclareOWtrans2Package(void);
  extern void DeclareCompress2Package(void);
  extern void DeclareDWtrans2dPackage(void);
  extern void DeclareMpPackage(void);
  extern void DeclareSoundPackage(void);
/*  extern void DeclareCirclesPackage(void); */
 
  DeclareSignalPackage();
  DeclareSoundPackage();

  DeclareWtrans1dPackage();
  DeclareExtrema1dPackage();

  DeclareWtmm1dPackage();

  DeclareImagePackage();

  DeclareStftPackage(); 
  DeclareMpPackage();

  DeclareOWtrans2Package();
  
  DeclareCompress2Package();
    
  DeclareDWtrans2dPackage();
  
/*  DeclareCirclesPackage(); */
} 

 
 

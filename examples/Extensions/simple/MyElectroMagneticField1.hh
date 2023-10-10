//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * This file was obtained from Topas MC Inc under the license       *
// * agreement set forth at http://www.topasmc.org/registration       *
// * Any use of this file constitutes full acceptance of              *
// * this TOPAS MC license agreement.                                 *
// *                                                                  *
// ********************************************************************
//

#ifndef MyElectroMagneticField1_hh
#define MyElectroMagneticField1_hh

#include "TsVElectroMagneticField.hh"

class MyElectroMagneticField1 : public TsVElectroMagneticField
{
public:
	MyElectroMagneticField1(TsParameterManager* pM, TsGeometryManager* gM,
						  TsVGeometryComponent* component);
	~MyElectroMagneticField1();
	
	void GetFieldValue(const G4double[4], G4double *fieldBandE) const;
	void ResolveParameters();
};

#endif

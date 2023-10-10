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

#ifndef MyMagneticField1_hh
#define MyMagneticField1_hh

#include "TsVMagneticField.hh"

class MyMagneticField1 : public TsVMagneticField
{
public:
	MyMagneticField1(TsParameterManager* pM, TsGeometryManager* gM,
						  TsVGeometryComponent* component);
	~MyMagneticField1();
	
	void GetFieldValue(const double p[3], double* Field) const;
	void ResolveParameters();
};

#endif

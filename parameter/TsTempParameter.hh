//
// ********************************************************************
// *                                                                  *
// * Copyright 2024 The TOPAS Collaboration                           *
// * Copyright 2022 The TOPAS Collaboration                           *
// *                                                                  *
// * Permission is hereby granted, free of charge, to any person      *
// * obtaining a copy of this software and associated documentation   *
// * files (the "Software"), to deal in the Software without          *
// * restriction, including without limitation the rights to use,     *
// * copy, modify, merge, publish, distribute, sublicense, and/or     *
// * sell copies of the Software, and to permit persons to whom the   *
// * Software is furnished to do so, subject to the following         *
// * conditions:                                                      *
// *                                                                  *
// * The above copyright notice and this permission notice shall be   *
// * included in all copies or substantial portions of the Software.  *
// *                                                                  *
// * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,  *
// * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES  *
// * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND         *
// * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT      *
// * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,     *
// * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
// * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR    *
// * OTHER DEALINGS IN THE SOFTWARE.                                  *
// *                                                                  *
// ********************************************************************
//

#ifndef TsTempParameter_hh
#define TsTempParameter_hh

#include "globals.hh"

class TsTempParameter
	{
	public:
		TsTempParameter(const G4String& name, const G4String& type, const G4String& value, G4bool isChangeable):
		fName(name), fType(type), fValue(value), fIsChangeable(isChangeable), fMustBeAbsolute(false) {};

		TsTempParameter() {};

		~TsTempParameter() {};

		const G4String& GetName()const{return fName;};
		const G4String& GetType()const{return fType;};
		const G4String& GetValue()const{return fValue;};
		const G4bool& IsChangeable()const{return fIsChangeable;};
		const G4bool& MustBeAbsolute()const{return fMustBeAbsolute;};

		void SetName(const G4String& name) {fName = name;};
		void SetType(const G4String& type) {fType = type;};
		void SetChangeable(const G4bool& isChangeable) {fIsChangeable = isChangeable;};
		void SetMustBeAbsolute() {fMustBeAbsolute = true;};

	private:
		G4String fName;
		G4String fType;
		G4String fValue;
		G4bool fIsChangeable;
		G4bool fMustBeAbsolute;
	};

#endif

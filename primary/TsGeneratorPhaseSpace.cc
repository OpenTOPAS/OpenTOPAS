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

#include "TsGeneratorPhaseSpace.hh"

#include "TsParameterManager.hh"
#include "TsSourcePhaseSpace.hh"

#include "G4Event.hh"

TsGeneratorPhaseSpace::TsGeneratorPhaseSpace(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName) :
TsVGenerator(pM, gM, pgM, sourceName)
{
	fParticleBuffer = new std::queue<TsPrimaryParticle>;

	ResolveParameters();
	CacheGeometryPointers();
}


TsGeneratorPhaseSpace::~TsGeneratorPhaseSpace()
{
}


void TsGeneratorPhaseSpace::ResolveParameters() {
}


void TsGeneratorPhaseSpace::UpdateForNewRun(G4bool rebuiltSomeComponents) {
	TsVGenerator::UpdateForNewRun(rebuiltSomeComponents);
	ResolveParameters();
}


void TsGeneratorPhaseSpace::ClearGenerator() {
	while(!fParticleBuffer->empty())
		fParticleBuffer->pop();
}


void TsGeneratorPhaseSpace::GeneratePrimaries(G4Event* anEvent)
{
    if (!CurrentSourceHasGeneratedEnough()) {

        // If the buffer is empty, read a batch of histories from the file into the buffer
        if (fParticleBuffer->empty()) {
            ((TsSourcePhaseSpace*)fPs)->ReadSomeDataFromFileToBuffer(fParticleBuffer);
        }

        // Get the first particle from the buffer
        // Confirm that first particle in buffer is a new history
        TsPrimaryParticle p = fParticleBuffer->front();

        // Confirm that first particle is a new history (error if it's not)
        if (!p.isNewHistory) {
            G4cerr << "TsGeneratorPhaseSpace::GeneratePrimaries error: First particle in buffer not a new history." << G4endl;
            fPm->AbortSession(1);
        }

        // Generate all of particles in the current history.
		// Depending on upstream interactions, this may or may not contain the original primary,
		// and this may or may not contain some number of secondaries.
		// The first particle we encounter is guaranteed to have the IsNewHistory flag
		// (since our previous loop will have used up all the particles from the previous history).
		// We keep looping until we find another IsNewHistory or we hit the end of the file.
		G4bool addMoreParticlesToThisHistory = true;

        while ( addMoreParticlesToThisHistory ) {
            if (p.particleDefinition) {
                TransformPrimaryForComponent(&p);
                GenerateOnePrimary(anEvent, p);
            }

            // Discard particle we've handled.
            fParticleBuffer->pop();

            // See if there are more particles in the buffer
            if (fParticleBuffer->size() == 0) {
                addMoreParticlesToThisHistory = false;
            } else {
                p = fParticleBuffer->front();
                if (p.isNewHistory)
                    addMoreParticlesToThisHistory = false;
            }
        }

		AddPrimariesToEvent(anEvent);
    }
}

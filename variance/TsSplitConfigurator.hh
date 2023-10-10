#ifndef TsSplitConfigurator_hh
#define TsSplitConfigurator_hh TsSplitConfigurator_hh

#include "G4ProcessPlacer.hh"
#include "G4VSamplerConfigurator.hh"

class TsParameterManager;
class TsGeometryManager;
class TsSplitProcess;
class TsIStore;
class G4VPhysicalVolume;

class TsSplitConfigurator : public G4VSamplerConfigurator
{
public:
	TsSplitConfigurator(TsParameterManager* pm, TsGeometryManager* gm,
				const G4String worldVolume,
				const G4String& particleName,
				TsIStore &istore, G4bool paraflag);

	virtual ~TsSplitConfigurator();
	virtual void Configure(G4VSamplerConfigurator* preConf);
	virtual const G4VTrackTerminator *GetTrackTerminator() const;

	void SetWorldName(G4String name);

private:
	TsSplitConfigurator(const TsSplitConfigurator&) = delete;
	TsSplitConfigurator& operator=(const TsSplitConfigurator&) = delete;

	TsParameterManager* fPm;
	TsGeometryManager* fGm;

	const G4VPhysicalVolume* fWorld;
	G4String fWorldName;
	G4ProcessPlacer fPlacer;
	TsIStore &fIStore;

	TsSplitProcess* fSplitProcess;

	G4bool fParaFlag;

};

#endif

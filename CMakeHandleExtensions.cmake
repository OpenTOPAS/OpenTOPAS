configure_file (
	"${CMAKE_CURRENT_SOURCE_DIR}/extensions/TsExtensionManager.cc.in"
	"${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc"
)

configure_file (
	"${CMAKE_CURRENT_SOURCE_DIR}/extensions/TsExtensionManager.hh.in"
	"${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.hh"
)

configure_file (
	"${CMAKE_CURRENT_SOURCE_DIR}/extensions/CMakeLists.txt.in"
	"${CMAKE_CURRENT_BINARY_DIR}/extensions/CMakeLists.txt"
)

# If there are user extensions, modify files to use them
set (TOPAS_EXTENSIONS_DIR "${TOPAS_EXTENSIONS_DIR}" CACHE PATH "/path/to/topas/extensions")
if (TOPAS_EXTENSIONS_DIR)

    file(GLOB oldfiles "${CMAKE_CURRENT_BINARY_DIR}/extensions/*.cc" "${CMAKE_CURRENT_BINARY_DIR}/extensions/*.hh")
    list(REMOVE_ITEM oldfiles "${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc")
    list(REMOVE_ITEM oldfiles "${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.hh")
    if (oldfiles)
	file(REMOVE ${oldfiles})
    endif()

    foreach(onetopdir ${TOPAS_EXTENSIONS_DIR})
		get_filename_component(ONE_EXTENSIONS_DIR "${onetopdir}" ABSOLUTE)
		file(GLOB_RECURSE extfiles "${ONE_EXTENSIONS_DIR}/*.cc")

		foreach(item ${extfiles})
			if (${item} MATCHES "/\\.")
					list(REMOVE_ITEM extfiles ${item})
			endif()
		endforeach(item)

		if (NOT extfiles)
			message (FATAL_ERROR "No extension files were found in directory: " ${ONE_EXTENSIONS_DIR})
		else()
			message ("Found extensions in directory: " ${ONE_EXTENSIONS_DIR})

			SET(FILENAME_LIST "")
			foreach (file ${extfiles})
				string(FIND ${file} "/" slashposition REVERSE)
				string(SUBSTRING ${file} ${slashposition} -1 filename)
				string(FIND ${filename} "." dotposition)
				math(EXPR dotposition "${dotposition}-1")
				string(SUBSTRING ${filename} 1 ${dotposition} filename)

				list(FIND FILENAME_LIST ${filename} is_in_list)
				if (NOT ${is_in_list} STREQUAL "-1")
					message (FATAL_ERROR "Attempt to add same extension file twice: ${filename}.cc")
				endif()
				SET(FILENAME_LIST ${FILENAME_LIST} ${filename})

				# file(READ "${file}" contents LIMIT 120)
				FILE (READ "${file}" contents)
				STRING (REGEX REPLACE ";" "\\\\;" contents "${contents}")
				STRING (REGEX REPLACE "\n" ";" contents "${contents}")
				foreach (c IN LISTS contents)
					STRING (STRIP "${c}" c)
					if (NOT c STREQUAL "")
						SET (contents "${c}")
						break ()
					endif ()
				endforeach ()

				string(SUBSTRING ${contents} 3 -1 contents)
				string(FIND ${contents} " for " position)
				if (${position} STREQUAL "-1")
					message ("\nInvalid first line in file: ${filename}.cc")
					message ("All cc files in the extensions directory must start with a special")
					message ("comment line that describes the function of this class.")
					message ("See TOPAS User Guide for details.\n")
					message (FATAL_ERROR)
				endif()

				string(SUBSTRING ${contents} 0 ${position} key)
				string(SUBSTRING ${contents} ${position} -1 valuestring)
				string(SUBSTRING ${valuestring} 5 -1 valuestring)
				string(FIND ${valuestring} "\n" position)
				string(SUBSTRING ${valuestring} 0 ${position} valuestring)
				string(TOLOWER ${valuestring} valuestringlower)
				if (key STREQUAL "Component")
					message ("${file} is a Component for ${valuestring}")
					file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc EXT_CONTENTS)

					string(REGEX REPLACE
					"\#include \"TsParameterManager.hh\""
					"\#include \"TsParameterManager.hh\"
					\#include \"${filename}.hh\""
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"InstantiateComponent\\(TsParameterManager\\*, TsMaterialManager\\*, TsGeometryManager\\*, TsVGeometryComponent\\*, G4VPhysicalVolume\\*, G4String, G4String"
					"InstantiateComponent(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String childCompType, G4String childName"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"Insertion point for Components"
					"Insertion point for Components
					if (childCompType==\"${valuestringlower}\")
					return new ${filename}(pM, this, mM, gM, pgc, pv, childName);"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"TsExtensionManager\\(TsParameterManager\\*\\)"
					"TsExtensionManager(TsParameterManager* pM)"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"Insertion point for Component Type Names"
					"Insertion point for Component Type Names
					pM->RegisterComponentTypeName(\"${valuestring}\");"
					EXT_CONTENTS "${EXT_CONTENTS}")

					file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc "${EXT_CONTENTS}")
				elseif (key STREQUAL "Imaging to Material Converter")
					message ("${file} is an Imaging to Material Converter for ${valuestring}")
					file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc EXT_CONTENTS)

					string(REGEX REPLACE
					"\#include \"TsParameterManager.hh\""
					"\#include \"TsParameterManager.hh\"
					\#include \"${filename}.hh\""
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"InstantiateImagingToMaterial\\(TsParameterManager\\*, TsVGeometryComponent\\*, std::vector<G4Material\\*>\\*, G4String"
					"InstantiateImagingToMaterial(TsParameterManager* pM, TsVGeometryComponent* geometryComponent, std::vector<G4Material*>* materialList, G4String converterName"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"Insertion point for Imaging to Imaging To Material Converter"
					"Insertion point for Imaging to Imaging To Material Converter
					if (converterName==\"${valuestringlower}\")
					return new ${filename}(pM, geometryComponent, materialList);"
					EXT_CONTENTS "${EXT_CONTENTS}")

					file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc "${EXT_CONTENTS}")
				elseif (key STREQUAL "Magnetic Field")
					message ("${file} is a Magnetic Field for ${valuestring}")
					file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc EXT_CONTENTS)

					string(REGEX REPLACE
					"\#include \"TsParameterManager.hh\""
					"\#include \"TsParameterManager.hh\"
					\#include \"${filename}.hh\""
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"InstantiateMagneticField\\(TsParameterManager\\*, TsGeometryManager\\*, TsVGeometryComponent\\*, G4String"
					"InstantiateMagneticField(TsParameterManager* pM, TsGeometryManager* gM, TsVGeometryComponent* geometryComponent, G4String magneticFieldType"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"Insertion point for Magnetic Fields"
					"Insertion point for Magnetic Fields
					if (magneticFieldType==\"${valuestringlower}\")
					return new ${filename}(pM, gM, geometryComponent);"
					EXT_CONTENTS "${EXT_CONTENTS}")

					file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc "${EXT_CONTENTS}")
				elseif (key STREQUAL "ElectroMagnetic Field")
					message ("${file} is a ElectroMagnetic Field for ${valuestring}")
					file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc EXT_CONTENTS)

					string(REGEX REPLACE
					"\#include \"TsParameterManager.hh\""
					"\#include \"TsParameterManager.hh\"
					\#include \"${filename}.hh\""
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"InstantiateElectroMagneticField\\(TsParameterManager\\*, TsGeometryManager\\*, TsVGeometryComponent\\*, G4String"
					"InstantiateElectroMagneticField(TsParameterManager* pM, TsGeometryManager* gM, TsVGeometryComponent* geometryComponent, G4String electroMagneticFieldType"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"Insertion point for ElectroMagnetic Fields"
					"Insertion point for ElectroMagnetic Fields
					if (electroMagneticFieldType==\"${valuestringlower}\")
					return new ${filename}(pM, gM, geometryComponent);"
					EXT_CONTENTS "${EXT_CONTENTS}")

					file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc "${EXT_CONTENTS}")
				elseif (key STREQUAL "Filter")
					message ("${file} is a Filter for ${valuestring}")
					file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc EXT_CONTENTS)

					string(REGEX REPLACE
					"\#include \"TsParameterManager.hh\""
					"\#include \"TsParameterManager.hh\"
					\#include \"${filename}.hh\""
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"InstantiateFilter\\(TsParameterManager\\*, TsMaterialManager\\*, TsGeometryManager\\*, TsFilterManager\\*, TsVGenerator\\*, TsVScorer\\*, TsVFilter\\* parentFilter"
					"InstantiateFilter(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsFilterManager* fM, TsVGenerator* generator, TsVScorer* scorer, TsVFilter* parentFilter"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"\/\/G4String filtername;"
					"G4String filterName;"
					EXT_CONTENTS "${EXT_CONTENTS}")

					# valuestring could be a semicolon-separated list of values
					string(REPLACE "," ";" valuestring ${valuestring})
					list(LENGTH valuestring listlength)

					foreach (onevalue ${valuestring})
					string(TOLOWER ${onevalue} onevaluelower)

					string(REGEX REPLACE
					"Insertion point for Filters"
					"Insertion point for Filters
					filterName = \"${onevaluelower}\";
					pM->RegisterFilterName(filterName);
					if (HaveFilterNamed(pM, generator, scorer, filterName))
					parentFilter = new ${filename}(filterName, pM, mM, gM, fM, generator, scorer, parentFilter);"
					EXT_CONTENTS "${EXT_CONTENTS}")

					endforeach (onevalue)

					file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc "${EXT_CONTENTS}")
				elseif (key STREQUAL "Particle Source")
					message ("${file} is a Particle Source for ${valuestring}")
					file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc EXT_CONTENTS)

					string(REGEX REPLACE
					"\#include \"TsParameterManager.hh\""
					"\#include \"TsParameterManager.hh\"
					\#include \"${filename}.hh\""
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"InstantiateParticleSource\\(TsParameterManager\\*, TsSourceManager\\*, G4String, G4String"
					"InstantiateParticleSource(TsParameterManager* pM, TsSourceManager* psM, G4String sourceType, G4String sourceName"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"Insertion point for Particle Sources"
					"Insertion point for Particle Sources
					if (sourceType==\"${valuestringlower}\")
					return new ${filename}(pM, psM, sourceName);"
					EXT_CONTENTS "${EXT_CONTENTS}")

					file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc "${EXT_CONTENTS}")
				elseif (key STREQUAL "Particle Generator")
					message ("${file} is a Particle Generator for ${valuestring}")
					file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc EXT_CONTENTS)

					string(REGEX REPLACE
					"\#include \"TsParameterManager.hh\""
					"\#include \"TsParameterManager.hh\"
					\#include \"${filename}.hh\""
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"InstantiateParticleGenerator\\(TsParameterManager\\*, TsGeometryManager\\*, TsGeneratorManager\\*, G4String, G4String"
					"InstantiateParticleGenerator(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String generatorType, G4String generatorName"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"Insertion point for Particle Generators"
					"Insertion point for Particle Generators
					if (generatorType==\"${valuestringlower}\")
					return new ${filename}(pM, gM, pgM, generatorName);"
					EXT_CONTENTS "${EXT_CONTENTS}")

					file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc "${EXT_CONTENTS}")
				elseif (key STREQUAL "Scorer")
					message ("${file} is a Scorer for ${valuestring}")
					file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc EXT_CONTENTS)

					string(REGEX REPLACE
					"\#include \"TsParameterManager.hh\""
					"\#include \"TsParameterManager.hh\"
					\#include \"${filename}.hh\""
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"InstantiateScorer\\(TsParameterManager\\*, TsMaterialManager\\*, TsGeometryManager\\*, TsScoringManager\\*, G4String, G4String, G4String, G4String, G4bool"
					"InstantiateScorer(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, G4String currentScorerName, G4String quantityName, G4String quantityNameLower, G4String outFileName, G4bool isSubScorer"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"Insertion point for Scorers"
					"Insertion point for Scorers
					if (quantityNameLower==\"${valuestringlower}\")
					return new ${filename}(pM, mM, gM, scM, this, currentScorerName, quantityName, outFileName, isSubScorer);"
					EXT_CONTENTS "${EXT_CONTENTS}")

					file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc "${EXT_CONTENTS}")
				elseif (key STREQUAL "Outcome Model")
					message ("${file} is a Outcome Model for ${valuestring}")
					file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc EXT_CONTENTS)

					string(REGEX REPLACE
					"\#include \"TsParameterManager.hh\""
					"\#include \"TsParameterManager.hh\"
					\#include \"${filename}.hh\""
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"InstantiateOutcomeModel\\(TsParameterManager\\*, G4String, G4String"
					"InstantiateOutcomeModel(TsParameterManager* pM, G4String parmPrefix, G4String modelName"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"Insertion point for Outcome Models"
					"Insertion point for Outcome Models
					if (modelName==\"${valuestringlower}\")
					return new ${filename}(pM, parmPrefix);"
					EXT_CONTENTS "${EXT_CONTENTS}")

					file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc "${EXT_CONTENTS}")
				elseif (key STREQUAL "Physics List")
					message ("${file} is a Physics List for ${valuestring}")
					file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc EXT_CONTENTS)

					string(REGEX REPLACE
					"\#include \"TsParameterManager.hh\""
					"\#include \"TsParameterManager.hh\"
					\#include \"${filename}.hh\""
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"InstantiatePhysicsList\\(TsParameterManager\\*, G4String"
					"InstantiatePhysicsList(TsParameterManager* pM, G4String physicsListName"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"Insertion point for Physics Lists"
					"Insertion point for Physics Lists
					if (physicsListName==\"${valuestringlower}\")
					return new ${filename}(pM);"
					EXT_CONTENTS "${EXT_CONTENTS}")

					file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc "${EXT_CONTENTS}")
				elseif (key STREQUAL "Physics Module")
					message ("${file} is a Physics Module for ${valuestring}")
					file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc EXT_CONTENTS)

					string(REGEX REPLACE
					"\#include \"TsParameterManager.hh\""
					"\#include \"TsParameterManager.hh\"
					\#include \"${filename}.hh\""
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"InstantiatePhysicsModule\\(TsParameterManager\\*, G4String"
					"InstantiatePhysicsModule(TsParameterManager* pM, G4String physicsModuleName"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"Insertion point for Physics Modules"
					"Insertion point for Physics Modules
					if (physicsModuleName==\"${valuestringlower}\")
					return new CreatorWithPm<${filename}>(pM);"
					EXT_CONTENTS "${EXT_CONTENTS}")

					file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc "${EXT_CONTENTS}")
				elseif (key STREQUAL "BeginSession")
					message ("${file} is user code for ${key}")

					if (HasBeginSession)
						message (FATAL_ERROR "Attempt to add second BeginSession class to same build.")
					endif()
					set(HasBeginSession 1)

					file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc EXT_CONTENTS)

					string(REGEX REPLACE
					"\#include \"TsParameterManager.hh\""
					"\#include \"TsParameterManager.hh\"
					\#include \"${filename}.hh\""
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"BeginSession\\(TsParameterManager\\*\\)"
					"BeginSession(TsParameterManager* pM)"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"Insertion point for BeginSession"
					"Insertion point for BeginSession
					std::unique_ptr<${filename}> beginSessionPtr(new ${filename}(pM));"
					EXT_CONTENTS "${EXT_CONTENTS}")

					file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc "${EXT_CONTENTS}")
				elseif (key STREQUAL "BeginRun")
					message ("${file} is user code for ${key}")

					if (HasBeginRun)
						message (FATAL_ERROR "Attempt to add second BeginRun class to same build.")
					endif()
					set(HasBeginRun 1)

					file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc EXT_CONTENTS)

					string(REGEX REPLACE
					"\#include \"TsParameterManager.hh\""
					"\#include \"TsParameterManager.hh\"
					\#include \"${filename}.hh\""
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"BeginRun\\(TsParameterManager\\*\\)"
					"BeginRun(TsParameterManager* pM)"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"Insertion point for BeginRun"
					"Insertion point for BeginRun
					std::unique_ptr<${filename}> beginRunPtr(new ${filename}(pM));"
					EXT_CONTENTS "${EXT_CONTENTS}")

					file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc "${EXT_CONTENTS}")
				elseif (key STREQUAL "BeginHistory")
					message ("${file} is user code for ${key}")

					if (HasBeginHistory)
						message (FATAL_ERROR "Attempt to add second BeginHistory class to same build.")
					endif()
					set(HasBeginHistory 1)

					file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc EXT_CONTENTS)

					string(REGEX REPLACE
					"\#include \"TsParameterManager.hh\""
					"\#include \"TsParameterManager.hh\"
					\#include \"${filename}.hh\""
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"BeginHistory\\(TsParameterManager\\*, const G4Run\\*, const G4Event\\*"
					"BeginHistory(TsParameterManager* pM, const G4Run* run, const G4Event* event"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"Insertion point for BeginHistory"
					"Insertion point for BeginHistory
					std::unique_ptr<${filename}> beginHistoryPtr(new ${filename}(pM, run, event));"
					EXT_CONTENTS "${EXT_CONTENTS}")

					file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc "${EXT_CONTENTS}")
				elseif (key STREQUAL "EndHistory")
					message ("${file} is user code for ${key}")

					if (HasEndHistory)
						message (FATAL_ERROR "Attempt to add second EndHistory class to same build.")
					endif()
					set(HasEndHistory 1)

					file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc EXT_CONTENTS)

					string(REGEX REPLACE
					"\#include \"TsParameterManager.hh\""
					"\#include \"TsParameterManager.hh\"
					\#include \"${filename}.hh\""
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"EndHistory\\(TsParameterManager\\*, const G4Run\\*, const G4Event\\*"
					"EndHistory(TsParameterManager* pM, const G4Run* run, const G4Event* event"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"Insertion point for EndHistory"
					"Insertion point for EndHistory
					std::unique_ptr<${filename}> endHistoryPtr(new ${filename}(pM, run, event));"
					EXT_CONTENTS "${EXT_CONTENTS}")

					file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc "${EXT_CONTENTS}")
				elseif (key STREQUAL "EndRun")
					message ("${file} is user code for ${key}")

					if (HasEndRun)
						message (FATAL_ERROR "Attempt to add second EndRun class to same build.")
					endif()
					set(HasEndRun 1)

					file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc EXT_CONTENTS)

					string(REGEX REPLACE
					"\#include \"TsParameterManager.hh\""
					"\#include \"TsParameterManager.hh\"
					\#include \"${filename}.hh\""
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"EndRun\\(TsParameterManager\\*\\)"
					"EndRun(TsParameterManager* pM)"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"Insertion point for EndRun"
					"Insertion point for EndRun
					std::unique_ptr<${filename}> endRunPtr(new ${filename}(pM));"
					EXT_CONTENTS "${EXT_CONTENTS}")

					file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc "${EXT_CONTENTS}")
				elseif (key STREQUAL "EndRun2")
					message ("${file} is user code for ${key}")

					if (HasEndRun2)
						message (FATAL_ERROR "Attempt to add second EndRun2 class to same build.")
					endif()
					set(HasEndRun2 1)

					file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc EXT_CONTENTS)

					string(REGEX REPLACE
					"\#include \"TsParameterManager.hh\""
					"\#include \"TsParameterManager.hh\"
					\#include \"${filename}.hh\""
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"EndRun\\(TsParameterManager\\*\\)"
					"EndRun(TsParameterManager* pM)"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"Insertion point for SecondEndRun"
					"Insertion point for SecondEndRun
					std::unique_ptr<${filename}> secondEndRunPtr(new ${filename}(pM));"
					EXT_CONTENTS "${EXT_CONTENTS}")

					file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc "${EXT_CONTENTS}")
				elseif (key STREQUAL "EndRun3")
					message ("${file} is user code for ${key}")

					if (HasEndRun3)
						message (FATAL_ERROR "Attempt to add second EndRun3 class to same build.")
					endif()
					set(HasEndRun3 1)

					file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc EXT_CONTENTS)

					string(REGEX REPLACE
					"\#include \"TsParameterManager.hh\""
					"\#include \"TsParameterManager.hh\"
					\#include \"${filename}.hh\""
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"EndRun\\(TsParameterManager\\*\\)"
					"EndRun(TsParameterManager* pM)"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"Insertion point for ThirdEndRun"
					"Insertion point for ThirdEndRun
					std::unique_ptr<${filename}> thirdEndRunPtr(new ${filename}(pM));"
					EXT_CONTENTS "${EXT_CONTENTS}")

					file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc "${EXT_CONTENTS}")
				elseif (key STREQUAL "EndSession")
					message ("${file} is user code for ${key}")

					if (HasEndSession)
						message (FATAL_ERROR "Attempt to add second EndSession class to same build.")
					endif()
					set(HasEndSession 1)

					file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc EXT_CONTENTS)

					string(REGEX REPLACE
					"\#include \"TsParameterManager.hh\""
					"\#include \"TsParameterManager.hh\"
					\#include \"${filename}.hh\""
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"EndSession\\(TsParameterManager\\*\\)"
					"EndSession(TsParameterManager* pM)"
					EXT_CONTENTS "${EXT_CONTENTS}")

					string(REGEX REPLACE
					"Insertion point for EndSession"
					"Insertion point for EndSession
					std::unique_ptr<${filename}> endSessionPtr(new ${filename}(pM));"
					EXT_CONTENTS "${EXT_CONTENTS}")

					file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/TsExtensionManager.cc "${EXT_CONTENTS}")
				elseif (NOT key STREQUAL "Extra Class")
					message ("\nInvalid first line in file: ${filename}.cc")
					message ("All cc files in the extensions directory must start with a special")
					message ("comment line that describes the function of this class.")
					message ("See TOPAS User Guide for details.\n")
					message (FATAL_ERROR)
				endif()

				configure_file (${file} "${CMAKE_CURRENT_BINARY_DIR}/extensions/${filename}.cc")

				file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/CMakeLists.txt EXT_CONTENTS)
				string(REGEX REPLACE
				"TsExtensionManager.hh"
				"TsExtensionManager.hh
				${filename}.cc"
				EXT_CONTENTS "${EXT_CONTENTS}")
				file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/CMakeLists.txt "${EXT_CONTENTS}")
			endforeach (file)

			file(GLOB_RECURSE extheaders "${ONE_EXTENSIONS_DIR}/*.hh")
			SET(FILENAME_LIST "")
			foreach (header ${extheaders})
				string(FIND ${header} "/" slashposition REVERSE)
				string(SUBSTRING ${header} ${slashposition} -1 headername)
				string(FIND ${headername} "." dotposition)
				math(EXPR dotposition "${dotposition}-1")
				string(SUBSTRING ${headername} 1 ${dotposition} headername)

				list(FIND FILENAME_LIST ${headername} is_in_list)
				if (NOT ${is_in_list} STREQUAL "-1")
					message (FATAL_ERROR "Attempt to add same extension file twice: ${headername}.hh")
				endif()
				SET(FILENAME_LIST ${FILENAME_LIST} ${headername})

				configure_file (${header} "${CMAKE_CURRENT_BINARY_DIR}/extensions/${headername}.hh")

				file(READ ${CMAKE_CURRENT_BINARY_DIR}/extensions/CMakeLists.txt EXT_CONTENTS)
				string(REGEX REPLACE
				"TsExtensionManager.hh"
				"TsExtensionManager.hh
				${headername}.hh"
				EXT_CONTENTS "${EXT_CONTENTS}")
				file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extensions/CMakeLists.txt "${EXT_CONTENTS}")
			endforeach (header)
		endif()
    endforeach(onetopdir)
endif()

file (GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/examples/Basic/*.txt")
install (FILES ${files} DESTINATION examples/Basic)

file(GLOB files
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/Patient/*.txt"
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/Patient/*.bin"
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/Patient/*.log"
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/Patient/*.dat"
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/Patient/*.zip"
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/Patient/*.tar.bz2")
install (FILES ${files} DESTINATION examples/Patient)

file (GLOB files
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/Brachytherapy/*.txt"
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/Brachytherapy/*.dat"
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/Brachytherapy/*.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/Brachytherapy/*.pdf")
install (FILES ${files} DESTINATION examples/Brachytherapy)

file (GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/examples/Brachytherapy/HDR/*.txt")
install (FILES ${files} DESTINATION examples/Brachytherapy/HDR)

file (GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/examples/Brachytherapy/LDR/*.txt")
install (FILES ${files} DESTINATION examples/Brachytherapy/LDR)

file (GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/examples/Graphics/*.txt")
install (FILES ${files} DESTINATION examples/Graphics)

file (GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/examples/MVLinac/*.txt")
install (FILES ${files} DESTINATION examples/MVLinac)

file (GLOB files
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/Nozzle/*.txt"
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/Nozzle/*.ap"
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/Nozzle/*.rc")
install (FILES ${files} DESTINATION examples/Nozzle)

file (GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/examples/Optical/*.txt")
install (FILES ${files} DESTINATION examples/Optical)

file (GLOB files
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/Outcome/*.txt"
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/Outcome/*.bin"
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/Outcome/*.binheader")
install (FILES ${files} DESTINATION examples/Outcome)

file (GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/examples/PhaseSpace/*.txt")
install (FILES ${files} DESTINATION examples/PhaseSpace)

file (GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/examples/Scoring/*.txt")
install (FILES ${files} DESTINATION examples/Scoring)

file (GLOB files
	"${CMAKE_CURRENT_SOURCE_DIR}/examples/SpecialComponents/*.txt"
	"${CMAKE_CURRENT_SOURCE_DIR}/examples/SpecialComponents/*.ply"
	"${CMAKE_CURRENT_SOURCE_DIR}/examples/SpecialComponents/*.stl"
	"${CMAKE_CURRENT_SOURCE_DIR}/examples/SpecialComponents/*.TABLE")
install (FILES ${files} DESTINATION examples/SpecialComponents)

file (GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/examples/TimeFeature/*.txt")
install (FILES ${files} DESTINATION examples/TimeFeature)

file (GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/examples/UCSFETF/*.txt")
install (FILES ${files} DESTINATION examples/UCSFETF)

file (GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/examples/VarianceReduction/*.txt")
install (FILES ${files} DESTINATION examples/VarianceReduction)
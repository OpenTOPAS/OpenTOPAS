#
# External libraries necessary for every build

if (TOPAS_WITH_STATIC_GEANT4)
	file (GLOB geant4libs
		"${Geant4_DIR}/../*.a")
else ()
	file (GLOB geant4libs 
		"${Geant4_DIR}/../*.so*" 
		"${Geant4_DIR}/../*.dylib"
		"${Geant4_DIR}/../*.lib")
endif ()

file (GLOB gdcmlibs 
	"${GDCM_LIBRARY_DIRS}/*.so*" 
	"${GDCM_LIBRARY_DIRS}/*.a" 
	"${GDCM_LIBRARY_DIRS}/*.dylib"
	"${GDCM_LIBRARY_DIRS}/*.lib")
file (GLOB excludegdcmlibs 
	"${GDCM_LIBRARY_DIRS}/libgdcmMEXD.*" 
	"${GDCM_LIBRARY_DIRS}/libsocketxx.*")
if (excludegdcmlibs)
	list (REMOVE_ITEM gdcmlibs ${excludegdcmlibs})
endif ()

if (APPLE)
	file (GLOB expat "/usr/lib/libexpat.1.dylib")
	install (FILES
		${geant4libs}
		${gdcmlibs}
		${expat}
		DESTINATION Frameworks)
else ()
    install (FILES
        ${geant4libs}
        ${gdcmlibs}
        DESTINATION lib)
endif ()
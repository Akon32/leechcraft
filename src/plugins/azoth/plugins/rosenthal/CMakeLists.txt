CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_azoth_rosenthal)
INCLUDE (InitLCPlugin OPTIONAL)

INCLUDE (${QT_USE_FILE})

IF (MSVC)
	ADD_DEFINITIONS (-DWIN32 -DHUNSPELL_STATIC)
ENDIF (MSVC)
INCLUDE_DIRECTORIES (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
SET (ROSENTHAL_SRCS
	rosenthal.cpp
	highlighter.cpp
	xmlsettingsmanager.cpp
	)
SET (ROSENTHAL_HEADERS
	rosenthal.h
	highlighter.h
	xmlsettingsmanager.h
	)
SET (ROSENTHAL_RESOURCES azothrosenthalresources.qrc)
CreateTrVars ("azoth_rosenthal" "en;es;ru_RU;uk_UA" ROSENTHAL_TRANSLATIONS ROSENTHAL_COMPILED_TRANSLATIONS)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (ROSENTHAL_MOC_SRCS ${ROSENTHAL_HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_ADD_RESOURCES (ROSENTHAL_RCCS ${ROSENTHAL_RESOURCES})

FILE (GLOB R_HUNSPELL_SRCS "hunspell/*.hxx" "hunspell/*.cxx")

ADD_CUSTOM_COMMAND (OUTPUT
	${ROSENTHAL_COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${ROSENTHAL_TRANSLATIONS}
	DEPENDS ${ROSENTHAL_TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_azoth_rosenthal SHARED
    ${ROSENTHAL_SRCS}
    ${ROSENTHAL_MOC_SRCS}
	${ROSENTHAL_COMPILED_TRANSLATIONS}
	${ROSENTHAL_RCCS}
	${R_HUNSPELL_SRCS}
    )
TARGET_LINK_LIBRARIES (leechcraft_azoth_rosenthal
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_azoth_rosenthal DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES azothrosenthalsettings.xml DESTINATION ${LC_SETTINGS_DEST})
INSTALL (FILES ${ROSENTHAL_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})

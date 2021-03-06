CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_azoth_p100q)
INCLUDE (InitLCPlugin OPTIONAL)

SET (QT_USE_QTWEBKIT TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
SET (P100Q_SRCS
	xmlsettingsmanager.cpp
	p100q.cpp
	)
SET (P100Q_HEADERS
	xmlsettingsmanager.h
	p100q.h
	)
SET (P100Q_RESOURCES azothp100qresources.qrc)

CreateTrVars ("azoth_p100q" "en;es;ru_RU;uk_UA" P100Q_TRANSLATIONS P100Q_COMPILED_TRANSLATIONS)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (P100Q_MOC_SRCS ${P100Q_HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_ADD_RESOURCES (P100Q_RCCS ${P100Q_RESOURCES})

ADD_CUSTOM_COMMAND (OUTPUT
	${P100Q_COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${P100Q_TRANSLATIONS}
	DEPENDS ${P100Q_TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_azoth_p100q SHARED
	${P100Q_SRCS}
	${P100Q_MOC_SRCS}
	${P100Q_RCCS}
	${P100Q_COMPILED_TRANSLATIONS}
	)
TARGET_LINK_LIBRARIES (leechcraft_azoth_p100q
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_azoth_p100q DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES azothp100qsettings.xml DESTINATION ${LC_SETTINGS_DEST})
INSTALL (FILES ${P100Q_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})

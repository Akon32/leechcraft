CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_lastfmscrobble)
INCLUDE (InitLCPlugin OPTIONAL)

SET (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

FIND_PACKAGE (LastFM REQUIRED)

SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTXML TRUE)
INCLUDE (${QT_USE_FILE})

INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LastFM_INCLUDE_DIR}
	)

SET (SRCS
	lastfmsubmitter.cpp
	xmlsettingsmanager.cpp
	lastfmscrobble.cpp
	pendingsimilarartists.cpp
	)
SET (HEADERS
	lastfmsubmitter.h
	xmlsettingsmanager.h
	lastfmscrobble.h
	pendingsimilarartists.h
	)
SET (RESOURCES lastfmscrobbleresources.qrc)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_LIBRARY (leechcraft_lastfmscrobble SHARED
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_lastfmscrobble
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${LastFM_LIBRARIES}
	)

INSTALL (TARGETS leechcraft_lastfmscrobble DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES lastfmscrobblesettings.xml DESTINATION ${LC_SETTINGS_DEST})

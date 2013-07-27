PROJECT(${QTSINGLEAPPLICATION_LIBRARY})
  
IF(WIN32)
    SET(PLATFORM win)
ELSE(WIN32)
    SET(PLATFORM unix)
ENDIF(WIN32)

SET(SOURCES qtlocalpeer.cpp
    # qtlockedfile_${PLATFORM}.cpp
    qtlockedfile.cpp
    qtsingleapplication.cpp
    qtsinglecoreapplication.cpp
)

SET(HEADERS qtlocalpeer.h
    qtlockedfile.h
    qtsingleapplication.h
    qtsinglecoreapplication.h
)

ADD_LIBRARY(${PROJECT_NAME} ${SOURCES} ${HEADERS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_LIBRARIES})

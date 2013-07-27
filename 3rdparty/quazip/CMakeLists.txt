PROJECT(${QUAZIP_LIBRARY})

SET(SOURCES JlCompress.cpp 
    qioapi.cpp 
    quaadler32.cpp 
    quacrc32.cpp 
    quagzipfile.cpp 
    quaziodevice.cpp 
    quazip.cpp 
    quazipdir.cpp 
    quazipfile.cpp 
    quazipnewinfo.cpp 
    unzip.c 
    zip.c
  )
  
  SET(HEADERS crypt.h 
    ioapi.h 
    JlCompress.h 
    quaadler32.h 
    quachecksum32.h 
    quacrc32.h 
    quagzipfile.h 
    quaziodevice.h 
    quazipdir.h 
    quazipfile.h 
    quazipfileinfo.h 
    quazip_global.h 
    quazip.h 
    quazipnewinfo.h
  )
  
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_LIBRARIES})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/libs)

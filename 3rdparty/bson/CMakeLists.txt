PROJECT(${BSON_LIBRARY})

SET(SOURCES bcon.c 
    bson.c 
    encoding.c 
    env.c 
    gridfs.c
    mongo.c
    md5.c 
    numbers.c
)

ADD_LIBRARY(${PROJECT_NAME} ${SOURCES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME})

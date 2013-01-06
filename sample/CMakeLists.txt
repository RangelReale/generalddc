if (APPLE)
find_library(IOKIT_LIBRARY IOKit)
find_library(COREFOUNDATION_LIBRARY CoreFoundation)
find_library(APPLICATIONSERVICES_LIBRARY ApplicationServices)
endif()

SET(SRCS
        main.cpp
)
SET(INCS
)

ADD_EXECUTABLE( generalddc_sample ${SRCS} ${INCS} )

TARGET_LINK_LIBRARIES ( generalddc_sample generalddc )

if (WIN32)
	TARGET_LINK_LIBRARIES ( generalddc_sample Dxva2 )
endif()
if (APPLE)
	TARGET_LINK_LIBRARIES ( generalddc_sample ${IOKIT_LIBRARY} ${COREFOUNDATION_LIBRARY} ${APPLICATIONSERVICES_LIBRARY} )
endif()



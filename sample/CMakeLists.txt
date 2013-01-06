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

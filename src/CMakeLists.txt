SET(SRCS
	Device.cpp
	List.cpp
)

SET(INCS
	../include/GeneralDDC/Types.h
	../include/GeneralDDC/Device.h
	../include/GeneralDDC/DeviceImpl.h
	../include/GeneralDDC/List.h
	../include/GeneralDDC/Exceptions.h
)

if (WIN32)
	add_definitions(-DGENERALDDC_IMPL_WIN32)
	LIST(APPEND SRCS DeviceImpl_WIN32.cpp)
	LIST(APPEND INCS ../include/GeneralDDC/DeviceImpl_WIN32.h)
elseif(APPLE)
	add_definitions(-DGENERALDDC_IMPL_APPLE)
	LIST(APPEND SRCS DeviceImpl_APPLE.cpp)
	LIST(APPEND INCS ../include/GeneralDDC/DeviceImpl_APPLE.h)
elseif ("${CMAKE_SYSTEM}" MATCHES "Linux")
	add_definitions(-DGENERALDDC_IMPL_LINUX)
	LIST(APPEND SRCS DeviceImpl_LINUX.cpp)
	LIST(APPEND INCS ../include/GeneralDDC/DeviceImpl_LINUX.h)
else()
	message( FATAL_ERROR "No platform implementation available" )
endif()

ADD_LIBRARY(generalddc STATIC ${SRCS} ${INCS} )

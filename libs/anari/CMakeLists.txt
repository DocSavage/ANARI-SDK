## Copyright 2021 The Khronos Group
## SPDX-License-Identifier: Apache-2.0

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

if(BUILD_TESTING)
  set(GLM_SOURCES anari_cpp_glm_defs.cpp)
  set(GLM_LIBS glm)
endif()

add_library(${PROJECT_NAME}
  anari_cpp_std_defs.cpp
  API.cpp
  Device.cpp
  Helpers.cpp
  Library.cpp
  ${GLM_SOURCES}
)

if (ANARI_USE_GENERATED_HEADERS)
  add_dependencies(${PROJECT_NAME} GeneratedHeaders)
  set(GENERATED_HEADER_DIR ${CMAKE_BINARY_DIR}/include)
endif()

if (NOT BUILD_SHARED_LIBS)
  target_compile_definitions(${PROJECT_NAME} PUBLIC -DANARI_STATIC_DEFINE)
endif()

target_include_directories(${PROJECT_NAME}
PUBLIC
  $<BUILD_INTERFACE:${GENERATED_HEADER_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
  ${CMAKE_DL_LIBS}
PRIVATE
  Threads::Threads
  ${GLM_LIBS}
)

## Install library + targets

set_target_properties(${PROJECT_NAME}
    PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

install(TARGETS anari
  EXPORT anari_Exports
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    NAMELINK_SKIP
  # on Windows put the dlls into bin
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  # ... and the import lib into the devel package
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT anari_Exports
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/anari-${PROJECT_VERSION}
  NAMESPACE anari::
)

install(TARGETS anari
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    NAMELINK_ONLY
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

## Install headers

if (ANARI_USE_GENERATED_HEADERS)
  install(DIRECTORY ${CMAKE_BINARY_DIR}/include/anari
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
      PATTERN *.h
  )

  install(DIRECTORY ${CMAKE_BINARY_DIR}/include/anari_util
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/anari
    FILES_MATCHING
      PATTERN *.h
  )
else()
  install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/anari
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
endif()

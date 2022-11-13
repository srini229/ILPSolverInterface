FetchContent_Declare(
  symphony
  URL https://www.coin-or.org/download/source/SYMPHONY/SYMPHONY-5.6.17.tgz
  URL_HASH MD5=59fe0fa58c8bef019967766c86247b9d
)
include(ProcessorCount)
ProcessorCount(N)
if (NOT N)
  set(N 1)
endif()
FetchContent_GetProperties(symphony)
if(NOT symphony_POPULATED)
  FetchContent_Populate(symphony)
  message(STATUS "Building SYMPHONY library from source.")
  include(ExternalProject)
  ExternalProject_Add(symphony
      SOURCE_DIR ${symphony_SOURCE_DIR}
      CONFIGURE_COMMAND ${symphony_SOURCE_DIR}/configure --disable-openmp --disable-zlib --disable-bzlib --without-blas --without-lapack --enable-static --disable-shared --with-pic --prefix=${symphony_BINARY_DIR}
      BUILD_BYPRODUCTS ${symphony_BINARY_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}Sym${CMAKE_STATIC_LIBRARY_SUFFIX}
      ${symphony_BINARY_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}OsiSym${CMAKE_STATIC_LIBRARY_SUFFIX}
      BUILD_COMMAND make -j${N}
      BUILD_IN_SOURCE 1
      )
  set(symphony_LIBRARIES
    ${symphony_BINARY_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}Sym${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${symphony_BINARY_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}OsiSym${CMAKE_STATIC_LIBRARY_SUFFIX}
    )
endif()

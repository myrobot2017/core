# Options for building CORE.

# Build shared libraries by default.
option(CORE_SHARED_LIBS "Build shared libraries." ON)
if(CORE_SHARED_LIBS)
  set(CORE_LIB_PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX})
  set(CORE_LIB_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
  set(CORE_LIB_TYPE "SHARED")
  #set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_SHARED_LIBRARY_SUFFIX})
  if(WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_IMPORT_LIBRARY_SUFFIX})
  endif(WIN32)
else(CORE_SHARED_LIBS)
  set(CORE_LIB_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
  set(CORE_LIB_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(CORE_LIB_TYPE "STATIC")
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})
endif(CORE_SHARED_LIBS)
mark_as_advanced(CORE_SHARED_LIBS)
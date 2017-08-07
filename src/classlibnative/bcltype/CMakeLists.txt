set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(PerfCountersSupportedBuild)
    add_definitions(-DENABLE_PERF_COUNTERS)
endif(PerfCountersSupportedBuild)

set(BCLTYPE_SOURCES
    arraynative.cpp
    arrayhelpers.cpp
    bignum.cpp
    currency.cpp
    decimal.cpp
    diyfp.cpp
    grisu3.cpp
    windowsruntimebufferhelper.cpp
    number.cpp
    oavariant.cpp
    objectnative.cpp
    stringnative.cpp
    stringbuffer.cpp
    system.cpp
    varargsnative.cpp
    variant.cpp
)

# oleaut is used for decimal arithmethics on windows, except 64bit where our implementation is faster
if(CLR_CMAKE_TARGET_ARCH_AMD64 OR NOT DEFINED WIN32)
    list(APPEND BCLTYPE_SOURCES
     decimal_calc.cpp)
endif(CLR_CMAKE_TARGET_ARCH_AMD64 OR NOT DEFINED WIN32)

if(CLR_CMAKE_TARGET_ARCH_AMD64 AND WIN32)
    list(APPEND BCLTYPE_SOURCES
     udiv128.S)
endif(CLR_CMAKE_TARGET_ARCH_AMD64 AND WIN32)

if(CLR_CMAKE_PLATFORM_UNIX)
    add_compile_options(-fPIC)
endif(CLR_CMAKE_PLATFORM_UNIX)

add_library_clr(bcltype
    STATIC
    ${BCLTYPE_SOURCES}
)


find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

set_source_files_properties(../bluezero.i PROPERTIES CPLUSPLUS ON)

swig_add_library(
    bluezero-csharp
    LANGUAGE csharp
    SOURCES ../bluezero.i
)
swig_link_libraries(bluezero-csharp b0)


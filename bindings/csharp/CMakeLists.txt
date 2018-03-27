find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

set_source_files_properties(../bluezero.i PROPERTIES CPLUSPLUS ON)

swig_add_library(
    bluezero-csharp
    LANGUAGE csharp
    SOURCES ../bluezero.i
)
swig_link_libraries(bluezero-csharp b0)

# cmake doesn't support C# in generators other than VisualStudio
# to build C# code run: csc *.cs

configure_file(test_bluezero.cs test_bluezero.cs COPYONLY)

#enable_language(CSharp)
#add_executable(test_bluezero
#    test_bluezero.cs
#)
#target_link_libraries(test_bluezero
#    bluezero-csharp
#)

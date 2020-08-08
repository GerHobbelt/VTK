# Allow the user to customize their build with some local options
include("${CMAKE_CURRENT_SOURCE_DIR}/LocalUserOptions.cmake" OPTIONAL)

set(ns "VTK")
if (VTK_BUILD_COMPILE_TOOLS_ONLY)
  set(ns "VTKCompileTools")
endif ()

set(sources
  vtkParse.tab.c
  vtkParseData.c
  vtkParseExtras.c
  vtkParseHierarchy.c
  vtkParseMain.c
  vtkParseMangle.c
  vtkParseMerge.c
  vtkParsePreprocess.c
  vtkParseString.c
  vtkWrap.c
  vtkWrapText.c)

set(headers
  vtkParse.h
  vtkParseData.h
  vtkParseExtras.h
  vtkParseHierarchy.h
  vtkParseMain.h
  vtkParseMangle.h
  vtkParseMerge.h
  vtkParsePreprocess.h
  vtkParseString.h
  vtkParseType.h
  vtkWrap.h)

set_property(SOURCE vtkParseMain.c
  PROPERTY
    COMPILE_DEFINITIONS "VTK_PARSE_VERSION=\"${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}\"")

vtk_module_add_module("${ns}::WrappingTools"
  SOURCES ${sources}
  HEADERS ${headers})

if (NOT TARGET "${ns}::WrapHierarchy")
  vtk_module_add_executable(WrapHierarchy
    DEVELOPMENT
    BASENAME vtkWrapHierarchy
    vtkWrapHierarchy.c)
  target_link_libraries(WrapHierarchy
    PRIVATE
      "${ns}::WrappingTools")
  add_executable("${ns}::WrapHierarchy" ALIAS
    WrapHierarchy)
endif ()

if (NOT TARGET WrapPython)

  # Additional prefixes to match for methods
  set(VTK_PREFIX_STRING_CHECK_ADDITIONAL "PREFIX_STRING_CHECK_ADDITIONAL(ss) || (0==0)" CACHE STRING 
          "Additonal pre-procesor logic to allow wrapping of names not begining with vtk. See Wrapping/Tools/vtkPrefixCheck.in for details")
  set(VTK_PREFIX_STRING_CHECK_ADDITIONAL_UC "PREFIX_STRING_CHECK_ADDITIONAL_UC(ss) || (0==0)" CACHE STRING 
          "Additonal pre-procesor logic to allow wrapping of names not begining with vtk. See Wrapping/Tools/vtkPrefixCheck.in for details")
  set(VTK_PREFIX_STRING_CHECK_ADDITIONAL_NS "PREFIX_STRING_CHECK_ADDITIONAL_NS(ss) || (0==0)" CACHE STRING 
          "Additonal pre-procesor logic to allow wrapping of names not begining with vtk. See Wrapping/Tools/vtkPrefixCheck.in for details")

  # Create the vtkPrefixCheck.h
  configure_file(vtkPrefixCheck.h.in vtkPrefixCheck.h @ONLY)

  vtk_module_add_executable(WrapPython
    DEVELOPMENT
    BASENAME vtkWrapPython
    vtkWrapPython.c
    vtkWrapPythonClass.c
    vtkWrapPythonConstant.c
    vtkWrapPythonEnum.c
    vtkWrapPythonMethod.c
    vtkWrapPythonMethodDef.c
    vtkWrapPythonNamespace.c
    vtkWrapPythonOverload.c
    vtkWrapPythonTemplate.c
    vtkWrapPythonType.c)
  target_link_libraries(WrapPython
    PRIVATE
      "${ns}::WrappingTools")
  add_executable("${ns}::WrapPython" ALIAS
    WrapPython)

  vtk_module_add_executable(WrapPythonInit
    DEVELOPMENT
    BASENAME vtkWrapPythonInit
    vtkWrapPythonInit.c)
  add_executable("${ns}::WrapPythonInit" ALIAS
    WrapPythonInit)
endif ()

if (NOT TARGET WrapJava)
  vtk_module_add_executable(ParseJava
    DEVELOPMENT
    BASENAME vtkParseJava
    vtkParseJava.c)
  target_link_libraries(ParseJava
    PRIVATE
      "${ns}::WrappingTools")
  add_executable("${ns}::ParseJava" ALIAS
    ParseJava)

  vtk_module_add_executable(WrapJava
    DEVELOPMENT
    BASENAME vtkWrapJava
    vtkWrapJava.c)
  target_link_libraries(WrapJava
    PRIVATE
      "${ns}::WrappingTools")
  add_executable("${ns}::WrapJava" ALIAS
    WrapJava)
endif ()

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  cmake_minimum_required(VERSION 3.16)
  project(banana_editor LANGUAGES CXX)
  set(BANANA_EDITOR_IS_TOP_LEVEL TRUE)
else()
  set(BANANA_EDITOR_IS_TOP_LEVEL FALSE)
endif()

if(NOT TARGET ROOT::Core)
  find_package(ROOT REQUIRED COMPONENTS Core RIO Hist Tree Net Gui Graf Rint)
endif()
if(DEFINED ROOT_USE_FILE)
  include("${ROOT_USE_FILE}")
endif()
if(NOT COMMAND ROOT_GENERATE_DICTIONARY)
  include("${ROOT_DIR}/modules/RootNewMacros.cmake")
endif()

set(BANANA_EDITOR_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(BANANA_EDITOR_EXAMPLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/example_editors")
set(BANANA_EDITOR_EXAMPLE_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/example_data" CACHE PATH "Path to the example ROOT data directory")

option(BANANA_EDITOR_BUILD_EXAMPLES "Build all bundled banana editor example executables" ${BANANA_EDITOR_IS_TOP_LEVEL})
option(BANANA_EDITOR_BUILD_KRATTA_EDITOR "Build the bundled KRATTA banana editor executable" ${BANANA_EDITOR_BUILD_EXAMPLES})
option(BANANA_EDITOR_BUILD_OSCAR_EDITOR "Build the bundled OSCAR banana editor executable" ${BANANA_EDITOR_BUILD_EXAMPLES})

# banana editor library
ROOT_GENERATE_DICTIONARY(G__TBananaEditor
  ${BANANA_EDITOR_SRC_DIR}/TBananaEditor.hh
  LINKDEF ${BANANA_EDITOR_SRC_DIR}/TBananaEditorLinkDef.h
)

add_library(TBananaEditor SHARED
  ${BANANA_EDITOR_SRC_DIR}/TBananaEditor.cc
  ${BANANA_EDITOR_SRC_DIR}/TBananaEditor.hh
  G__TBananaEditor.cxx
)
target_link_libraries(TBananaEditor PUBLIC ROOT::Core ROOT::RIO ROOT::Hist ROOT::Tree ROOT::Net ROOT::Gui ROOT::Graf)
target_include_directories(TBananaEditor
  PUBLIC ${BANANA_EDITOR_SRC_DIR}
  PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
)

function(add_banana_editor_executable target)
  if(ARGC LESS 2)
    message(FATAL_ERROR "add_banana_editor_executable requires a target name and at least one source file")
  endif()

  add_executable(${target} ${ARGN})
  target_link_libraries(${target} TBananaEditor ROOT::Core ROOT::RIO ROOT::Hist ROOT::Tree ROOT::Net ROOT::Gui ROOT::Graf ROOT::Rint)
  set_target_properties(${target} PROPERTIES ENABLE_EXPORTS TRUE)
endfunction()

function(add_banana_editor_bundled_example target source)
  add_banana_editor_executable(${target} ${source})
  set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

  add_custom_command(TARGET ${target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink "${BANANA_EDITOR_EXAMPLE_DATA_DIR}" "${CMAKE_BINARY_DIR}/example_data"
    COMMENT "Creating symbolic link to example_data directory"
  )
endfunction()

if(BANANA_EDITOR_BUILD_KRATTA_EDITOR)
  add_banana_editor_bundled_example(kratta_editor ${BANANA_EDITOR_EXAMPLE_DIR}/kratta_editor.cpp)
endif()

if(BANANA_EDITOR_BUILD_OSCAR_EDITOR)
  add_banana_editor_bundled_example(oscar_editor ${BANANA_EDITOR_EXAMPLE_DIR}/oscar_editor.cpp)
endif()

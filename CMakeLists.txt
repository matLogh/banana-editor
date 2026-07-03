if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  cmake_minimum_required(VERSION 3.16)
  project(banana_editor LANGUAGES CXX)
  set(BANANA_EDITOR_IS_TOP_LEVEL TRUE)
else()
  set(BANANA_EDITOR_IS_TOP_LEVEL FALSE)
endif()

if(NOT TARGET ROOT::Core)
  find_package(ROOT REQUIRED COMPONENTS Core RIO Hist Tree Net Gui Rint)
endif()
if(DEFINED ROOT_USE_FILE)
  include("${ROOT_USE_FILE}")
endif()
if(NOT COMMAND ROOT_GENERATE_DICTIONARY)
  include("${ROOT_DIR}/modules/RootNewMacros.cmake")
endif()

option(BANANA_EDITOR_BUILD_EXAMPLES "Build banana editor example executables" ${BANANA_EDITOR_IS_TOP_LEVEL})

set(BANANA_EDITOR_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(BANANA_EDITOR_EXAMPLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/example_editors")
set(BANANA_EDITOR_EXAMPLE_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/example_data" CACHE PATH "Path to the example ROOT data directory")

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
target_link_libraries(TBananaEditor PUBLIC ROOT::Core ROOT::RIO ROOT::Hist ROOT::Tree ROOT::Net ROOT::Gui)
target_include_directories(TBananaEditor
  PUBLIC ${BANANA_EDITOR_SRC_DIR}
  PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
)

if(BANANA_EDITOR_BUILD_EXAMPLES)
  function(add_banana_editor_example target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} TBananaEditor ROOT::Core ROOT::RIO ROOT::Hist ROOT::Tree ROOT::Net ROOT::Gui ROOT::Rint)
    set_target_properties(${target} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
      ENABLE_EXPORTS TRUE
    )

    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E create_symlink "${BANANA_EDITOR_EXAMPLE_DATA_DIR}" "${CMAKE_BINARY_DIR}/example_data"
      COMMENT "Creating symbolic link to example_data directory"
    )
  endfunction()

  add_banana_editor_example(kratta_editor ${BANANA_EDITOR_EXAMPLE_DIR}/kratta_editor.cpp)
  add_banana_editor_example(oscar_editor ${BANANA_EDITOR_EXAMPLE_DIR}/oscar_editor.cpp)
endif()

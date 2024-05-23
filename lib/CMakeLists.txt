# THE LIST OF PLUGINS AND THE CORRESPONDING SOURCE FILES
# ======================================================
set(LLVM_WM_PLUGINS
  Flexo
)

set(Flexo_SOURCES
  Flexo.cpp
  Utils.cpp
)

set(Flexo_SRC_DIRS
  WeirdRegisters
  WeirdRegisters/Mappings
  WeirdMachines
  Circuits
)

set(yosys_path ${CMAKE_CURRENT_SOURCE_DIR}/../yosys)

if (NOT EXISTS ${yosys_path}/kernel/yosys.h)
  set(yosys_path /usr/local/share/yosys/include)
endif()

set(Flexo_INCLUDE
  ${yosys_path}
  /usr/include/tcl8.6
)

link_directories(/usr/local/lib/yosys/)

set(Flexo_LIBS
  yosys
  m
  rt
  readline
  ffi
  dl
  z
  tcl8.6
  tclstub8.6
)

set(Flexo_DEFS
  _YOSYS_
  YOSYS_ENABLE_READLINE
  YOSYS_ENABLE_PLUGINS
  YOSYS_ENABLE_GLOB
  YOSYS_ENABLE_ZLIB
  YOSYS_ENABLE_TCL
  YOSYS_ENABLE_ABC
  YOSYS_ENABLE_COVER
)

# CONFIGURE THE PLUGIN LIBRARIES
# ==============================
foreach( plugin ${LLVM_WM_PLUGINS} )
  # Create a library corresponding to 'plugin'
  add_library(
    ${plugin}
    SHARED
    ${${plugin}_SOURCES}
  )

  # Add subdirectories to 'plugin'
  foreach( dir ${${plugin}_SRC_DIRS} )
    add_subdirectory( ${dir} )
  endforeach()

  # Configure include directories for 'plugin'
  target_include_directories(
    ${plugin}
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
    ${${plugin}_INCLUDE}
  )

  # Add libraries to 'plugin'
  foreach( lib ${${plugin}_LIBS} )
    target_link_libraries(
      ${plugin}
      PRIVATE
      ${lib}
    )
  endforeach()

  foreach( def ${${plugin}_DEFS} )
    target_compile_definitions(
      ${plugin}
      PRIVATE
      ${def}
    )
  endforeach()

  # On Darwin (unlike on Linux), undefined symbols in shared objects are not
  # allowed at the end of the link-edit. The plugins defined here:
  #  - _are_ shared objects
  #  - reference symbols from LLVM shared libraries, i.e. symbols which are
  #    undefined until those shared objects are loaded in memory (and hence
  #    _undefined_ during static linking)
  # The build will fail with errors like this:
  #    "Undefined symbols for architecture x86_64"
  # with various LLVM symbols being undefined. Since those symbols are later
  # loaded and resolved at runtime, these errors are false positives.
  # This behaviour can be modified via the '-undefined' OS X linker flag as
  # follows.
  target_link_libraries(
    ${plugin}
    PRIVATE
    "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>"
  )
endforeach()

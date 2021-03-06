file(GLOB HEADERS "include/*.h")
file(GLOB SOURCES "src/*.cpp")

set(LINK_LIBS
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_PYTHON_LIBRARY}
  ${Boost_SIGNALS_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Log4CPP_LIBRARIES}
  ${PYTHON_LIBRARIES}
  )

if (RAM_WITH_CORE)
  add_library(ram_core SHARED ${SOURCES} ${HEADERS})
  target_link_libraries(ram_core ${LINK_LIBS})
  set_target_properties(ram_core PROPERTIES
    DEFINE_SYMBOL RAM_PKG_CORE
    ARCHIVE_OUTPUT_DIRECTORY "${LIBDIR}"
    LIBRARY_OUTPUT_DIRECTORY "${LIBDIR}"
    RUNTIME_OUTPUT_DIRECTORY "${LIBDIR}"
    )

  test_module(core "ram_core")
  if (RAM_WITH_MATH AND RAM_TESTS)
    target_link_libraries(Tests_core ram_math)
  endif (RAM_WITH_MATH AND RAM_TESTS)
endif (RAM_WITH_CORE)

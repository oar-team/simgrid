foreach(x check_defaults stack_overflow)
  add_executable       (${x}  ${x}/${x}.c)
  target_link_libraries(${x}  simgrid)
  set_target_properties(${x}  PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${x})

  set(teshsuite_src ${teshsuite_src} ${CMAKE_CURRENT_SOURCE_DIR}/${x}/${x}.c)
endforeach()

foreach(x generic_simcalls)
  add_executable       (${x}  ${x}/${x}.cpp)
  target_link_libraries(${x}  simgrid)
  set_target_properties(${x}  PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${x})
  set(teshsuite_src ${teshsuite_src} ${CMAKE_CURRENT_SOURCE_DIR}/${x}/${x}.cpp)
endforeach()

foreach (factory raw thread boost ucontext)
  set(tesh_files    ${tesh_files}    ${CMAKE_CURRENT_SOURCE_DIR}/check_defaults/factory_${factory}.tesh)
endforeach()

set(teshsuite_src  ${teshsuite_src}                                                                        PARENT_SCOPE)
set(tesh_files     ${tesh_files}     
    ${CMAKE_CURRENT_SOURCE_DIR}/stack_overflow/stack_overflow.tesh  
    ${CMAKE_CURRENT_SOURCE_DIR}/generic_simcalls/generic_simcalls.tesh    
    PARENT_SCOPE)

IF(HAVE_RAW_CONTEXTS)
  ADD_TESH(tesh-simix-factory-default --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simix/check_defaults --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simix/check_defaults factory_raw.tesh)
ELSEIF(HAVE_UCONTEXT_CONTEXTS)
  ADD_TESH(tesh-simix-factory-default --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simix/check_defaults --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simix/check_defaults factory_ucontext.tesh)
ELSEIF(HAVE_BOOST_CONTEXTS)
  ADD_TESH(tesh-simix-factory-default --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simix/check_defaults --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simix/check_defaults factory_boost.tesh)
ELSE()
  ADD_TESH(tesh-simix-factory-default --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simix/check_defaults --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simix/check_defaults factory_thread.tesh)
ENDIF()

if (NOT enable_memcheck)
ADD_TESH_FACTORIES(stack-overflow   "thread;ucontext;boost;raw" --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simix/stack_overflow --setenv srcdir=${CMAKE_HOME_DIRECTORY} --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simix/stack_overflow stack_overflow.tesh)
ADD_TESH_FACTORIES(generic-simcalls "thread;ucontext;boost;raw" --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simix/generic_simcalls --setenv srcdir=${CMAKE_HOME_DIRECTORY} --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simix/generic_simcalls generic_simcalls.tesh)
endif()

foreach (factory raw thread boost ucontext)
  string (TOUPPER have_${factory}_contexts VARNAME)
  if (${VARNAME})
    if (release AND (WIN32 OR CMAKE_SYSTEM_NAME MATCHES "Darwin"))
      SET_TESTS_PROPERTIES(stack-overflow-${factory} PROPERTIES WILL_FAIL true)
    endif()
    ADD_TESH(tesh-simix-factory-${factory} --cfg contexts/factory:${factory} --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simix/check_defaults --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simix/check_defaults factory_${factory}.tesh)
  endif()
endforeach()

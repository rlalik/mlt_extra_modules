set(rl_src
    factory.c
    filter_typewriter.c
    typewriter.cpp
    typewriter.h
)
add_library(mltrl MODULE ${rl_src})
target_link_libraries(mltrl
    PRIVATE
        m
    PUBLIC
        mlt Qt::Xml
)
# Create module in parent directory, for the benefit of "source setenv".
set_target_properties(mltrl PROPERTIES LIBRARY_OUTPUT_DIRECTORY ..)
install(TARGETS mltrl LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/mlt)
set(yml
    filter_typewriter.yml
)
install(FILES ${yml} DESTINATION ${CMAKE_INSTALL_DATADIR}/mlt/rl)

# Add all targets to the build-tree export set
export(TARGETS mltrl
  FILE "${PROJECT_BINARY_DIR}/MltExtraModulesTargets.cmake")

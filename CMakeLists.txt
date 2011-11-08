cmake_minimum_required(VERSION 2.6)
project(transaq_terminal)

SET(BOOST_ROOT "/usr/local/")
SET(Boost_NO_SYSTEM_PATHS true)
find_package(Boost 1.47 REQUIRED system thread)

add_executable(transaq_terminal
    transaq_wrapper.cpp
    transaq_loader_stub.cpp
    main.cpp)

target_link_libraries(transaq_terminal ${Boost_LIBRARIES} pthread)
install(TARGETS transaq_terminal DESTINATION ./)


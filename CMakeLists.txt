cmake_minimum_required(VERSION 2.6)
project(trade_terminal)

SET(BOOST_ROOT "/usr/local/")
SET(Boost_NO_SYSTEM_PATHS true)
find_package(Boost 1.47 REQUIRED system thread program_options)

add_executable(trade_terminal
    transaq_client.cpp
    transaq_wrapper.cpp
    transaq_loader_stub.cpp
    main.cpp)

target_link_libraries(trade_terminal ${Boost_LIBRARIES} ssl pthread)
install(TARGETS trade_terminal DESTINATION ./)


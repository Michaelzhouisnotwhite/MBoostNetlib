project(MBoostNetlib-src)

file(GLOB project_srcs CONFIGURE_DEPENDS *.cpp)
add_library(MBoostNetlib-src STATIC ${project_srcs})

target_link_libraries(
  MBoostNetlib-src PRIVATE fmt::fmt Boost::bind Boost::asio Boost::url
                           Boost::beast toy::generic)
if(WIN32)
  target_link_libraries(MBoostNetlib-src PRIVATE wsock32 ws2_32 User32)
endif()
target_include_directories(MBoostNetlib-src INTERFACE ${PROJECT_SOURCE_DIR})
if(UNIX)
  target_link_libraries(MBoostNetlib-src PRIVATE Backward::Interface)
endif()

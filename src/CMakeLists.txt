project(MBoostNetlib-src)

file(GLOB project_srcs *.cpp)
add_library(MBoostNetlib-src STATIC ${project_srcs})

target_link_libraries(
  MBoostNetlib-src PRIVATE fmt::fmt Boost::bind Boost::asio Boost::url
                           Boost::beast toy::generic)
target_include_directories(MBoostNetlib-src INTERFACE ${PROJECT_SOURCE_DIR})
if(UNIX)
  target_link_libraries(MBoostNetlib-src PRIVATE Backward::Interface)
endif()

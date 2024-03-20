file(GLOB single_files_hdrs CONFIGURE_DEPENDS single_*.h)
aux_source_directory(. single_files)
foreach (single_file ${single_files})
    get_filename_component(proj_name ${single_file} NAME_WLE)

    foreach (hdr ${single_files_hdrs})
        if (${hdr} MATCHES ${proj_name})
            list(APPEND ${proj_name}_hdrlist ${hdr})
        endif ()
    endforeach ()

    add_executable(${proj_name} ${single_file} ${${proj_name}_hdrlist} ${PROJECT_RESOURCES})

    target_link_libraries(${proj_name} PRIVATE MBoostNetlib-src fmt::fmt Boost::bind Boost::asio Boost::url Boost::beast toy::generic)

    if (UNIX)
        # target_link_libraries(${proj_name} PRIVATE Backward::Interface)
    endif ()

    message("add single test project: ${proj_name}")
endforeach ()
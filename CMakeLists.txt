cmake_minimum_required(VERSION 3.25)
project(auto-create)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

if (MSVC)
    set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            )
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
endif(MSVC)

include_directories(include)

add_executable(${PROJECT_NAME} main.cpp dir_item.cpp utils.cpp)

# 安装文件
install(TARGETS ${PROJECT_NAME} DESTINATION .)
install(FILES D:/msys64/mingw64/bin/libstdc++-6.dll DESTINATION .)
install(FILES D:/msys64/mingw64/bin/libwinpthread-1.dll DESTINATION .)
install(FILES D:/msys64/mingw64/bin/libgcc_s_seh-1.dll DESTINATION .)

# 添加打包规则

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "发行版")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})

set(CPACK_GENERATOR ZIP)

include(CPack)

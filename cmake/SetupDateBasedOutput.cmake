# SetupDateBasedOutput.cmake
# 按日期创建输出目录并设置全局输出路径
# 作者: CMAKE配置
# 日期: 自动生成

# 获取当前日期 (YYYYMMDD格式)
string(TIMESTAMP CURRENT_DATE "%Y%m%d")

# 构造目标目录名
set(DATED_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/TARGET-${CURRENT_DATE}")

# 创建日期目录（如果不存在）
file(MAKE_DIRECTORY "${DATED_OUTPUT_DIR}")

# 设置全局输出路径变量
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${DATED_OUTPUT_DIR}" CACHE PATH "Runtime output directory" FORCE)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${DATED_OUTPUT_DIR}" CACHE PATH "Library output directory" FORCE)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${DATED_OUTPUT_DIR}" CACHE PATH "Archive output directory" FORCE)

# 确保所有配置类型都使用相同的输出目录
foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)
  string(TOUPPER "${config}" config_upper)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${DATED_OUTPUT_DIR}" CACHE PATH "Runtime output directory for ${config}" FORCE)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${DATED_OUTPUT_DIR}" CACHE PATH "Library output directory for ${config}" FORCE)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${DATED_OUTPUT_DIR}" CACHE PATH "Archive output directory for ${config}" FORCE)
endforeach()

# 导出全局变量供子项目使用
set(DATED_OUTPUT_DIR "${DATED_OUTPUT_DIR}" CACHE INTERNAL "Date-based output directory")

# 调试信息
message(STATUS "Date-based output setup completed:")
message(STATUS "  Current date: ${CURRENT_DATE}")
message(STATUS "  Output directory: ${DATED_OUTPUT_DIR}")
message(STATUS "  All targets will be built to: ONVIF-${CURRENT_DATE}/")

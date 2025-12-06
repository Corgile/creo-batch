# Check DLL dependencies after build
ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND echo =================== 检查 ${PROJECT_NAME} 的依赖 ===================
    COMMAND Dependencies -imports $<TARGET_FILE:${PROJECT_NAME}> | findstr "\\.dll"
    COMMENT "检查依赖的 DLL"
    COMMAND echo =================== 完成 ===================
    VERBATIM
)

INSTALL(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
INSTALL(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}>
    EXPORT ${PROJECT_NAME}
    DESTINATION pdb
    OPTIONAL
)

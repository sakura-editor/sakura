cmake_minimum_required(VERSION 3.0.2)

get_target_property(HeaderMake_RUNTIME_OUTPUT_DIRECTORY HeaderMake RUNTIME_OUTPUT_DIRECTORY)
set(sakura_project_dir         ${CMAKE_CURRENT_SOURCE_DIR}/../sakura_core)
set(update_HeaderMake_BAT      ${CMAKE_CURRENT_SOURCE_DIR}/funccode.bat)
set(update_HeaderMake_DIR_EXE  ${HeaderMake_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR})
set(update_HeaderMake_DIR_DAT  ${sakura_project_dir})

set(update_gitHash_BAT         ${CMAKE_CURRENT_SOURCE_DIR}/githash.bat)
set(update_gitHash_DIR_DAT     ${sakura_project_dir})

add_custom_target(OutHeaderMake ${update_HeaderMake_BAT} ${update_HeaderMake_DIR_EXE} ${update_HeaderMake_DIR_DAT})
add_custom_target(OutGitHash    ${update_gitHash_BAT}    ${update_gitHash_DIR_DAT})

# includeフォルダーが空でも警告が出ないモードを指定。
set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)

set(SOURCE_PATH ${CURRENT_BUILDTREES_DIR}/src)

file(COPY "${VCPKG_ROOT_DIR}/../../HeaderMake/HeaderMake.cpp" DESTINATION "${SOURCE_PATH}")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/vcpkg.json" DESTINATION "${SOURCE_PATH}")

# CMakeコンフィグを実行。
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

# 一旦インストールする。
vcpkg_cmake_install()

# /binに出力されたexeをツールフォルダにコピー。
vcpkg_copy_tools(TOOL_NAMES HeaderMake)

# ツールインストールに不要なフォルダーを削除。
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/bin" "${CURRENT_PACKAGES_DIR}/debug")

# 著作権ファイルはトップフォルダーのものを流用する。
vcpkg_install_copyright(FILE_LIST "${VCPKG_ROOT_DIR}/../../LICENSE")

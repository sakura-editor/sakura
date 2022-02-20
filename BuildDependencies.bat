@echo off
setlocal

cd "%~dp0vcpkg"

IF not EXIST vcpkg.exe (
	CALL bootstrap-vcpkg.bat
)

vcpkg install boost-spirit:x64-windows
vcpkg install boost-spirit:x86-windows

IF not EXIST "%~dp0.nuget" (
	MKDIR "%~dp0.nuget"
)

vcpkg export ^
	boost-algorithm:x86-windows ^
	boost-align:x86-windows ^
	boost-array:x86-windows ^
	boost-assert:x86-windows ^
	boost-atomic:x86-windows ^
	boost-bind:x86-windows ^
	boost-build:x64-windows ^
	boost-chrono:x86-windows ^
	boost-concept-check:x86-windows ^
	boost-config:x86-windows ^
	boost-container:x86-windows ^
	boost-container-hash:x86-windows ^
	boost-conversion:x86-windows ^
	boost-core:x86-windows ^
	boost-date-time:x86-windows ^
	boost-detail:x86-windows ^
	boost-endian:x86-windows ^
	boost-exception:x86-windows ^
	boost-function:x86-windows ^
	boost-function-types:x86-windows ^
	boost-fusion:x86-windows ^
	boost-integer:x86-windows ^
	boost-intrusive:x86-windows ^
	boost-io:x86-windows ^
	boost-iterator:x86-windows ^
	boost-lexical-cast:x86-windows ^
	boost-modular-build-helper:x64-windows ^
	boost-move:x86-windows ^
	boost-mp11:x86-windows ^
	boost-mpl:x86-windows ^
	boost-numeric-conversion:x86-windows ^
	boost-optional:x86-windows ^
	boost-phoenix:x86-windows ^
	boost-pool:x86-windows ^
	boost-predef:x86-windows ^
	boost-preprocessor:x86-windows ^
	boost-proto:x86-windows ^
	boost-range:x86-windows ^
	boost-ratio:x86-windows ^
	boost-rational:x86-windows ^
	boost-regex:x86-windows ^
	boost-smart-ptr:x86-windows ^
    boost-spirit:x86-windows ^
	boost-static-assert:x86-windows ^
	boost-system:x86-windows ^
	boost-thread:x86-windows ^
	boost-throw-exception:x86-windows ^
	boost-tokenizer:x86-windows ^
	boost-tuple:x86-windows ^
	boost-type-index:x86-windows ^
	boost-type-traits:x86-windows ^
	boost-typeof:x86-windows ^
	boost-uninstall:x86-windows ^
	boost-uninstall:x64-windows ^
	boost-unordered:x86-windows ^
	boost-utility:x86-windows ^
	boost-variant:x86-windows ^
	boost-variant2:x86-windows ^
	boost-vcpkg-helpers:x86-windows ^
	boost-winapi:x86-windows ^
	vcpkg-cmake:x64-windows ^
    boost-spirit:x64-windows ^
	--nuget --nuget-id=sakura.dependencies --nuget-version=1.0.0 --output-dir="%~dp0.nuget"

vcpkg export ^
	boost-algorithm:x86-windows ^
	boost-align:x86-windows ^
	boost-array:x86-windows ^
	boost-assert:x86-windows ^
	boost-atomic:x86-windows ^
	boost-bind:x86-windows ^
	boost-build:x64-windows ^
	boost-chrono:x86-windows ^
	boost-concept-check:x86-windows ^
	boost-config:x86-windows ^
	boost-container:x86-windows ^
	boost-container-hash:x86-windows ^
	boost-conversion:x86-windows ^
	boost-core:x86-windows ^
	boost-date-time:x86-windows ^
	boost-detail:x86-windows ^
	boost-endian:x86-windows ^
	boost-exception:x86-windows ^
	boost-function:x86-windows ^
	boost-function-types:x86-windows ^
	boost-fusion:x86-windows ^
	boost-integer:x86-windows ^
	boost-intrusive:x86-windows ^
	boost-io:x86-windows ^
	boost-iterator:x86-windows ^
	boost-lexical-cast:x86-windows ^
	boost-modular-build-helper:x64-windows ^
	boost-move:x86-windows ^
	boost-mp11:x86-windows ^
	boost-mpl:x86-windows ^
	boost-numeric-conversion:x86-windows ^
	boost-optional:x86-windows ^
	boost-phoenix:x86-windows ^
	boost-pool:x86-windows ^
	boost-predef:x86-windows ^
	boost-preprocessor:x86-windows ^
	boost-proto:x86-windows ^
	boost-range:x86-windows ^
	boost-ratio:x86-windows ^
	boost-rational:x86-windows ^
	boost-regex:x86-windows ^
	boost-smart-ptr:x86-windows ^
    boost-spirit:x86-windows ^
	boost-static-assert:x86-windows ^
	boost-system:x86-windows ^
	boost-thread:x86-windows ^
	boost-throw-exception:x86-windows ^
	boost-tokenizer:x86-windows ^
	boost-tuple:x86-windows ^
	boost-type-index:x86-windows ^
	boost-type-traits:x86-windows ^
	boost-typeof:x86-windows ^
	boost-uninstall:x86-windows ^
	boost-uninstall:x64-windows ^
	boost-unordered:x86-windows ^
	boost-utility:x86-windows ^
	boost-variant:x86-windows ^
	boost-variant2:x86-windows ^
	boost-vcpkg-helpers:x86-windows ^
	boost-winapi:x86-windows ^
	vcpkg-cmake:x64-windows ^
    boost-spirit:x64-windows ^
	--nuget --nuget-id=tests1.dependencies --nuget-version=1.0.0 --output-dir="%~dp0.nuget"

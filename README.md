# Workload report generator

Gets worklog report from Jira and calculated workload.

## Platform

Windows 10, Visual Studio 2019, CMake 3.18, Ninja

## Dependencies

Vcpkg package mamanger is used.

Dependencies are

* Boost
`vcpkg install boost:x64-windows-static`

* Logger - [spdlog](https://github.com/gabime/spdlog)
`vcpkg install spdlog:x64-windows-static`

* Formatting - [Fmt](https://github.com/fmtlib/fmt)
`vcpkg install fmt:x64-windows-static`

* Excel writing - [xlnt](https://github.com/tfussell/xlnt)
`vcpkg install xlnt:x64-windows-static`

* Json Parser - [nlohmann](https://github.com/nlohmann/json)
`vcpkg install nlohmann-json:x64-windows-static`

* Json Schema - [json schema validator](https://github.com/pboettch/json-schema-validator)
`vcpkg install json-schema-validator:x64-windows-static`

* Testing - [Catch2](https://github.com/catchorg/Catch2)
`vcpkg install catch2:x64-windows-static`

## VSCode configuration

Set following cmake options for the project. Content of `./vscode/settings.json`

```json
{
    "cmake.generator": "Ninja",
    "cmake.configureSettings": {
        "CMAKE_TOOLCHAIN_FILE": "C:/apps/vcpkg/scripts/buildsystems/vcpkg.cmake",
        "VCPKG_TARGET_TRIPLET": "x64-windows-static"
    }
}
```

## Codestyle

Used Facebook Folly codestyle. It's not documented explicitly. See project [itself](https://github.com/facebook/folly) for refference.

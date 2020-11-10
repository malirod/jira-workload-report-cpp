# Workload report generator

Gets worklog report from Jira and generates excel report with workload.

## Platform

Windows 10, Visual Studio 2019, CMake 3.19, Ninja

## Dependencies

Vcpkg package mamanger is used.

Dependencies are:

* [boost 1.74](https://boost.org)
* [spdlog 1.8.0#1](https://github.com/gabime/spdlog)
* [fmt 7.1.2](https://github.com/fmtlib/fmt)
* [xlnt 1.5.0](https://github.com/tfussell/xlnt)
* [nlohmann 3.9.1](https://github.com/nlohmann/json)
* [json-schema-validator 2.1.0](https://github.com/pboettch/json-schema-validator)
* [catch2 2.13.1](https://github.com/catchorg/Catch2)

## VSCode configuration

Set following cmake options for the project. Content of `./vscode/settings.json`

```json
{
    "cmake.generator": "Ninja",
    "cmake.configureSettings": {
        "CMAKE_TOOLCHAIN_FILE": "[vcpkg root]/scripts/buildsystems/vcpkg.cmake",
        "VCPKG_TARGET_TRIPLET": "x64-windows-static"
    }
}
```

## Codestyle

Used Facebook Folly codestyle. It's not documented explicitly. See project [itself](https://github.com/facebook/folly) for refference.

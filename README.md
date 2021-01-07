# Workload report generator

Gets worklog report from Jira and generates excel report with workload.

## Platform

Windows 10, Visual Studio 2019, CMake 3.19, Ninja

## Dependencies

Vcpkg package mamanger is used. All dependencies are listed in `vcpkg.json`.

Dependencies are:

* [boost](https://boost.org)
* [spdlog](https://github.com/gabime/spdlog)
* [fmt](https://github.com/fmtlib/fmt)
* [xlnt](https://github.com/tfussell/xlnt)
* [nlohmann](https://github.com/nlohmann/json)
* [json-schema-validator](https://github.com/pboettch/json-schema-validator)
* [catch2](https://github.com/catchorg/Catch2)

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

### Clangd plugin

Requires `compile_commands.json` to be  present in the project root. Copy it from build directory or create symlink.

#### Create symlink on Windows

Run followin command in the console with admin rigths

```shell
mklink compile_commands.json .\build\compile_commands.json
```

## Codestyle

Used Facebook Folly codestyle. It's not documented explicitly. See project [itself](https://github.com/facebook/folly) for refference.

## Clang-tidy

Expected that `compile_commands.json` is present in the project root.

```shell
python ./tools/run-clang-tidy.py -quiet
```

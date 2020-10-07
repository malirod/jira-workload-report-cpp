# Workload report generator

Gets worklog report from Jira and calculated workload.

## Platform

Windows 10, Visual Studio 2019, CMake 3.18, Ninja

## Dependencies

Vcpkg package mamanger is used.

Dependencies are

* Facebook Folly which in turn depends on Boost
`vcpkg install folly:x64-windows`

* Fmt
`vcpkg install fmt:x64-windows`

* xlnt
`vcpkg install xlnt:x64-windows`

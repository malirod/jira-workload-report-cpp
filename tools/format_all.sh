#!/bin/bash

# SPDX-License-Identifier: MIT

# Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

cd "$(dirname "$0")/.."
find jwlrep/ -type f '(' -name '*.cpp' -o -name '*.h' ')' -exec clang-format -style=file -i {} \;

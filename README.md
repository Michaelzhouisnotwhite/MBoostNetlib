# MBoostNetLib

A net lib based on boost asio, contains TCP client/server and HTTP client/server.

- [MBoostNetLib](#mboostnetlib)
  - [Installation](#installation)
    - [Unix](#unix)
  - [Quick Start](#quick-start)

## Motivation

Boost.Asio is a very popular c++ library. So I use this library to create a client/server utilities.

## Installation

### Unix

Modify the configuration in CMakePresets.json.

```jsonc
{
  "configurePresets": [
    {
      "name": "gnu-configure",
      "binaryDir": "${sourceDir}/cmake-preset-build/gnu/",
      "generator": "Ninja",
      "cacheVariables": {},
      "environment": {
        "CC": "/path/to/gcc",
        "CXX": "/path/to/g++"
      }
    }
  ]
}
```

cmake configure

```shell
# cmake configure
cmake --preset gnu-configure
```

cmake build

```shell
# cmake configure
cmake --build --preset gnu-build
```

## Quick Start

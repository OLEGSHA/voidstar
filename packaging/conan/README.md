# voidstar library Conan packaging

A [Conan 2.x](https://conan.io/) recipe for this package. The layout is compatible with [Conan Center Index](https://github.com/conan-io/conan-center-index/).

## Usage with Conan Center-compatible repositories

Copy `packaging/conan/voidstar` directory into the package list. It should work as-is, fetching sources from a [github.com/OLEGSHA/voidstar release](https://github.com/OLEGSHA/voidstar/releases).

## Local/development usage

To package an existing release, cd into `packaging/conan/voidstar/all` and use Conan commands as usual, e.g. `conan create . --version=0.1.0`.

Testing packaging of a modified release is trickier. Use `packaging/conan/create_dev_package.sh` to create a testing environment for the Conan recipe:

```sh
# Create a testing release archive and prepare conandata.yml
packaging/conan/create_dev_package.sh
cd build/packaging/conan

# Use Conan commands as usual
conan create --version=dev .

# Run create_dev_package.sh again after updating sources
edit some-header.h
../../../packaging/conan/create_dev_package.sh
conan create --version=dev .

# conanfile.py and test_package are symlinked, no updates required
edit conanfile.py
conan create --version=dev .
```


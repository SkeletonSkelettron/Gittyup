Gittyup-ng
==================================

Gittyup-ng is a graphical Git client designed to help you understand and manage your source code history.
It continues [Gittyup](https://github.com/Murmele/Gittyup) with a new interface written in QML.
Windows installers are on the [releases page](https://github.com/SkeletonSkelettron/Gittyup-ng/releases);
on other systems, build it from source by following the directions [below](#how-to-build).

Gittyup-ng runs beside Gittyup. The first time it runs, it starts with the settings of Gittyup.

Gittyup is a continuation of the [GitAhead](https://github.com/gitahead/gitahead) client.

Table of contents
=================
<!--ts-->
   * [Features](#features)
   * [How to Get Help](#how-to-get-help)
   * [Build Environment](#build-environment)
   * [Dependencies](#dependencies)
   * [How to Build](#how-to-build)
   * [How to Install](#how-to-install)
   * [How to Contribute](#how-to-contribute)
   * [License](#license)
<!--te-->

Features
---------------
To get an overview of the current features please have a look at the [GitHub Page](https://murmele.github.io/Gittyup/)

How to Get Help
---------------

Ask questions about building or using Gittyup on
[Stack Overflow](http://stackoverflow.com/questions/tagged/gittyup) by
including the `gittyup` tag. Remember to search for existing questions
before creating a new one.

Report bugs in Gittyup by opening an issue in the
[issue tracker](https://github.com/Murmele/gittyup/issues).
Remember to search for existing issues before creating a new one.

If you still need help, check out our Matrix channel
[Gittyup:matrix.org](https://matrix.to/#/#Gittyup:matrix.org).

Build Environment
-----------------

* C++11 compiler
  * Windows - MSVC >= 2017 recommended
  * Linux - GCC >= 6.2 recommended
  * macOS - Xcode >= 10.1 recommended
* CMake >= 3.19
* Ninja (optional)

Dependencies
------------

External dependencies can be satisfied by system libraries or installed
separately. Included dependencies are submodules of this repository. Some
submodules are optional or may also be satisfied by system libraries.

**External Dependencies**

* Qt (required >= 6.4)

**Included Dependencies**

* libgit2 (required)
* cmark (required)
* git (only needed for the credential helpers)
* libssh2 (needed by `libgit2` for SSH support)
* openssl (needed by `libssh2` and `libgit2` on some platforms)

Note that building `OpenSSL` on Windows requires `Perl` and `NASM`.

How to Build
------------

**Initialize Submodules**

    git submodule init
    git submodule update --depth 1

**Build OpenSSL**

    # Start from root of gittyup repo.
    cd dep/openssl/openssl

Windows:

    perl Configure VC-WIN64A
    nmake

macOS (Intel):

    ./Configure darwin64-x86_64-cc no-shared
    make
    
macOS (Apple Silicon)

    ./Configure darwin64-arm64-cc no-shared
    make
    
Linux:

    ./config -fPIC
    make

**Configure Build**

    # Start from root of gittyup repo.
    mkdir -p build/release
    cd build/release
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ../..

If you have Qt installed in a non-standard location, you may have to
specify the path to Qt by passing `-DCMAKE_PREFIX_PATH=<path-to-qt>`
where `<path-to-qt>` points to the Qt install directory that contains
`bin`, `lib`, etc.

**Build**
```
    ninja
```
    
### A Convenient Shell Script for Ubuntu is available [here](https://raw.githubusercontent.com/Murmele/Gittyup/master/pack/buildUbuntu.sh), and will install all the necessary prerequisites, and build a release version for immediate use.

How to Install
-----------------
### Windows

Download the installer or the zip from the [releases page](https://github.com/SkeletonSkelettron/Gittyup-ng/releases).
Start Gittyup-ng from the Start menu after installing it.

### Linux and macOS

Build Gittyup-ng from source by following the directions [above](#how-to-build).
The packages on Flathub, the AUR and Homebrew install Gittyup, not Gittyup-ng.

How to Contribute
-----------------

We welcome contributions of all kinds, including bug fixes, new features,
documentation and translations. By contributing, you agree to release
your contributions under the terms of the license.

Contribute by following the typical
[GitHub workflow](https://docs.github.com/en/get-started/quickstart/github-flow)
for pull requests. Fork the repository and make changes on a new named
branch. Create pull requests against the `master` branch. Follow the
[seven guidelines](https://chris.beams.io/posts/git-commit/) to writing a
great commit message.

Prior to committing a change, please use `cl-fmt.sh` to ensure your code
adheres to the formatting conventions for this project. You can also use the
`setup-env.sh` script to install a pre-commit hook which will automatically
run `clang-format` against all modified files.

Prior to pushing a change, please ensure you run the unit tests to avoid any
regressions. These are run using `ctest` in `<build-dir>`.

License
-------

Gittyup-ng and its predecessors Gittyup and GitAhead are licensed under the MIT license. See LICENSE.md for details.

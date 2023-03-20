#
# Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#

_triggers = {"branch": ["master", "develop", "drone*", "bugfix/*", "feature/*", "fix/*", "pr/*"]}
_container_tag = '65e51d3af7132dcb1001249629c24cc59b934cb6'


def linux_cmake(branch, name, image, packages):
  return {
    "name": name,
      "kind": "pipeline",
      "type": "docker",
      "trigger": _triggers,
      "platform": {
        "os": "linux",
        "arch": "amd64"
      },
      "clone": {
        "retries": 5
      },
      "node": {},
      "steps" : [
      {
        "name": "Install Dependencies",
        "image": image,
        "pull": "if-not-exists",
        "commands": [
          "dnf install -y " + " ".join(packages)
        ]
      },
      {
        "name": "Get boost",
        "image": image,
        "pull": "if-not-exists",
        "commands": [
          "cd ..",
          "git clone -b {} --depth 1 https://github.com/boostorg/boost.git".format(branch),
          "cd boost",
          """git submodule update --init --depth 20 --jobs 4 \
              libs/array     \
              libs/headers   \
              libs/system    \
              libs/container \
              libs/asio      \
              libs/core      \
              libs/config    \
              libs/assert
          """
        ]
      }]
    }




def main(ctx):
  branch = ctx.build.branch
  if branch != 'master' or branch != 'refs/heads/master':
    branch = 'develop';

  return [
    linux_cmake(branch=branch,
                name="gcc 10 (fedora 36)",
                image="docker.io/library/fedora:36",
                packages=["g++", "cmake", "git"])
  ]
#   return [
#       # CMake Linux
#       linux_cmake('Linux CMake valgrind',       _image('build-gcc11'), valgrind=1, build_shared_libs=0),
#       linux_cmake('Linux CMake coverage',       _image('build-gcc11'), coverage=1, build_shared_libs=0),
#       linux_cmake('Linux CMake MySQL 5.x',      _image('build-clang14'),           build_shared_libs=0),
#       linux_cmake('Linux CMake MariaDB',        _image('build-clang14'),           build_shared_libs=1),
#       linux_cmake('Linux CMake cmake 3.8',      _image('build-cmake3_8'),   standalone_tests=0,   install_tests=0),
#       linux_cmake('Linux CMake no OpenSSL',     _image('build-noopenssl'),  standalone_tests=0, add_subdir_tests=0, install_tests=0),
#       linux_cmake('Linux CMake gcc Release',    _image('build-gcc11'), cmake_build_type='Release'),
#       linux_cmake('Linux CMake gcc MinSizeRel', _image('build-gcc11'), cmake_build_type='MinSizeRel'),

#       # CMake Windows
#       windows_cmake('Windows CMake static', build_shared_libs=0),
#       windows_cmake('Windows CMake shared', build_shared_libs=1),

#       # Docs
#       docs()
#   ]

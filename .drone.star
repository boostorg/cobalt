#
# Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#

deps = [
    'libs/array',
    'libs/align',
    'libs/algorithm',
    'libs/asio',
    'libs/assert',
    'libs/beast',
    'libs/bind',
    'libs/callable_traits',
    'libs/chrono',
    'libs/circular_buffer',
    'libs/concept_check',
    'libs/config',
    'libs/container',
    'libs/context',
    'libs/core',
    'libs/date_time',
    'libs/detail',
    'libs/exception',
    'libs/function',
    'libs/headers',
    'libs/integer',
    'libs/intrusive',
    'libs/io',
    'libs/iterator',
    'libs/leaf',
    'libs/mpl',
    'libs/move',
    'libs/mp11',
    'libs/numeric',
    'libs/optional',
    'libs/predef',
    'libs/preprocessor',
    'libs/range',
    'libs/ratio',
    'libs/smart_ptr',
    'libs/static_assert',
    'libs/system',
    'libs/test',
    'libs/thread',
    'libs/throw_exception',
    'libs/type_traits',
    'libs/utility',
    'libs/variant2',
    'libs/winapi',
    'tools/build',
    'tools/boost_install',
    'tools/boostdep'
    ]


def git_boost_steps(branch, image="alpine/git", env_win_style=False):
    return [{
            "name": "boost ({})".format(branch),
            "image": image,
            "commands": [
               "git clone -b {} --depth 1 https://github.com/boostorg/boost.git boost".format(branch)
                ]
        },
        {
            "name": "boost submodules",
            "image": image,
            "commands": [
                "cd boost",
                "git submodule update --init --depth 20 --jobs 8 " + " ".join(deps)
            ]
        },
        {
            "name": "clone",
            "image": image,
            "commands": [
                "cd boost/libs",
                "git clone {} cobalt".format("$Env:DRONE_GIT_HTTP_URL" if env_win_style else "$DRONE_GIT_HTTP_URL"),
                "cd cobalt",
                "git checkout {}".format("$Env:DRONE_COMMIT" if env_win_style else "$DRONE_COMMIT")
            ]
        }
    ]


def format_b2_args(**kwargs):
    res = ""
    for k in kwargs:
        res += " {}={}".format(k.replace('_', '-'), kwargs[k])
    return res


def linux_build_steps(image, privileged, **kwargs):
    args = format_b2_args(**kwargs)
    return [
        {
            "name": "bootstrap",
            "image": image,
            "commands": [
                "cd boost",
                "./bootstrap.sh"
            ]
        },
        {
            "name": "build",
            "image": image,
            "commands" : [
                "cd boost/libs/cobalt",
                "../../b2 build -j8 " + args
            ]
        },
        {
            "name": "test",
            "image": image,
            "privileged" : privileged,
            "commands" : [
                "echo 0 | sudo tee /proc/sys/kernel/randomize_va_space" if privileged else "echo",
                "cd boost/libs/cobalt",
                "../../b2 test -j8 " + args
            ]
        }
    ]


def windows_build_steps(image, **kwargs):
    args = format_b2_args(**kwargs)
    return [
        {
            "name": "bootstrap",
            "image": image,
            "commands": [
                "cd boost",
                ".\\\\bootstrap.bat"
            ]
        },
        {
            "name" : "build",
            "image" : image,
            "commands": [
                "cd boost/libs/cobalt",
                "..\\\\..\\\\b2 build -j8 " + args
            ]
        },
        {
            "name": "test",
            "image": image,
            "commands": [
                "cd boost/libs/cobalt",
                "..\\\\..\\\\b2 test -j8 " + args
            ]
        }
    ]


def linux(
        name,
        branch,
        image,
        privileged = False,
        **kwargs):

    return {
        "kind": "pipeline",
        "type": "docker",
        "name": name,
        "clone": {"disable": True},
        "platform": {
            "os": "linux",
            "arch": "amd64"
        },
        "steps": git_boost_steps(branch) + linux_build_steps(image, privileged, **kwargs)
    }


def windows(
        name,
        branch,
        image,
        **kwargs):

    return {
        "kind": "pipeline",
        "type": "docker",
        "name": name,
        "clone": {"disable": True},
        "platform": {
            "os": "windows",
            "arch": "amd64"
        },
        "steps": git_boost_steps(branch, image, True) + windows_build_steps(image, **kwargs)
    }


def main(ctx):
    branch = ctx.build.branch
    if ctx.build.event == 'tag' or (branch != 'master' and branch != 'refs/heads/master'):
        branch = 'develop'

    return [
        linux("gcc-11",                 branch, "docker.io/library/gcc:11",   variant="release", cxxstd="20"),
        linux("gcc-12",                 branch, "docker.io/library/gcc:12.3", variant="release", cxxstd="20"),
        linux("gcc-13",                 branch, "docker.io/library/gcc:13",   variant="release", cxxstd="20"),
        linux("gcc-13 (asan)",          branch, "docker.io/library/gcc:13",   variant="release", cxxstd="20", debug_symbols="on", address_sanitizer="on"),
        linux("gcc-13 (usan)",          branch, "docker.io/library/gcc:13",   variant="release", cxxstd="20", debug_symbols="on", undefined_sanitizer="on"),
        linux("gcc-13 (tsan)",          branch, "docker.io/library/gcc:13",   variant="release", cxxstd="20", debug_symbols="on", thread_sanitizer="on"),
        linux("gcc-13 (io_context)",    branch, "docker.io/library/gcc:13",   variant="release", cxxstd="20", **{'boost.cobalt.executor': 'use_io_context'}),
        linux("gcc-13 (container.pmr)", branch, "docker.io/library/gcc:13",   variant="release", cxxstd="20", **{'boost.cobalt.pmr': 'boost-container'}),
        linux("gcc-13 (no pmr)",        branch, "docker.io/library/gcc:13",   variant="release", cxxstd="20", **{'boost.cobalt.pmr': 'no'}),
        linux("clang",                  branch, "cppalliance/droneubuntu2404:1", privileged=True, toolset='clang', variant="release", cxxstd="20"),
        linux("clang (container.pmr)",  branch, "cppalliance/droneubuntu2404:1", privileged=True, toolset='clang', variant="release", cxxstd="20", **{'boost.cobalt.pmr': 'boost-container'}),
        linux("clang (no pmr)",         branch, "cppalliance/droneubuntu2404:1", privileged=True, toolset='clang', variant="release", cxxstd="20", **{'boost.cobalt.pmr': 'no'}),
        linux("clang (asan)",           branch, "cppalliance/droneubuntu2404:1", privileged=True, toolset='clang', variant="release", cxxstd="20", debug_symbols="on",  address_sanitizer="on"),
        linux("clang (usan)",           branch, "cppalliance/droneubuntu2404:1", privileged=True, toolset='clang', variant="release", cxxstd="20", debug_symbols="on",  undefined_sanitizer="on"),
        linux("clang (tsan)",           branch, "cppalliance/droneubuntu2404:1", privileged=True, toolset='clang', variant="release", cxxstd="20", debug_symbols="on",  thread_sanitizer="on"),
        windows("msvc-14.3 (x64)",      branch, "cppalliance/dronevs2022:latest", variant="release", cxxstd="20", address_model="64"),
        windows("msvc-14.3 (x32)",      branch, "cppalliance/dronevs2022:latest", variant="release", cxxstd="20", address_model="32")
    ]

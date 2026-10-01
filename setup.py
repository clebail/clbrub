#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Module Python autonome rubikcore (sans Qt) : python3 setup.py build_ext --inplace

import numpy
from setuptools import setup, Extension

setup(
    name="rubikcore",
    version="0.1",
    ext_modules=[
        Extension(
            "rubikcore",
            sources=["rubikcore.cpp", "CCubeCore.cpp"],
            depends=["CCubeCore.h"],
            include_dirs=[numpy.get_include()],
            extra_compile_args=["-std=c++11", "-O3", "-Wall", "-Wextra"],
            language="c++",
        )
    ],
)

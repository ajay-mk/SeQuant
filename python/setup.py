#!/usr/bin/env python3
"""
Setup script for SeQuant Python interface.

This package provides Python bindings to the SeQuant C++ library for
symbolic tensor algebra in quantum many-body theory.

Installation
------------

Standard installation (requires SeQuant C++ library to be built):
    pip install .

Development installation:
    pip install -e .

Note: The C++ extension module (_sequant) must be built separately using CMake
before installing this package. See the main SeQuant documentation for details
on building with Python support.

Build from source (alternative method):
    python setup.py build_ext --inplace
"""

from setuptools import setup, find_packages
import os
import sys

# Read the long description from README
long_description = ""
readme_path = os.path.join(os.path.dirname(__file__), "README.md")
if os.path.exists(readme_path):
    with open(readme_path, "r", encoding="utf-8") as f:
        long_description = f.read()
else:
    long_description = __doc__

# Package metadata
PACKAGE_NAME = "sequant"
VERSION = "0.1.0"
DESCRIPTION = "Python interface to SeQuant library for symbolic tensor algebra"
AUTHOR = "The SeQuant Developers"
AUTHOR_EMAIL = "valeev@vt.edu"
URL = "https://github.com/ValeevGroup/SeQuant"
LICENSE = "LGPL-3.0"

# Python version requirement
PYTHON_REQUIRES = ">=3.8"

# Runtime dependencies
INSTALL_REQUIRES = []

# Development dependencies
EXTRAS_REQUIRE = {
    "dev": [
        "pytest>=6.0",
        "pytest-cov",
        "black",
        "mypy",
    ],
    "docs": [
        "sphinx>=4.0",
        "sphinx-rtd-theme",
    ],
}

# Classifiers
CLASSIFIERS = [
    "Development Status :: 3 - Alpha",
    "Intended Audience :: Science/Research",
    "License :: OSI Approved :: GNU Lesser General Public License v3 (LGPLv3)",
    "Operating System :: POSIX :: Linux",
    "Operating System :: MacOS",
    "Programming Language :: C++",
    "Programming Language :: Python :: 3",
    "Programming Language :: Python :: 3.8",
    "Programming Language :: Python :: 3.9",
    "Programming Language :: Python :: 3.10",
    "Programming Language :: Python :: 3.11",
    "Programming Language :: Python :: 3.12",
    "Topic :: Scientific/Engineering :: Chemistry",
    "Topic :: Scientific/Engineering :: Physics",
]

# Keywords
KEYWORDS = ["quantum chemistry", "tensor algebra", "coupled cluster", "many-body theory"]


def get_package_data():
    """
    Specify non-Python files to include in the package.
    The compiled extension module (_sequant.so) will be included automatically
    if it's in the package directory.
    """
    package_data = {
        "sequant": [
            "*.so",  # Linux/Mac shared libraries
            "*.pyd",  # Windows extension modules
            "py.typed",  # PEP 561 type marker
        ],
    }
    return package_data


def main():
    """Main setup function."""

    # Check if we're building from source or installing pre-built
    if "build_ext" in sys.argv:
        print(
            "WARNING: The C++ extension should be built using CMake, not setup.py.\n"
            "Please refer to the SeQuant documentation for build instructions.\n"
            "This setup.py is primarily for packaging the already-built extension.",
            file=sys.stderr,
        )

    setup(
        name=PACKAGE_NAME,
        version=VERSION,
        description=DESCRIPTION,
        long_description=long_description,
        long_description_content_type="text/markdown",
        author=AUTHOR,
        author_email=AUTHOR_EMAIL,
        url=URL,
        license=LICENSE,
        packages=find_packages(exclude=["tests", "tests.*", "examples"]),
        package_data=get_package_data(),
        python_requires=PYTHON_REQUIRES,
        install_requires=INSTALL_REQUIRES,
        extras_require=EXTRAS_REQUIRE,
        classifiers=CLASSIFIERS,
        keywords=KEYWORDS,
        zip_safe=False,  # Don't install as a zip file (needed for extension modules)
        include_package_data=True,
    )


if __name__ == "__main__":
    main()

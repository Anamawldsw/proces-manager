from setuptools import setup, Extension
import pybind11

ext_modules = [
    Extension(
        "test",
        ["main_module.cpp"],
        include_dirs=[pybind11.get_include()],
        language="c++"
    ),
]

setup(
    name = "test",
    ext_modules = ext_modules,
)
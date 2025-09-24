from setuptools import setup, find_packages

setup(
    name="process-command-example",
    version="0.1.0",
    packages=find_packages(),
    install_requires=[
        "pytest",
        "pytest-mock",
    ],
)

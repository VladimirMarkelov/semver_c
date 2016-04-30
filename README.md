# Semantic Versioning
Crossplatform library for parsing and comparing versions.
For details about semantic versioning, please see ['Semantic Versioning'](http://semver.org/)

# API

## Parsing versions

### int parse_version(const char* str, SemVersion* version);
The function to parse a string that contains a version description or to check the validity of the version.
Input parameters:
* str - pointer to a string with version description: a string of digits, Latin letters, dots, '+' and '-' signs using format MAJOR.MINOR.PATCH[-release][+build]


# Using the library
I use MinGW to build the library on Windows.

In Linux just run **make** to build the library or **make && cd tests && make** to build the library and its demo

In Windows run **make -f Makefile.win** to build the library.

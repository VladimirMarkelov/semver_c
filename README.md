# Semantic Versioning
Crossplatform library for parsing and comparing versions - version 1.0.0.
For details about semantic versioning, please see ['Semantic Versioning'](http://semver.org/)

# API

## Parsing versions

### int parse_version(const char* str, SemVersion* version)
The function parses a string that contains a version description and/or checks the validity of the version. If **str** is not **NULL** and **version** is not **NULL** then version structure is filled with zeroes before parsing. So, **version** can contain invalid or incomplete data in case of parse_version returns an error.

Input:
* str - pointer to a string with version description: a string of digits, Latin letters, dots, '+' and '-' signs using format MAJOR.MINOR.PATCH[-prerelease][+build]. Release and build parts are optional. Trailing and leading spaces are allowed and skipped while parsing the version. Argument 'str' cannot be NULL.
* version - a pointer to a structure that is used to keep parsed version. This parameter can be NULL. In this case the function does only validity check.

The function result is error code - one of:
* SEMVER_OK - the version is valid and **version** is filled with correct and complete data
* SEMVER_INVALID_VERSION - **str** is **NULL**
* SEMVER_INVALID_MAJOR, SEMVER_INVALID_MINOR, SEMVER_INVALID_PATCH, SEMVER_INVALID_PRERELEASE, and SEMVER_INVALID_BUILD - fail to parse the corresponding part of a version.

Note: **str** can contain compare operator (one of <, <=, >=, >, ==, =, !=, ^, and ~). It maybe useful for parsing **ver_b** for a function version_equals

#### Compare operators
* > - greater (COMPARE_GREATER)
* >= - greater or equal (COMPARE_GREATEROREQUAL)
* < - less (COMPARE_LESS)
* less or equal (COMPARE_LESSOREQUAL)
* =, and == - equal (COMPARE_EQUAL)
* != - not equal (COMPARE_NEQUAL)
* ^ - major versions equal and the version is equal to or greater then (COMPARE_MAJOR)
* ~ - major and minor versions equal and the version is equal to or greater then (COMPARE_MINOR)

## Compare versions

### int compare_versions(const SemVersion* ver_a, const SemVersion* ver_b)
The function can be used to sort version list or just to compare two versions. The result of the function is an integer:
* <0 - **ver_b** is greater than **ver_a**
* >0 - **ver_a** is greater than **ver_b**
* 0 - versions are equal

Notes:

1. Build part of version does not affect the result. So compare_versions(1.0.0+build2, 1.0.0+build12) = 0, and compare_versions(1.0.0+45, 1.0.0) = 0
2. Version without prerelease part is greater than any other version with the same major, minor, and patch parts. So, compare_versions(1.0.0, 1.0.0-rc2) = 1
3. **ver_a** and **ver_b** can be NULL. NULL is considered less than any other version. If both arguments are NULL then the result of the function is 0.
4. If any section of prerelease starts with a digit than this section is treated as number (all letters after a digit after skipped till the next section or build number). So, compare_versions(1.0.0-beta.10a, 1.0.0-beta.10b) = 0, but compare_versions(1.0.0-beta.10.a, 1.0.0-beta.10.b) = -1
5. If sections of prereleases start with a letter than the sections are compared lexicographically. If one section is greater than the other and it starts with the shortest section than the longer section is greater: compare_versions(1.0.0-beta.abcd, 1.0.0-beta.abc) = 1

### int version_equals(const SemVersion* ver_a, const SemVersion* ver_b)
The function checks if the version **ver_a** meets the requirements set with **ver_b**.

Input:
* **ver_a** - version to compare. Its **cmp** field does not affect the function result.
* **ver_b** - version to compare with. It can be a simple version like **ver_a** or contain a rule to compare set in its field **cmp** (please see, **COMPARE_..** constants)

Result:
* 0 - **ver_a** does not meet requirement set with **ver_b**
* 1 - **ver_a** fits **ver_b** requirements.

Notes:

1. Empty **cmp** (COMPARE_NONE) is the same as COMPARE_EQUAL.
2. COMPARE_MINOR and COMPARE_MAJOR are syntax sugar for two compound checks:
  * COMPARE_MAJOR: (version_a >= version_b) && (verson_a.major == version_b.major)
  * COMPARE_MINOR: (version_a >= version_b) && (verson_a.major == version_b.major) && (verson_a.minor == version_b.minor)

### int check_version(const SemVersion* ver, const char *version_list)
The function checks if a version **ver** meets any of the requirements set in **version_list**.

Input:
* **ver** - version to check
* **version_list** - list of versions(including optional compare operators) separated with comma. Trailing and leading spaces are fine and are skipped but spaces after comma may make the string invalid. Range of versions is a two valid versions joined with ' - ' (please note that a dash must be surrounded with spaces, otherwise it is treated as prerelease separator). Range is a syntax sugar to mention minimal and maximal version witout compare operators. Both range limits are inclusive, so '1.0.0 - 1.1.0' means the same as '>=1.0.0,<=1.1.0'. There is a special version '*' - it means 'any version is OK'. Examples of valid version lists: '1.16.1 - 1.17.0,1.12.3-beta.31+345', '1.20.3,1.16.1 - 1.17.0', '>=1.11.0,<=1.14.1,!=1.12.20,1.10.9', ' * '.

Result:
* SEMVER_OK - version **ver** meets the list requirements
* SEMVER_OUT_OF_RANGE - version **ver** does not fit the requirements
* SEMVER_INVALID_VERSION - **ver** is **NULL**
* SEMVER_OUT_OF_MEMORY - failed to allocate memory for version range (if ranges are used)
* SEMVER_INVALID_VERSION_LIST - **version_list** is **NULL** or one of one of version or version range in list is invalid

Rules to check:

1. At first the function checks if a version meets any one-item rule in a version list (all items with COMPARE_NONE, COMPARE_EQUAL, COMPARE_NEQUAL, COMPARE_MAJOR, and COMPARE_MINOR **cmp** field) and stores version ranges in a version range list in the order of appearance, combining by two items of opposite **cmp** values. So, make sure that you enlist versions with COMPARE_GREATER, COMPARE_GREATEROREQUAL, COMPARE_LESS, and COMPARE_LESSOREQUAL in correct order (e.g, '>1.5.0,>=1.1.0,<1.2.0' makes two ranges to check: [ver>1.5.0 && ver<1.2.0] and [ver>=1.1.0] but '>=1.1.0,>1.5.0,<1.2.0' makes ranges to check: [ver>=1.1.0 && ver<1.2.0] and [ver>1.5.0])
2. If no single version equals **ver** then the function checks if **ver** fits any version range in list
3. COMPARE_MINOR and COMPARE_MAJOR are syntax sugar for two compound checks:
  * COMPARE_MAJOR: (version_a >= version_b) && (verson_a.major == version_b.major)
  * COMPARE_MINOR: (version_a >= version_b) && (verson_a.major == version_b.major) && (verson_a.minor == version_b.minor)

# Using the library

## Building the library
I use MinGW on Windows. To build the library and the tests using MinGW just run mingw32-make in the root directory. If you use another make tool you can set it in command line:
```
<tool-name> MAKE=<tool-name>
```
On Linux you can try the same trick (it should work):
```
make MAKE=<tool-name>
```

## Using without building the library
You can add only required files to your project to minimize size and compile time. The library contains 4 parts (each part depends on all previously mentioned parts):

1. Core does not depend on anyhting and includes only basic features: parse and check version, compare two versions, and check if a version meets a requirement set with another version. The core does not do any dynamic memory allocation - only static variables or pointer to a user-defined variabes. Core files:
  * semver.c
  * semver.h
2. Checking if a version fits any item in a version list. This function uses dynamic memory allocation. Files to include:
  * semver_check.c
  * semver_check.h
  * ver_range.c
  * ver_range.h
3. Pretty printing function for ranges and single version. Only test applications need these file (you can use them for debugging or logging):
  * semver_utils.c
  * semver_utils.h
4. Test applications: everything in the directory **test**


#ifndef SEMVER_20160414
#define SEMVER_20160414

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PRERELEASE_LEN 16
#define MAX_BUILD_LEN 16
#define INVALID_NUMBER UINT_MAX
#define PART_COUNT 3

typedef enum prerelease_t {
    /* version has prerelease description but
     * it is not alpha, beta or rc
     */
    PRERELEASE_BASIC,
    /* Alpha version */
    PRERELEASE_ALPHA,
    /* Beta version */
    PRERELEASE_BETA,
    /* Release candidate */
    PRERELEASE_RC,
    /* version does not have prerelease description */
    PRERELEASE_NONE,
} Prerelease;

typedef enum version_compare_t {
    COMPARE_NONE,
    COMPARE_EQUAL,
    COMPARE_NEQUAL,
    COMPARE_GREATER,
    COMPARE_LESS,
    COMPARE_GREATEROREQUAL,
    COMPARE_LESSOREQUAL,
    /* must have the same major version number and must be greater then or equal to the version.
     * A syntax sugar for :
     *      COMPARE_MAJOR(version, a.b.c-pre) == ((version >= a.b.c-pre) && (version < a+1.0.0))
     */
    COMPARE_MAJOR,
    /* must have the same major and minor version numbers and must be greater then or equal to the version
     * A syntax sugar for :
     *      COMPARE_MINOR(version, a.b.c-pre) == ((version >= a.b.c-pre) && (version < a.b+1.0))
     */
    COMPARE_MINOR,
} VersionCompare;

typedef struct semver_t {
    unsigned int major;
    unsigned int minor;
    unsigned int patch;
    VersionCompare cmp;
    Prerelease prerelease;
    char prerelease_str[MAX_PRERELEASE_LEN];
    char build_str[MAX_BUILD_LEN];
} SemVersion;

enum {
    SEMVER_OK = 0,
    SEMVER_INVALID_VERSION = 1,
    SEMVER_INVALID_MAJOR,
    SEMVER_INVALID_MINOR,
    SEMVER_INVALID_PATCH,

    SEMVER_INVALID_PRERELEASE,
    SEMVER_INVALID_BUILD,
    SEMVER_INVALID_VERSION_LIST,
    SEMVER_INVALID_RANGE,

    SEMVER_OUT_OF_MEMORY,
    SEMVER_INVALID_RANGE_ITEM,
    SEMVER_ITEM_FULL,
    SEMVER_OUT_OF_RANGE,
};

/* Parses string and fills the version structure.
 * version can be NULL - in this case the function just
 * checks if the str is valid version string.
 *
 * The function clears version structure before parsing,
 * so in case of any error the structure contains incomplete
 * data and cannot be used.
 *
 * Returns:
 * SEMVER_OK - str is valid version
 * SEMVER_INVALID_VERSION - str is NULL
 * SEMVER_INVALID_MAJOR, SEMVER_INVALID_MINOR, SEMVER_INVALID_PATCH,
 * SEMVER_INVALID_PRERELEASE, and SEMVER_INVALID_BUILD - corresponding
 * part of version string is not valid.
 */
int parse_version(const char* str, SemVersion* version);

/* Returns which version is greater:
 * -1 - ver_b is greater
 *  1 - ver_a is greater
 *  0 - versions are equal
 *  Notes:
 *  A. Empty preprelease(PRERELEASE_NONE) is greater than any
 *  other prerelease type, so 1.0.0 > 1.0.0-beta
 *  B. Build part of a version is not used to compare, so
 *  1.0.0+890 == 1.0.0+build1
 */
int compare_versions(const SemVersion* ver_a, const SemVersion* ver_b);

/* Returns 1 if ver_a meets the ver_b requirements, and 0 otherwise.
 *
 * Requirements are version to compare and compare operator set in
 * ver_b structure (COMPARE_NONE is the same as COMPARE_EQUAL).
 *
 * So, it returns 1 for:
 * A. ver_a(1.0.0) and ver_b(0.9.9 + COMPARE_GREATER)
 * B. ver_a(1.0.0) and ver_b(0.9.9 + COMPARE_NEQUAL)
 * C. ver_a(1.0.0) and ver_b(1.0.0 + COMPARE_EQUAL or COMPARE_NONE)
 */
int version_equals(const SemVersion* ver_a, const SemVersion* ver_b);

#ifdef __cplusplus
}
#endif
#endif

#ifndef SEMVER_CHECK_20160414
#define SEMVER_CHECK_20160414

#ifdef __cplusplus
extern "C" {
#endif

/* Returns SEMVER_OK if the version meets the requirements set with
 * version_list, SEMVER_OUT_OF_RANGE if it does not meet requirements,
 * or error code if version_list contains invalid data
 *
 * Returns SEMVER_INVALID_VERSION if ver is NULL
 *
 * Spaces at the beginning and the end of version_list are skipped.
 * Special version list value '*' means - any version is fine.
 *
 * Version list is a comma separated list of single versions and
 * version ranges.
 * A single version can have a compare operator right before it:
 * ^(compare only major version), ~(compare only major and minor
 * versions), >, <, >=, <=, = (or ==), and !=.
 * Version range is two valid versions witout compare operators
 * separated with ' - ' (note spaces around dash - without spaces
 * the function can parse the range as version with prerelease).
 * Examples of valid version lists:
 * 1.16.1 - 1.17.0,1.12.3-beta.31+345
 * 1.20.3,1.16.1 - 1.17.0
 * >=1.11.0,<=1.14.1,!=1.12.20,1.10.9
 * 1.16.1 - 1.17.0,0.10.3 - 5.13.9,1.2.15 - 1.2.70
 *
 * How check works:
 * At first it looks for the sinlge version rules (COMPARE_NONE,
 * COMPARE_EQUAL, COMPARE_NEQUAL, COMPARE_MAJOR, and COMPARE_MINOR).
 * If ver fits nothing of them then the function checks all version
 * ranges. So, if you want to limit the version with releases
 * the list '^1.1.0;>=1.0.0' won't work: check_version returns 1
 * for '1.1.0-beta' because the functions stops checking after the
 * first list item - COMPARE_MAJOR. You have to use the lists like
 * '>=1.1.0,<2.0.0' or '1.1.0 - 1.999.999' in this case.
 */
int check_version(const SemVersion* ver, const char *version_list);

#ifdef __cplusplus
}
#endif
#endif

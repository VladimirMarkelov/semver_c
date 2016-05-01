/* Utility functions.
 * The library does not use this module - it is for debug and logging purposes only.
 * This file can be excluded from library without any issue.
 */
#ifndef SEMVER_UTILS_20160417
#define SEMVER_UTILS_20160417

#ifdef __cplusplus
extern "C" {
#endif

struct VersionRange;
struct SemVersion;

/* Pretty print for VersionRange */
void print_range(VersionRange* range);
/* Pretty print for SemVersion */
void print_version(SemVersion* version);

#ifdef __cplusplus
}
#endif
#endif

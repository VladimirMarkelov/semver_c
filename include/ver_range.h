#ifndef RANGE_20160417
#define RANGE_20160417

#ifdef __cplusplus
extern "C" {
#endif

struct SemVersion;

/*
 * A simple internal structure to keep version ranges.
 * Storage is implemented as single-linked list. Every item
 * in the list describes a range with lower and upper limit.
 * Any limit can be NULL that means 'no limit'.
 *
 * min_ver can hold only versions with COMPARE_GREATER and
 * COMPARE_GREATEROREQUAL flag; max_ver can hold only versions
 * with COMPARE_LESS and COMPARE_LESSOREQUAL flag.
 */
typedef struct version_range_t {
    /* the lowest version in range, NULL - no limit */
    SemVersion *min_ver;
    /* the highest version in range, NULL - no limit */
    SemVersion *max_ver;
    /* pointer to the next range */
    struct version_range_t *next;
} VersionRange;

/* Initializes single-linked list and returns a pointer to the list head.
 * Returns NULL if initialization fails.
 * Do not change the returned values, otherwise free_version_range may fail.
 */
VersionRange* init_version_range();

/* Frees the list created with init_version_range.
 * It sets the head of the list to NULL at the end.
 */
void free_version_range(VersionRange** range);

/* Adds a range limit to the list.
 *
 * If as_new is not 0 then a new range created and added to the list.
 * Otherwise, the function tries to find the first half-full range
 * that can hold the version. If there is no appropriate half-full range
 * then a new range is created.
 *
 * The function returns a pointer to the range that is used to store
 * the version or NULL in case of any error: out of memory; version or
 * range is NULL; version compare operator is not one of COMPARE_GREATER,
 * COMPARE_LESS, COMPARE_GREATEROREQUAL, or COMPARE_LESSOREQUAL.
 *
 * A copy of version is added to the range list, so original structure
 * can be destroyes right after successful function call.
 */
VersionRange* add_version(VersionRange* range, SemVersion* version, int as_new);

/* Sets the limit for the selected half-full range.
 *
 * A copy of version is added to the range list, so original structure
 * can be destroyes right after successful function call.
 *
 * Returns:
 * SEMVER_OK - the limit is successfully set
 * SEMVER_OUT_OF_MEMORY - out of memory while trying to make version copy
 * SEMVER_INVALID_RANGE_ITEM - item is NULL
 * SEMVER_INVALID_VERSION - if version is NULL or compare flag is not one
 * of COMPARE_GREATER, COMPARE_LESS, COMPARE_GREATEROREQUAL, or COMPARE_LESSOREQUAL
 * SEMVER_ITEM_FULL - item does not fit: it is full or, e.g, you tried
 * to complete an item that has min_ver set with version that has compare
 * flag COMPARE_GREATER or COMPARE_GREATEROREQUAL
 */
int complete_version_range(VersionRange* item, SemVersion* version);

/* Returns size of the range list.
 * Returns 0 for NULL range.
 */
int range_size(const VersionRange* range);

/* Returns a pointer to item in the range list with index number idx.
 * Returns NULL if range is NULL or idx is out of range.
 */
VersionRange* get_range_item(VersionRange* range, int idx);

#ifdef __cplusplus
}
#endif
#endif

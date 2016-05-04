#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#include "semver.h"
#include "semver_check.h"
#include "ver_range.h"

int check_version(const SemVersion* ver, const char *version_list) {
    if (version_list == NULL) {
        return SEMVER_INVALID_VERSION_LIST;
    }
    if (ver == NULL) {
        return SEMVER_INVALID_VERSION;
    }

    while (*version_list != '\0' && *version_list == ' ') version_list++;

    if (*version_list == '*') {
        return SEMVER_OK;
    }

    char* tmp_version = calloc(strlen(version_list)+1, sizeof(char));
    int in_range = 0;
    int item_exists = 1;
    int res = SEMVER_OUT_OF_RANGE;
    VersionRange* range = NULL;
    VersionRange* first_item = NULL;

    while (item_exists) {
        while (*version_list == ' ' || *version_list == ',' || *version_list == '-') {
            version_list++;
        }
        if (*version_list == '\0') {
            break;
        }

        char* comma = strchr(version_list, ',');
        char* dash = strstr(version_list, " - ");
        int parse = SEMVER_OK;
        SemVersion v;

        if (comma == NULL && dash == NULL) {
            strcpy(tmp_version, version_list);
            item_exists = 0;
            parse = parse_version(tmp_version, &v);
            if (in_range == 1) {
                v.cmp = COMPARE_LESSOREQUAL;
                in_range = 2;
            }
        } else if ((comma == NULL && dash != NULL) ||
                   (dash != NULL && comma - version_list > dash - version_list)) {
            if (in_range) {
                res = SEMVER_INVALID_VERSION_LIST;
                break;
            } else {
                in_range = 1;
                strncpy(tmp_version, version_list, dash - version_list);
                tmp_version[dash - version_list] = '\0';
                version_list = dash + 3;
                parse = parse_version(tmp_version, &v);
                v.cmp = COMPARE_GREATEROREQUAL;
            }
        } else {
            strncpy(tmp_version, version_list, comma - version_list);
            tmp_version[comma - version_list] = '\0';
            version_list = comma + 1;
            parse = parse_version(tmp_version, &v);
            if (in_range == 1) {
                v.cmp = COMPARE_LESSOREQUAL;
                in_range = 2;
            }
        }

        if (parse != SEMVER_OK) {
            res = SEMVER_INVALID_VERSION_LIST;
            break;
        }

        if (v.cmp == COMPARE_NEQUAL || v.cmp == COMPARE_NONE || v.cmp == COMPARE_EQUAL) {
            if (version_equals(ver, &v)) {
                res = SEMVER_OK;
                break;
            } else {
                res = SEMVER_OUT_OF_RANGE;
            }
        } else if (v.cmp == COMPARE_MAJOR || v.cmp == COMPARE_MINOR) {
            if (range == NULL) {
                range = init_version_range();
            }

            if (range == NULL) {
                res = SEMVER_OUT_OF_MEMORY;
                break;
            }

            int compare = v.cmp;
            v.cmp = COMPARE_GREATEROREQUAL;
            first_item = add_version(range, &v, 1);
            if (first_item == NULL) {
                res = SEMVER_OUT_OF_MEMORY;
                break;
            }

            v.prerelease = PRERELEASE_NONE;
            v.cmp = COMPARE_LESS;
            v.patch = 0;

            if (compare == COMPARE_MAJOR) {
                v.minor = 0;
                v.major++;
            } else {
                v.minor++;
            }

            int ok = complete_version_range(first_item, &v);
            if (ok != SEMVER_OK) {
                res = ok;
                break;
            }
        } else {
            if (range == NULL) {
                range = init_version_range();
            }

            if (range == NULL) {
                res = SEMVER_OUT_OF_MEMORY;
                break;
            }

            if (in_range == 1) {
                first_item = add_version(range, &v, 1);
                if (first_item == NULL) {
                    res = SEMVER_OUT_OF_MEMORY;
                    break;
                }
            } else if (in_range == 2) {
                int ok = complete_version_range(first_item, &v);
                if (ok != SEMVER_OK) {
                    res = ok;
                    break;
                }
                in_range = 0;
            } else {
                VersionRange* vr = add_version(range, &v, 0);
                if (vr == NULL) {
                    res = SEMVER_OUT_OF_MEMORY;
                    break;
                }
            }
        }
    }

    if (res != SEMVER_OK && range_size(range) > 0) {
        VersionRange* tmp = range;
        res = SEMVER_OUT_OF_RANGE;
        while (tmp != NULL) {
            int less = SEMVER_OK;
            if (tmp->min_ver != NULL) {
                if (! version_equals(ver, tmp->min_ver)) {
                    less = SEMVER_OUT_OF_RANGE;
                }
            }
            int greater = SEMVER_OK;
            if (tmp->max_ver != NULL) {
                if (! version_equals(ver, tmp->max_ver)) {
                    greater = SEMVER_OUT_OF_RANGE;
                }
            }

            if (less == SEMVER_OK && greater == SEMVER_OK) {
                res = SEMVER_OK;
                break;
            }

            tmp = tmp->next;
        }
    }

    free(tmp_version);
    free_version_range(&range);

    return res;
}


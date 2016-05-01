#include <stdlib.h>

#include "semver.h"
#include "ver_range.h"

VersionRange* init_version_range() {
    VersionRange* range = calloc(1, sizeof(VersionRange));
    return range;
}

void free_version_range(VersionRange** range) {
    if (range == NULL) {
        return;
    }

    VersionRange* next = (*range);
    while (next != NULL) {
        VersionRange* save = next->next;
        free(next->min_ver);
        free(next->max_ver);
        free(next);
        next = save;
    }

    *range = NULL;
}

VersionRange* add_version(VersionRange* range, SemVersion* version, int as_new) {
    if (version == NULL || range == NULL) {
        return NULL;
    }
    if (version->cmp <= COMPARE_NEQUAL || version->cmp >= COMPARE_MAJOR) {
        return NULL;
    }

    VersionRange *last_range = range;

    if (as_new) {
        while (last_range->next != NULL) {
            last_range = last_range->next;
        }
    } else {
        VersionRange* tmp = range;
        while (tmp != NULL) {
            if (tmp->min_ver == NULL && (version->cmp == COMPARE_GREATER || version->cmp == COMPARE_GREATEROREQUAL)) {
                tmp->min_ver = malloc(sizeof(SemVersion));
                if (tmp->min_ver == NULL) {
                    return NULL;
                }
                *tmp->min_ver = *version;
                return tmp;
            } else if (tmp->max_ver == NULL && (version->cmp == COMPARE_LESSOREQUAL || version->cmp == COMPARE_LESS)) {
                tmp->max_ver = malloc(sizeof(SemVersion));
                if (tmp->max_ver == NULL) {
                    return NULL;
                }
                *tmp->max_ver = *version;
                return tmp;
            }

            last_range = tmp;
            tmp = tmp->next;
        }
    }

    VersionRange* new_range = NULL;
    if (last_range->min_ver == NULL && last_range->max_ver == NULL) {
        new_range = last_range;
    } else {
        new_range = calloc(1, sizeof(VersionRange));
        if (new_range == NULL) {
            return NULL;
        }
        last_range->next = new_range;
    }

    SemVersion* clone = malloc(sizeof(SemVersion));
    if (clone == NULL) {
        return NULL;
    }
    *clone = *version;

    if (version->cmp == COMPARE_GREATER || version->cmp == COMPARE_GREATEROREQUAL) {
        new_range->min_ver = clone;
    } else {
        new_range->max_ver = clone;
    }

    return new_range;
}

int complete_version_range(VersionRange* item, SemVersion* version) {
    if (item == NULL) {
        return SEMVER_INVALID_RANGE_ITEM;
    }
    if (version == NULL) {
        return SEMVER_INVALID_VERSION;
    }
    if (version->cmp <= COMPARE_NEQUAL || version->cmp >= COMPARE_MAJOR) {
        return SEMVER_INVALID_VERSION;
    }

    if ((item->min_ver != NULL && (version->cmp == COMPARE_GREATER || version->cmp == COMPARE_GREATEROREQUAL)) ||
           (item->max_ver != NULL && (version->cmp == COMPARE_LESS || version->cmp == COMPARE_LESSOREQUAL))) {
        return SEMVER_ITEM_FULL;
    }

    SemVersion* clone = malloc(sizeof(SemVersion));
    if (clone == NULL) {
        return SEMVER_OUT_OF_MEMORY;
    }
    *clone = *version;

    if (version->cmp == COMPARE_GREATER || version->cmp == COMPARE_GREATEROREQUAL) {
        item->min_ver = clone;
    } else {
        item->max_ver = clone;
    }

    return SEMVER_OK;
}

int range_size(const VersionRange* range) {
    if (range == NULL) {
        return 0;
    }

    const VersionRange* tmp = range;
    int size = 1;
    while (tmp->next != NULL) {
        size++;
        tmp = tmp->next;
    }

    if (tmp->min_ver == NULL && tmp->max_ver == NULL) {
        --size;
    }

    return size;
}

VersionRange* get_range_item(VersionRange* range, int idx) {
    if (range == NULL || idx < 0) {
        return NULL;
    }

    VersionRange* tmp = range;
    while (idx > 0 && tmp != NULL) {
        idx--;
        tmp = tmp->next;
    }

    if (idx == 0) {
        return tmp;
    }

    return NULL;
}


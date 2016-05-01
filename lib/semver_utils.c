#include <stdio.h>

#include "semver.h"
#include "ver_range.h"
#include "semver_utils.h"

void print_range(VersionRange* range) {
    printf("Range: ");
    if (range == NULL) {
        printf("EMPTY\n");
        return;
    }
    printf("%x\n", range);

    int idx = 0;
    VersionRange* tmp = range;
    while (tmp != NULL) {
        printf("Item %d\n", idx);
        printf("    >=");
        if (tmp->min_ver != NULL) {
            printf(" [+]");
        } else {
            printf(" [ ]");
        }
        printf("    <=");
        if (tmp->max_ver != NULL) {
            printf(" [+]");
        } else {
            printf(" [ ]");
        }
        printf("\n");

        tmp = tmp->next;
        idx++;
    }
}

void print_version(SemVersion* version) {
    printf("Version: ");
    if (version == NULL) {
        printf("NULL\n");
        return;
    }
    printf("%x\n", version);

    printf("     Major: %d\n", version->major);
    printf("     Minor: %d\n", version->minor);
    printf("     Patch: %d\n", version->patch);
    if (version->prerelease != PRERELEASE_NONE) {
        printf("     Prerelease: [%d] %s\n", version->prerelease, version->prerelease_str);
    }
    if (*version->build_str != '\0') {
        printf("    Build: %s\n", version->build_str);
    }
}


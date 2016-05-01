#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#include "semver.h"
#include "ver_range.h"

static int is_valid_char(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >='A' && c <= 'Z') || c == '.';
}

static int begins_with(const char* str, char* with) {
    if (str == NULL || with == NULL) {
        return 0;
    }

    return strncmp(str, with, strlen(with)) == 0;
}

static const char* read_number(const char* str, unsigned int* num) {
    if (str == NULL || num == NULL) {
        return str;
    }

    while (*str == ' ') {
        str++;
    }

    if (! isdigit(*str)) {
        *num = INVALID_NUMBER;
        return str;
    }

    *num = (unsigned int)atol(str);

    while (isdigit(*str)) {
        str++;
    }

    return str;
}

static VersionCompare read_compare_method(const char **str) {
        static char* multi[] = {
            "!=", "==", ">=", "<="
        };
        static int multi_val[] = {
            COMPARE_NEQUAL, COMPARE_EQUAL, COMPARE_GREATEROREQUAL, COMPARE_LESSOREQUAL
        };
        static char single[] = { '^', '~', '<', '>', '=' };
        static int single_val[] = { COMPARE_MAJOR, COMPARE_MINOR, COMPARE_LESS, COMPARE_GREATER, COMPARE_EQUAL };

        int mcount = sizeof(multi_val) / sizeof(multi_val[0]);
        int scount = sizeof(single_val) / sizeof(single_val[0]);

        for (int i = 0; i < mcount; i++) {
            if (begins_with(*str, multi[i])) {
                (*str) += 2;
                return multi_val[i];
            }
        }
        for (int i = 0; i < scount; i++) {
            if (**str == single[i]) {
                (*str)++;
                return single_val[i];
            }
        }

        return COMPARE_NONE;
}

int parse_version(const char *str, SemVersion* version) {
    if (str == NULL) {
        return SEMVER_INVALID_VERSION;
    }

    if (version != NULL) {
        memset(version, 0, sizeof(SemVersion));
        version->prerelease = PRERELEASE_NONE;
    }

    /* skip all whitespaces and 'v' */
    while (*str == ' ' || *str == 'v' || *str == 'V') {
        str++;
    }

    /* read compare operation */
    if (! isdigit(*str)) {
        VersionCompare cmp = read_compare_method(&str);
        if (cmp == COMPARE_NONE) {
            return SEMVER_INVALID_MAJOR;
        }
        if (version != NULL) {
            version->cmp = cmp;
        }
    }

    unsigned int parts[PART_COUNT] = { 0, 0, 0 };
    int err[PART_COUNT] = { SEMVER_INVALID_MAJOR, SEMVER_INVALID_MINOR, SEMVER_INVALID_PATCH };

    for (int i=0; i<PART_COUNT; i++) {
        str = read_number(str, &parts[i]);
        if (parts[i] == INVALID_NUMBER) {
            return err[i];
        }
        if (i != PART_COUNT-1) {
           if (*str != '.') {
              return err[i];
           }
           str++;
        }
    }

    if (*str != '\0' && *str != '+' && *str != '-') {
        return SEMVER_INVALID_PATCH;
    }

    if (version != NULL) {
        version->major = parts[0];
        version->minor = parts[1];
        version->patch = parts[2];
    }

    /* read prerelease */
    if (*str == '-') {
        str++;
        static char* prevalues[] = { "alpha", "beta", "rc" };
        static int prereleases[] = { PRERELEASE_ALPHA, PRERELEASE_BETA, PRERELEASE_RC };
        int pre = PRERELEASE_BASIC;
        for (int i = 0; i < sizeof(prereleases) / sizeof(prereleases[0]); i++) {
            if (begins_with(str, prevalues[i])) {
                pre = prereleases[i];
                break;
            }
        }
        if (version != NULL) {
            version->prerelease = pre;
        }

        int idx = 0;
        while (*str != '\0' && *str != '+') {
            if (is_valid_char(*str)) {
                if (version != NULL && idx < MAX_PRERELEASE_LEN - 1) {
                    version->prerelease_str[idx++] = *str;
                }
                str++;
            } else {
                return SEMVER_INVALID_PRERELEASE;
            }
        }
    }

    /* check build */
    if (*str != '\0' && *str != '+') {
        return SEMVER_INVALID_BUILD;
    }

    if (*str != '\0') {
        str++;
        int idx = 0;
        while (is_valid_char(*str)) {
            if (version != NULL && idx < MAX_BUILD_LEN) {
                version->build_str[idx++] = *str;
            }
            str++;
        }

        while (*str != '\0' && *str == ' ') str++;
    }

    return (*str == '\0') ? SEMVER_OK : SEMVER_INVALID_BUILD;
}

static const char* skip_to_first_char(const char* str, char c) {
    if (str == NULL) {
        return str;
    }

    while (*str != '\0') {
        if (*str == c) {
            str++;
            break;
        }
        str++;
    }

    return str;
}

/* Returns which version prerelease is greater:
 * -1 - ver_b prerelease is greater
 *  1 - ver_a prerelease is greater
 *  0 - prereleases are equal
 *  Notes:
 *  A. Empty preprelease(PRERELEASE_NONE) is greater than any
 *  other prerelease type (1.0.0 > 1.0.0-rc)
 *  B. PRERELEASE_BASIC (that does not start with 'beta', 'alpha', or 'rc')
 *  is less than any other kind of preprelease (1.0.0-abc < 1.0.0-alpha)
 *  C. Prerelease can have a few parts separated with '.'. Every part is
 *  processes separately (example: 1.1.0-beta.2.1)
 *  D. If part of prerelease starts with digit then the part is compared as
 *  number and as string otherwise (1.0.0-beta.10 > 1.0.0-beta.01 but
 *  1.0.0-beta.ab < 1.0.0-beta.ef)
 *  E. If one part contains the other part then the longer one is greater
 *  (1.0.0-beta.abcd > 1.0.0-beta.abc)
 */
static int compare_prerelease(const SemVersion* ver_a, const SemVersion* ver_b) {
    if (ver_a == NULL && ver_b == NULL) {
        return 0;
    } else if (ver_a == NULL) {
        return -1;
    } else if (ver_b == NULL) {
        return 1;
    }

    if (ver_a->prerelease > ver_b->prerelease) {
        return 1;
    } else if (ver_a->prerelease < ver_b->prerelease) {
        return -1;
    } else if (ver_a->prerelease == PRERELEASE_NONE) {
        return 0;
    }

    const char *pre_a = ver_a->prerelease_str;
    const char *pre_b = ver_b->prerelease_str;

    /* skip beta, alpha, rc */
    if (ver_a->prerelease != PRERELEASE_BASIC) {
        pre_a = skip_to_first_char(pre_a, '.');
        pre_b = skip_to_first_char(pre_b, '.');
    }

    if (*pre_a == '\0' && *pre_b != '\0') {
        return -1;
    } else if (*pre_a != '\0' && *pre_b == '\0') {
        return 1;
    }

    while (*pre_a != '\0' && *pre_b != '\0') {
        if (isdigit(*pre_a) && isdigit(*pre_b)) {
            int int_a = atoi(pre_a);
            int int_b = atoi(pre_b);
            if (int_a < int_b) {
                return -1;
            } else if (int_a > int_b) {
                return 1;
            }
        } else {
            int len_a = 0;
            int len_b = 0;
            const char* save_a = pre_a;
            const char* save_b = pre_b;

            while (*save_a != '\0' && *save_a != '.') {
                len_a++;
                save_a++;
            }
            while (*save_b != '\0' && *save_b != '.') {
                len_b++;
                save_b++;
            }

            int to_cmp = (len_a > len_b) ? len_b : len_a;
            int res = strncmp(pre_a, pre_b, to_cmp);
            if (res != 0) {
                return res;
            }
        }

        pre_a = skip_to_first_char(pre_a, '.');
        pre_b = skip_to_first_char(pre_b, '.');
    }

    if (*pre_a =='\0' && *pre_b != '\0') {
        return 1;
    } else if (*pre_a !='\0' && *pre_b == '\0') {
        return -1;
    }

    return 0;
}

int compare_versions(const SemVersion* ver_a, const SemVersion* ver_b) {
    if (ver_a == NULL && ver_b == NULL) {
        return 0;
    } else if (ver_a == NULL) {
        return -1;
    } else if (ver_b == NULL) {
        return 1;
    }

    if (ver_a->major > ver_b->major) {
        return 1;
    } else if (ver_a->major < ver_b->major) {
        return -1;
    }

    if (ver_a->minor > ver_b->minor) {
        return 1;
    } else if (ver_a->minor < ver_b->minor) {
        return -1;
    }

    if (ver_a->patch > ver_b->patch) {
        return 1;
    } else if (ver_a->patch < ver_b->patch) {
        return -1;
    }

    return compare_prerelease(ver_a, ver_b);
}

int version_equals(const SemVersion* ver_a, const SemVersion* ver_b) {
    if (ver_a == NULL && ver_b == NULL) {
        return 1;
    } else if (ver_a == NULL) {
        return 0;
    } else if (ver_b == NULL) {
        return 0;
    }

    if (ver_b->cmp == COMPARE_EQUAL || ver_b->cmp == COMPARE_NONE) {
        return compare_versions(ver_a, ver_b) == 0;
    } else if (ver_b->cmp == COMPARE_NEQUAL) {
        return compare_versions(ver_a, ver_b) != 0;
    } else if (ver_b->cmp == COMPARE_GREATEROREQUAL) {
        int r = compare_versions(ver_a, ver_b);
        return r  >= 0;
    } else if (ver_b->cmp == COMPARE_GREATER) {
        int r = compare_versions(ver_a, ver_b);
        return r > 0;
    } else if (ver_b->cmp == COMPARE_LESSOREQUAL) {
        int r = compare_versions(ver_a, ver_b);
        return r <= 0;
    } else if (ver_b->cmp == COMPARE_LESS) {
        int r = compare_versions(ver_a, ver_b);
        return r < 0;
    } else if (ver_b->cmp == COMPARE_MAJOR) {
        return ver_a->major == ver_b->major;
    } else if (ver_b->cmp == COMPARE_MINOR) {
        return ver_a->major == ver_b->major && ver_a->minor == ver_b->minor;
    }

    return 0;
}


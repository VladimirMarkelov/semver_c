#include <stdio.h>
#include <string.h>
#include "semver.h"
#include "semver_check.h"
#include "ver_range.h"
#include "semver_utils.h"

#include "unittest.h"

int tests_run = 0;

static char* test_range() {
    SemVersion ver;
    parse_version("1.2.3", &ver);

    VersionRange* range = init_version_range();
    mu_assert("Range intialized", range != NULL);
    mu_assert("Range empty", range_size(range) == 0);

    add_version(range, &ver, 0);
    mu_assert("Range empty after adding invalid item", range_size(range) == 0);

    ver.cmp = COMPARE_GREATER;
    add_version(range, &ver, 0);
    int sz = range_size(range);
    mu_assert("Range after adding correct item", sz == 1);
    add_version(range, &ver, 0);
    sz = range_size(range);
    mu_assert("Range after adding another correct item", sz == 2);

    VersionRange* r = get_range_item(range, 0);
    mu_assert("First item min only", r != NULL && r->min_ver != NULL && r->max_ver == NULL);
    r = get_range_item(range, 1);
    mu_assert("Second item min only", r != NULL && r->min_ver != NULL && r->max_ver == NULL);
    r = get_range_item(range, 2);
    mu_assert("No third item", r == NULL);

    ver.cmp = COMPARE_LESSOREQUAL;
    add_version(range, &ver, 0);
    sz = range_size(range);
    mu_assert("Range after adding another pair", sz == 2);
    r = get_range_item(range, 0);
    mu_assert("First item full only", r != NULL && r->min_ver != NULL && r->max_ver != NULL);
    r = get_range_item(range, 1);
    mu_assert("Second item min only", r != NULL && r->min_ver != NULL && r->max_ver == NULL);
    r = get_range_item(range, 2);
    mu_assert("No third item", r == NULL);

    ver.cmp = COMPARE_GREATER;
    add_version(range, &ver, 0);
    sz = range_size(range);
    mu_assert("Range after adding extra min", sz == 3);
    ver.cmp = COMPARE_LESSOREQUAL;

    VersionRange* forth = add_version(range, &ver, 1);
    sz = range_size(range);
    mu_assert("Range after adding as new pair", sz == 4);
    r = get_range_item(range, 0);
    mu_assert("First item full only", r != NULL && r->min_ver != NULL && r->max_ver != NULL);
    r = get_range_item(range, 1);
    mu_assert("Second item min only", r != NULL && r->min_ver != NULL && r->max_ver == NULL);
    r = get_range_item(range, 3);
    mu_assert("Forth item maxonly", r != NULL && r->min_ver == NULL && r->max_ver != NULL);

    ver.cmp = COMPARE_LESS;
    int ok = complete_version_range(forth, &ver);
    mu_assert("No way to complete with the same condition", ok == SEMVER_ITEM_FULL);
    ver.cmp = COMPARE_GREATEROREQUAL;
    ok = complete_version_range(forth, &ver);
    mu_assert("Complete with the opposite condition", ok == SEMVER_OK);
    ok = complete_version_range(forth, &ver);
    mu_assert("No way to complete with the same condition 2", ok == SEMVER_ITEM_FULL);
    r = get_range_item(range, 3);
    mu_assert("Forth item full", r != NULL && r->min_ver != NULL && r->max_ver != NULL);
    sz = range_size(range);
    mu_assert("Size did not change", sz == 4);

    ver.cmp = COMPARE_LESS;
    add_version(range, &ver, 0);
    sz = range_size(range);
    mu_assert("Size no chage 2", sz == 4);
    r = get_range_item(range, 0);
    mu_assert("First item full", r != NULL && r->min_ver != NULL && r->max_ver != NULL);
    r = get_range_item(range, 1);
    mu_assert("Second item full", r != NULL && r->min_ver != NULL && r->max_ver != NULL);
    r = get_range_item(range, 2);
    mu_assert("Third item min only", r != NULL && r->min_ver != NULL && r->max_ver == NULL);

    free_version_range(&range);
    mu_assert("Range empty", range_size(range) == 0);

    return 0;
}


static char* all_tests() {
    mu_run_test("Parsing range cases", test_range);
    return 0;
}

int main (int argc, char** argv) {
    char *result = all_tests();
     if (result != 0) {
         printf("%s\n", result);
     }
     else {
         printf("ALL TESTS PASSED\n");
     }
     printf("Tests run: %d\n", tests_run);

     return result != 0;
}

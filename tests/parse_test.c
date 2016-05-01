#include <stdio.h>
#include <string.h>
#include "semver.h"
#include "semver_check.h"
#include "ver_range.h"
#include "semver_utils.h"
#include "unittest.h"

int tests_run = 0;

static char* test_parse_version() {
    SemVersion ver;
    int res = parse_version("y.2.3-beta.31+345", &ver);
    mu_assert("Parsing y.2.3-beta.31+345", res == SEMVER_INVALID_MAJOR);
    res = parse_version("1.j.3-beta.31+345", &ver);
    mu_assert("Parsing 1.j.3-beta.31+345", res == SEMVER_INVALID_MINOR);
    res = parse_version("1.2.r-beta.31+345", &ver);
    mu_assert("Parsing 1.2.r-beta.31+345", res == SEMVER_INVALID_PATCH);
    res = parse_version("1.2.3-b=ta.31+345", &ver);
    mu_assert("Parsing 1.2.3-b=ta.31+345", res == SEMVER_INVALID_PRERELEASE);
    res = parse_version("1.2.3-beta.31+3$5", &ver);
    mu_assert("Parsing 1.2.3-beta.31+3$5", res == SEMVER_INVALID_BUILD);
    res = parse_version("  1.2.3-beta.31+a345  ", &ver);
    mu_assert("Parsing <  1.2.3-beta.31+a345  >", res == SEMVER_OK);
    res = parse_version("1.2.3", &ver);
    mu_assert("Parsing 1.2.3", res == SEMVER_OK);
    res = parse_version("1.2.3-beta.20a", &ver);
    mu_assert("Parsing 1.2.3-beta.20a", res == SEMVER_OK);

    res = parse_version("1.2.3-beta.31+345", &ver);
    mu_assert("Parsing 1.2.3-beta.31+345", res == SEMVER_OK);
    mu_assert("Major == 1", ver.major == 1);
    mu_assert("Minor == 2", ver.minor == 2);
    mu_assert("Patch == 3", ver.patch == 3);
    mu_assert("Prerelease == Beta", ver.prerelease == PRERELEASE_BETA);
    mu_assert("Prerelease == beta.31", strcmp(ver.prerelease_str, "beta.31") == 0);
    mu_assert("Build == 345", strcmp(ver.build_str, "345") == 0);
    mu_assert("Compare == none", ver.cmp == COMPARE_NONE);

    return 0;
}

static char* test_parse_compare() {
    SemVersion ver;
    int res = parse_version(" > 1.2.3-beta.31", &ver);
    mu_assert("Greater than 1.2.3-beta.31", res == SEMVER_OK && ver.cmp == COMPARE_GREATER);
    res = parse_version(" >= 1.2.3-beta.31", &ver);
    mu_assert("Greater or equal than 1.2.3-beta.31", res == SEMVER_OK && ver.cmp == COMPARE_GREATEROREQUAL);
    res = parse_version(" < 1.2.3-beta.31", &ver);
    mu_assert("Less than 1.2.3-beta.31", res == SEMVER_OK && ver.cmp == COMPARE_LESS);
    res = parse_version(" <= 1.2.3-beta.31", &ver);
    mu_assert("Less or equal than 1.2.3-beta.31", res == SEMVER_OK && ver.cmp == COMPARE_LESSOREQUAL);
    res = parse_version(" == 1.2.3-beta.31", &ver);
    mu_assert("Equal to 1.2.3-beta.31", res == SEMVER_OK && ver.cmp == COMPARE_EQUAL);
    res = parse_version(" =1.2.3-beta.31", &ver);
    mu_assert("Equal to =1.2.3-beta.31", res == SEMVER_OK && ver.cmp == COMPARE_EQUAL);
    res = parse_version("!=1.2.3-beta.31", &ver);
    mu_assert("Not equal to 1.2.3-beta.31", res == SEMVER_OK && ver.cmp == COMPARE_NEQUAL);
    res = parse_version(" ~ 1.2.3-beta.31", &ver);
    mu_assert("Compare minor 1.2.3-beta.31", res == SEMVER_OK && ver.cmp == COMPARE_MINOR);
    res = parse_version("^1.2.3-beta.31", &ver);
    mu_assert("Compare major 1.2.3-beta.31", res == SEMVER_OK && ver.cmp == COMPARE_MAJOR);
    res = parse_version("-1.2.3-beta.31", &ver);
    mu_assert("Compare fail 1.2.3-beta.31", ver.cmp == COMPARE_NONE);
    mu_assert("Compare fail 2", res == SEMVER_INVALID_MAJOR);

    return 0;
}

static char* test_parse_prerelease() {
    SemVersion ver;
    int res = parse_version("1.2.3-beta.31+345", &ver);
    mu_assert("Prerelease BETA", res == SEMVER_OK && ver.prerelease == PRERELEASE_BETA);
    res = parse_version("1.2.3-alpha.31+345", &ver);
    mu_assert("Prerelease ALPHA", res == SEMVER_OK && ver.prerelease == PRERELEASE_ALPHA);
    res = parse_version("1.2.3-rc.beta+345", &ver);
    mu_assert("Prerelease release candidate", res == SEMVER_OK && ver.prerelease == PRERELEASE_RC);
    res = parse_version("1.2.3-01837+345", &ver);
    mu_assert("Prerelease basic", res == SEMVER_OK && ver.prerelease == PRERELEASE_BASIC);
    res = parse_version("1.2.3+345", &ver);
    mu_assert("Prerelease NONE", res == SEMVER_OK && ver.prerelease == PRERELEASE_NONE);
    res = parse_version("1.2.5", &ver);
    mu_assert("Prerelease NONE 2", res == SEMVER_OK && ver.prerelease == PRERELEASE_NONE);

    return 0;
}

static char* test_parse_empty() {
    SemVersion ver;
    int res = parse_version("1.2.3-beta.31+345", NULL);
    mu_assert("No storage", res == SEMVER_OK);
    res = parse_version("", &ver);
    mu_assert("Empty version", res == SEMVER_INVALID_MAJOR);
    res = parse_version(NULL, NULL);
    mu_assert("NULL version", res == SEMVER_INVALID_VERSION);
    res = parse_version("1.2.3-beta.31.78.somelongtext.7818.after.alpha+345", &ver);
    mu_assert("Long prerelease", res == SEMVER_OK);
    mu_assert("Long prerelease check", strcmp(ver.prerelease_str, "beta.31.78.some") == 0);

    return 0;
}

static char* test_compare_versions() {
    SemVersion ver1, ver2;
    parse_version("1.12.3-beta.31+345", &ver1);

    parse_version("1.40.1-beta.31+345", &ver2);
    int res = compare_versions(&ver1, &ver2);
    mu_assert("Compare minor", res < 0);

    parse_version("1.10.1-beta.31+345", &ver2);
    res = compare_versions(&ver1, &ver2);
    mu_assert("Compare minor 2", res > 0);

    parse_version("1.12.3-beta.31+645", &ver2);
    res = compare_versions(&ver1, &ver2);
    mu_assert("Compare equal", res == 0);

    parse_version("1.12.3-beta.32+645", &ver2);
    res = compare_versions(&ver1, &ver2);
    mu_assert("Compare beta", res < 0);

    parse_version("1.12.3-beta.22+645", &ver2);
    res = compare_versions(&ver1, &ver2);
    mu_assert("Compare beta 2", res > 0);

    parse_version("3.12.3-beta.32+645", &ver2);
    res = compare_versions(&ver1, &ver2);
    mu_assert("Compare major", res < 0);

    parse_version("0.12.3-beta.32+645", &ver2);
    res = compare_versions(&ver1, &ver2);
    mu_assert("Compare major 2", res > 0);

    parse_version("1.12.3-rc.32+645", &ver2);
    res = compare_versions(&ver1, &ver2);
    mu_assert("Compare beta to RC", res < 0);

    parse_version("1.12.3-alpha.32+645", &ver2);
    res = compare_versions(&ver1, &ver2);
    mu_assert("Compare beta to alpha", res > 0);

    parse_version("1.12.3+645", &ver2);
    res = compare_versions(&ver1, &ver2);
    mu_assert("Compare beta to final", res < 0);

    parse_version("1.12.3", &ver2);
    res = compare_versions(&ver1, &ver2);
    mu_assert("Compare beta to final 2", res < 0);

    parse_version("1.12.3-beta.31a", &ver1);
    parse_version("1.12.3-beta.31b", &ver2);
    res = compare_versions(&ver1, &ver2);
    mu_assert("Compare beta.31a to beta31b", res == 0);

    parse_version("1.12.3-beta.31.a", &ver1);
    parse_version("1.12.3-beta.31.b", &ver2);
    res = compare_versions(&ver1, &ver2);
    mu_assert("Compare beta.31.a to beta31.b", res < 0);

    return 0;
}

static char* test_check_versions() {
    SemVersion ver1;
    parse_version("1.12.3-beta.31+345", &ver1);

    int res = check_version(&ver1, "1.13.3");
    mu_assert("Check one version", res == SEMVER_OUT_OF_RANGE);
    res = check_version(&ver1, "1.12.3-beta.31+345");
    mu_assert("Check one version 2", res == SEMVER_OK);
    res = check_version(&ver1, "1.12.3-beta.31+345,,");
    mu_assert("Check one version 2.1: equal", res == SEMVER_OK);
    res = check_version(&ver1, "*");
    mu_assert("Check any version: equal", res == SEMVER_OK);
    res = check_version(&ver1, "");
    mu_assert("Check empty version: equal", res == SEMVER_OUT_OF_RANGE);


    res = check_version(&ver1, ">1.12.3");
    mu_assert("Check one version 3: greater", res == SEMVER_OUT_OF_RANGE);
    res = check_version(&ver1, ">1.11.3");
    mu_assert("Check one version 4: greater", res == SEMVER_OK);
    res = check_version(&ver1, ">=1.12.3");
    mu_assert("Check one version 5: greater", res == SEMVER_OUT_OF_RANGE);
    res = check_version(&ver1, ">=1.12.3-beta.31");
    mu_assert("Check one version 6: greater", res == SEMVER_OK);
    res = check_version(&ver1, "=1.12.3-beta.31");
    mu_assert("Check one version 7: equal", res == SEMVER_OK);
    res = check_version(&ver1, "==1.12.3-beta.31");
    mu_assert("Check one version 8: equal", res == SEMVER_OK);
    res = check_version(&ver1, "!=1.12.3-beta.31");
    mu_assert("Check one version 9: not equal", res == SEMVER_OUT_OF_RANGE);
    res = check_version(&ver1, "!=1.12.9-beta.31");
    mu_assert("Check one version 10: not equal", res == SEMVER_OK);

    res = check_version(&ver1, ">=1.11.0,<=1.14.1");
    mu_assert("Check two version interval", res == SEMVER_OK);
    res = check_version(&ver1, "<=1.11.0,>=1.14.1");
    mu_assert("Check two version interval 2", res == SEMVER_OUT_OF_RANGE);

    res = check_version(&ver1, "^1.10.12");
    mu_assert("Check major version", res == SEMVER_OK);
    res = check_version(&ver1, "^1.12.12");
    mu_assert("Check major version", res == SEMVER_OK);
    res = check_version(&ver1, "^1.12.12,>=1.12.3");
    mu_assert("Check major version RC", res == SEMVER_OK);

    res = check_version(&ver1, "~1.10.12");
    mu_assert("Check minor version", res == SEMVER_OUT_OF_RANGE);
    res = check_version(&ver1, "~1.12.12");
    mu_assert("Check minor version 2", res == SEMVER_OK);
    res = check_version(&ver1, "~1.14.12");
    mu_assert("Check minor version 3", res == SEMVER_OUT_OF_RANGE);

    res = check_version(&ver1, "1.11.1,1.0.1,1.12.3-beta.31+345");
    mu_assert("Check three versions", res == SEMVER_OK);
    res = check_version(&ver1, "1.11.1,1.0.1,1.12.6-beta.31+345");
    mu_assert("Check three versions 2", res == SEMVER_OUT_OF_RANGE);

    res = check_version(&ver1, "1.10.1 - 1.11.0");
    mu_assert("Check range 1", res == SEMVER_OUT_OF_RANGE);
    res = check_version(&ver1, "1.10.1 - 1.17.0");
    mu_assert("Check range 2", res == SEMVER_OK);
    res = check_version(&ver1, "1.12.3-alpha - 1.12.3-beta.40");
    mu_assert("Check range 3", res == SEMVER_OK);
    res = check_version(&ver1, "1.12.3-alpha - 1.12.3-beta.4");
    mu_assert("Check range 4", res == SEMVER_OUT_OF_RANGE);
    res = check_version(&ver1, "1.12.3-beta - 1.12.3-rc.1");
    mu_assert("Check range 5", res == SEMVER_OK);

    res = check_version(&ver1, "1.16.1 - 1.17.0,1.12.3-beta.31+345");
    mu_assert("Check range 3", res == SEMVER_OK);
    res = check_version(&ver1, "1.16.1 - 1.17.0,^1.20.3");
    mu_assert("Check range 4", res == SEMVER_OK);
    res = check_version(&ver1, "^1.20.3,1.16.1 - 1.17.0,");
    mu_assert("Check range 4.1", res == SEMVER_OK);
    res = check_version(&ver1, "1.16.1 - 1.17.0,~1.12.8");
    mu_assert("Check range 5", res == SEMVER_OK);
    res = check_version(&ver1, "1.10.3 - 1.13.9,1.16.1 - 1.17.0");
    mu_assert("Check range 6", res == SEMVER_OK);
    res = check_version(&ver1, "1.16.1 - 1.17.0,1.10.3 - 1.13.9");
    mu_assert("Check range 7", res == SEMVER_OK);

    res = check_version(&ver1, "1.12.6 - 1.17.0");
    mu_assert("Corner case 1", res == SEMVER_OUT_OF_RANGE);
    res = check_version(&ver1, "1.10.6 - 1.12.6");
    mu_assert("Corner case 2", res == SEMVER_OK);

    res = check_version(&ver1, "1.16.1 - 1.17.0,0.10.3 - 0.13.9,1.12.5 - 1.12.7");
    mu_assert("Check three range 1", res == SEMVER_OUT_OF_RANGE);
    res = check_version(&ver1, "1.16.1 - 1.17.0,0.10.3 - 0.13.9,1.12.1 - 1.12.7");
    mu_assert("Check three range 2", res == SEMVER_OK);
    res = check_version(&ver1, "1.16.1 - 1.17.0,0.10.3 - 5.13.9,1.2.15 - 1.2.70");
    mu_assert("Check three range 3", res == SEMVER_OK);

    return 0;
}

static char* all_tests() {
    mu_run_test("Parsing corner cases", test_parse_empty);
    mu_run_test("Parsing versions", test_parse_version);
    mu_run_test("Parsing compare", test_parse_compare);
    mu_run_test("Parsing prerelease", test_parse_prerelease);
    mu_run_test("Compare versions", test_compare_versions);
    mu_run_test("Check versions", test_check_versions);
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

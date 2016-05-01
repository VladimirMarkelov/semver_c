#define mu_assert(name, test) do {\
                printf("    Checking %s .. ", name); \
                    if (!(test)) { printf("FAIL\n"); return name;} \
                    else printf("OK\n"); \
                } while (0)
#define mu_run_test(name, test) do { printf("Running %s...\n", name); \
                                char *message = test(); tests_run++; \
                                if (message) return message; } while (0)


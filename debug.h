
#ifndef DEBUGH
#define DEBUGH

#define print(format, ...) \
    printf("%s\t| %s:%d\t| " format "\n", __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)

#endif

#ifndef DATS_H
#define DATS_H
#define DATS_LOG(...) {\
    fprintf(stderr, "[%s:%d] ", __FILE__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); \
 }

#endif /* DATS_H */

#define DSYNTH_LOG(...)                                                        \
  {                                                                            \
    fprintf(stderr, "[%s:%d] ", __FILE__, __LINE__);                           \
    fprintf(stderr, __VA_ARGS__);                                              \
  }

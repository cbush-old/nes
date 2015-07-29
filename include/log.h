#ifndef LOG_H
#define LOG_H

#include <cstdio>

#define log_(...) printf("%s.%d:%s\t", __FILE__, __LINE__, __FUNCTION__); printf(__VA_ARGS__); printf("\n");

#define logv(...) log_(__VA_ARGS__);
#define logi(...) log_(__VA_ARGS__);
#define logw(...) log_(__VA_ARGS__);
#define loge(...) log_(__VA_ARGS__);

#endif

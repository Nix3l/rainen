#ifndef LOG_H
#define LOG_H

#include <stdio.h>

// TODO(nix3l): log file support in debug mode

#define LOG(...) do { fprintf(stdout, "(%s:[%s]:%u): ", __FILE__, __PRETTY_FUNCTION__, __LINE__); fprintf(stdout,  __VA_ARGS__); } while(0)
#define LOG_ERR(...) do { fprintf(stderr, "ERR (%s:[%s]:%u): ", __FILE__, __PRETTY_FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__); } while(0)
#define LOG_WARN(...) do { fprintf(stderr, "WARN (%s:[%s]:%u): ", __FILE__, __PRETTY_FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__); } while(0)

#endif

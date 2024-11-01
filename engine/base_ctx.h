#ifndef BASE_CTX_H
#define BASE_CTX_H

// NOTE(nix3l): not including arm, arm64 at the moment, may add later
#if defined(__clang__)
#   define COMPILER_CLANG   1
#elif defined(__GNUC__)
#   define COMPILER_GNU     1
#elif defined(_MSC_VER)
#   define COMPILER_MSVC    1
#   if defined(_M_AMD64)
#       define ARCH_x64     1
#   endif
#else
#   error "no clue what compiler this is"
#endif

// because who cares about mac anyway
#if defined(__gnu_linux__)
#   define OS_LINUX         1
#elif defined(_WIN32)
#   define OS_WINDOWS       1
#else
#   error "no clue what os this is"
#endif

#if defined(COMPILER_CLANG) || defined (COMPILER_GNU)
#   if defined(__amd64__)
#       define ARCH_X64     1
#   elif defined(__i386__)
#       define ARCH_X86     1
#   endif
#elif defined(COMPILER_MSVC)
#   if defined(_M_AMD64)
#       define ARCH_X64     1
#   elif defined(_M_IX86)
#       define ARCH_X86     1
#   endif
#endif

#if !defined(ARCH_X64) && !defined(ARCH_X86)
#   error "no clue what architecture this is"
#endif

#if !defined(COMPILER_CLANG)
#   define COMPILER_CLANG   0
#endif
#if !defined(COMPILER_GNU)
#   define COMPILER_GNU     0
#endif
#if !defined(COMPILER_MSVC)
#   define COMPILER_MSVC    0
#endif
#if !defined(OS_LINUX)
#   define OS_LINUX         0
#endif
#if !defined(OS_WINDOWS)
#   define OS_WINDOWS       0
#endif
#if !defined(ARCH_X64)
#   define ARCH_X64         0
#endif
#if !defined(ARCH_X86)
#   define ARCH_X86         0
#endif

#endif

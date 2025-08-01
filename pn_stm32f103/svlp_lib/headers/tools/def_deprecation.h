#ifndef DEF_DEPRECATION_H
#define DEF_DEPRECATION_H
//Предупреждение для устаревших (deprecated) функций
// Поддерживает GCC/CLANG/MSVC

#if defined(__GNUC__) || defined(__clang__)
#define DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED __declspec(deprecated)
#else
#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#define DEPRECATED
#endif
#endif // DEF_DEPRECATION_H

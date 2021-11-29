#pragma once

#define NO_COPY_CLASS(className)          \
    className(const className&) = delete; \
    className& operator=(const className&) = delete;

template<typename T>
inline int dcmp(T a, T b) {
    const T eps = 1e-8;
    T delta = a - b;
    if (delta > eps) return 1;
    if (delta < -eps) return -1;
    return 0;
}
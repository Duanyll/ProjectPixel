#pragma once

#define NO_COPY_CLASS(className)          \
    className(const className&) = delete; \
    className& operator=(const className&) = delete;

template <typename T>
inline int dcmp(T a, T b) {
    const T eps = 1e-5;
    T delta = a - b;
    if (delta > eps) return 1;
    if (delta < -eps) return -1;
    return 0;
}

typedef decltype(std::chrono::steady_clock::now()) TimeStamp;
typedef std::chrono::steady_clock::duration TimeDuration;

inline float to_float_duration(TimeDuration duration) {
    using namespace std::chrono;
    return duration / 1ms * 0.001;
}

template <class T>
inline typename std::enable_if<std::is_enum<T>::value, T>::type operator~(T a) {
    return (T) ~(int)a;
}
template <class T>
inline typename std::enable_if<std::is_enum<T>::value, T>::type operator|(T a,
                                                                          T b) {
    return (T)((int)a | (int)b);
}
template <class T>
inline typename std::enable_if<std::is_enum<T>::value, T>::type operator&(T a,
                                                                          T b) {
    return (T)((int)a & (int)b);
}
template <class T>
inline typename std::enable_if<std::is_enum<T>::value, T>::type operator^(T a,
                                                                          T b) {
    return (T)((int)a ^ (int)b);
}
template <class T>
inline typename std::enable_if<std::is_enum<T>::value, T>::type& operator|=(
    T& a, T b) {
    return (T&)((int&)a |= (int)b);
}
template <class T>
inline typename std::enable_if<std::is_enum<T>::value, T>::type& operator&=(
    T& a, T b) {
    return (T&)((int&)a &= (int)b);
}
template <class T>
inline typename std::enable_if<std::is_enum<T>::value, T>::type& operator^=(
    T& a, T b) {
    return (T&)((int&)a ^= (int)b);
}
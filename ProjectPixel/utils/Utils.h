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
inline typename std::enable_if<std::is_enum<T>::value, bool>::type operator&(T a,
                                                                          T b) {
    return (bool)((int)a & (int)b);
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

inline std::string generate_unique_id(const std::string& prefix = "id") {
    static int counter = 0;
    return std::format("{}-{}", prefix, ++counter);
}

template <class... Durations, class DurationIn>
std::tuple<Durations...> break_down_durations(DurationIn d) {
    std::tuple<Durations...> retval;
    using discard = int[];
    (void)discard{0, (void(((std::get<Durations>(retval) =
                                 std::chrono::duration_cast<Durations>(d)),
                            (d -= std::chrono::duration_cast<DurationIn>(
                                 std::get<Durations>(retval))))),
                      0)...};
    return retval;
}

inline std::string duration_to_msms(TimeDuration d) {
    auto [m, s, ms] =
        break_down_durations<std::chrono::minutes, std::chrono::seconds,
                             std::chrono::milliseconds>(d);
    return std::format("{:0>2}:{:0>2}:{:0>3}", m.count(), s.count(), ms.count());
}
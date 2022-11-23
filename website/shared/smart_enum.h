template<typename T = char>
struct SmartEnum
{
    T v;

    constexpr SmartEnum(T x = 0) : v(x) {}
    constexpr operator T() const { return v; }
};
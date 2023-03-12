#pragma once
template <class T>
constexpr double Log2(T t) {
	return t <= 2 ? t / 2.0 : 1 + Log2(t / 2.0);
}

template<class Float>
constexpr auto Ceil(Float x) {
	auto value = (int)x;
	return value == x ? value : value +1;
}

template<class Value>
constexpr Value Pow(Value base, int exponent) {
	if (exponent == 0 || base == 0)
		return 0;
	if (exponent == 1)
		return base;
	return base * Pow(base, exponent -1);
}
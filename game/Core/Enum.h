#pragma once

namespace Enum
{
	template<typename T> bool HasAllFlags(T state, T check);
	template<typename T> bool HasAnyFlags(T state, T check);
	template<typename T> T SetFlags(T state, T set, bool value);
	template<typename T> T SetFlags(T state, T set);
	template<typename T> T ClearFlags(T state, T clear);
	template<typename T> T CombineFlags(T f1, T f2);
	template<typename T> T CombineFlags(T f1, T f2, T f3);
	template<typename T> T CombineFlags(T f1, T f2, T f3, T f4);
}

template<typename T>
bool Enum::HasAllFlags(T state, T check)
{
	typedef std::underlying_type_t<T> TI;
	return ((TI)state & (TI)check) == (TI)check;
}

template<typename T>
bool Enum::HasAnyFlags(T state, T check)
{
	typedef std::underlying_type_t<T> TI;
	return ((TI)state & (TI)check) != (TI)0;
}

template<typename T>
T Enum::SetFlags(T state, T set, bool value)
{
	typedef std::underlying_type_t<T> TI;
	const TI istate = (TI)state;
	const TI iset = (TI)set;
	const TI ival = (TI)value;

	return (T)((istate & ~iset) | (iset * ival));
}

template<typename T>
T Enum::SetFlags(T state, T set)
{
	typedef std::underlying_type_t<T> TI;
	return (T)((TI)state | (TI)set);
}

template<typename T>
T Enum::ClearFlags(T state, T clear)
{
	typedef std::underlying_type_t<T> TI;
	return (T)((TI)state & ~(TI)clear);
}

template<typename T>
T Enum::CombineFlags(T f1, T f2)
{
	typedef std::underlying_type_t<T> TI;
	return (T)((TI)f1 | (TI)f2);
}

template<typename T>
T Enum::CombineFlags(T f1, T f2, T f3)
{
	typedef std::underlying_type_t<T> TI;
	return (T)((TI)f1 | (TI)f2 | (TI)f3);
}

template<typename T>
T Enum::CombineFlags(T f1, T f2, T f3, T f4)
{
	typedef std::underlying_type_t<T> TI;
	return (T)((TI)f1 | (TI)f2 | (TI)f3 | (TI)f4);
}

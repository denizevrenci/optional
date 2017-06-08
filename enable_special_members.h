#pragma once

#include <type_traits>

namespace util {

template <bool SWITCH, typename Tag = void>
struct EnableDefaultConstructor {
};

template <typename T>
using DefaultConstructibleBase = EnableDefaultConstructor<std::is_default_constructible<T>::value, T>;

template <bool SWITCH, typename Tag = void>
struct EnableDestructor {
};

template <typename T>
using DestructibleBase = EnableDestructor<std::is_destructible<T>::value, T>;

template <bool COPY, bool COPY_ASSN, bool MOVE, bool MOVE_ASSN, typename Tag = void>
struct EnableCopyMove {
};

template <typename T>
using CopyMoveBase = EnableCopyMove<std::is_copy_constructible<T>::value
	, std::is_copy_assignable<T>::value
	, std::is_move_constructible<T>::value
	, std::is_move_assignable<T>::value
	, T>;

template <bool DEF_CON, bool DEST
	, bool COPY, bool COPY_ASSN, bool MOVE, bool MOVE_ASSN, typename Tag = void>
struct EnableSpecialMembers
	: private EnableDefaultConstructor<DEF_CON, Tag>
	, private EnableDestructor<DEST, Tag>
	, private EnableCopyMove<COPY, COPY_ASSN, MOVE, MOVE_ASSN, Tag> {
};

template <typename T>
using SpecialMembersBase = EnableSpecialMembers<std::is_default_constructible<T>::value
	, std::is_destructible<T>::value
	, std::is_copy_constructible<T>::value
	, std::is_copy_assignable<T>::value
	, std::is_move_constructible<T>::value
	, std::is_move_assignable<T>::value
	, T>;

template <typename Tag>
struct EnableDefaultConstructor<false, Tag> {
	constexpr EnableDefaultConstructor() noexcept = delete;
};

template <typename Tag>
struct EnableDestructor<false, Tag> {
	~EnableDestructor() noexcept = delete;
};

template <typename Tag>
struct EnableCopyMove<false, false, false, false, Tag> {
	constexpr EnableCopyMove() noexcept = default;
	constexpr EnableCopyMove(const EnableCopyMove&) noexcept = delete;
	constexpr EnableCopyMove(EnableCopyMove&&) noexcept = delete;
	EnableCopyMove& operator =(const EnableCopyMove&) noexcept = delete;
	EnableCopyMove& operator =(EnableCopyMove&&) noexcept = delete;
};

template <typename Tag>
struct EnableCopyMove<false, false, false, true, Tag> {
	constexpr EnableCopyMove() noexcept = default;
	constexpr EnableCopyMove(const EnableCopyMove&) noexcept = delete;
	constexpr EnableCopyMove(EnableCopyMove&&) noexcept = delete;
	EnableCopyMove& operator =(const EnableCopyMove&) noexcept = delete;
	EnableCopyMove& operator =(EnableCopyMove&&) noexcept = default;
};

template <typename Tag>
struct EnableCopyMove<false, false, true, false, Tag> {
	constexpr EnableCopyMove() noexcept = default;
	constexpr EnableCopyMove(const EnableCopyMove&) noexcept = delete;
	constexpr EnableCopyMove(EnableCopyMove&&) noexcept = default;
	EnableCopyMove& operator =(const EnableCopyMove&) noexcept = delete;
	EnableCopyMove& operator =(EnableCopyMove&&) noexcept = delete;
};

template <typename Tag>
struct EnableCopyMove<false, false, true, true, Tag> {
	constexpr EnableCopyMove() noexcept = default;
	constexpr EnableCopyMove(const EnableCopyMove&) noexcept = delete;
	constexpr EnableCopyMove(EnableCopyMove&&) noexcept = default;
	EnableCopyMove& operator =(const EnableCopyMove&) noexcept = delete;
	EnableCopyMove& operator =(EnableCopyMove&&) noexcept = default;
};

template <typename Tag>
struct EnableCopyMove<false, true, false, false, Tag> {
	constexpr EnableCopyMove() noexcept = default;
	constexpr EnableCopyMove(const EnableCopyMove&) noexcept = delete;
	constexpr EnableCopyMove(EnableCopyMove&&) noexcept = delete;
	EnableCopyMove& operator =(const EnableCopyMove&) noexcept = default;
	EnableCopyMove& operator =(EnableCopyMove&&) noexcept = delete;
};

template <typename Tag>
struct EnableCopyMove<false, true, false, true, Tag> {
	constexpr EnableCopyMove() noexcept = default;
	constexpr EnableCopyMove(const EnableCopyMove&) noexcept = delete;
	constexpr EnableCopyMove(EnableCopyMove&&) noexcept = delete;
	EnableCopyMove& operator =(const EnableCopyMove&) noexcept = default;
	EnableCopyMove& operator =(EnableCopyMove&&) noexcept = default;
};

template <typename Tag>
struct EnableCopyMove<false, true, true, false, Tag> {
	constexpr EnableCopyMove() noexcept = default;
	constexpr EnableCopyMove(const EnableCopyMove&) noexcept = delete;
	constexpr EnableCopyMove(EnableCopyMove&&) noexcept = default;
	EnableCopyMove& operator =(const EnableCopyMove&) noexcept = default;
	EnableCopyMove& operator =(EnableCopyMove&&) noexcept = delete;
};

template <typename Tag>
struct EnableCopyMove<false, true, true, true, Tag> {
	constexpr EnableCopyMove() noexcept = default;
	constexpr EnableCopyMove(const EnableCopyMove&) noexcept = delete;
	constexpr EnableCopyMove(EnableCopyMove&&) noexcept = default;
	EnableCopyMove& operator =(const EnableCopyMove&) noexcept = default;
	EnableCopyMove& operator =(EnableCopyMove&&) noexcept = default;
};

template <typename Tag>
struct EnableCopyMove<true, false, false, false, Tag> {
	constexpr EnableCopyMove() noexcept = default;
	constexpr EnableCopyMove(const EnableCopyMove&) noexcept = default;
	constexpr EnableCopyMove(EnableCopyMove&&) noexcept = delete;
	EnableCopyMove& operator =(const EnableCopyMove&) noexcept = delete;
	EnableCopyMove& operator =(EnableCopyMove&&) noexcept = delete;
};

template <typename Tag>
struct EnableCopyMove<true, false, false, true, Tag> {
	constexpr EnableCopyMove() noexcept = default;
	constexpr EnableCopyMove(const EnableCopyMove&) noexcept = default;
	constexpr EnableCopyMove(EnableCopyMove&&) noexcept = delete;
	EnableCopyMove& operator =(const EnableCopyMove&) noexcept = delete;
	EnableCopyMove& operator =(EnableCopyMove&&) noexcept = default;
};

template <typename Tag>
struct EnableCopyMove<true, false, true, false, Tag> {
	constexpr EnableCopyMove() noexcept = default;
	constexpr EnableCopyMove(const EnableCopyMove&) noexcept = default;
	constexpr EnableCopyMove(EnableCopyMove&&) noexcept = default;
	EnableCopyMove& operator =(const EnableCopyMove&) noexcept = delete;
	EnableCopyMove& operator =(EnableCopyMove&&) noexcept = delete;
};

template <typename Tag>
struct EnableCopyMove<true, false, true, true, Tag> {
	constexpr EnableCopyMove() noexcept = default;
	constexpr EnableCopyMove(const EnableCopyMove&) noexcept = default;
	constexpr EnableCopyMove(EnableCopyMove&&) noexcept = default;
	EnableCopyMove& operator =(const EnableCopyMove&) noexcept = delete;
	EnableCopyMove& operator =(EnableCopyMove&&) noexcept = default;
};

template <typename Tag>
struct EnableCopyMove<true, true, false, false, Tag> {
	constexpr EnableCopyMove() noexcept = default;
	constexpr EnableCopyMove(const EnableCopyMove&) noexcept = default;
	constexpr EnableCopyMove(EnableCopyMove&&) noexcept = delete;
	EnableCopyMove& operator =(const EnableCopyMove&) noexcept = default;
	EnableCopyMove& operator =(EnableCopyMove&&) noexcept = delete;
};

template <typename Tag>
struct EnableCopyMove<true, true, false, true, Tag> {
	constexpr EnableCopyMove() noexcept = default;
	constexpr EnableCopyMove(const EnableCopyMove&) noexcept = default;
	constexpr EnableCopyMove(EnableCopyMove&&) noexcept = delete;
	EnableCopyMove& operator =(const EnableCopyMove&) noexcept = default;
	EnableCopyMove& operator =(EnableCopyMove&&) noexcept = default;
};

template <typename Tag>
struct EnableCopyMove<true, true, true, false, Tag> {
	constexpr EnableCopyMove() noexcept = default;
	constexpr EnableCopyMove(const EnableCopyMove&) noexcept = default;
	constexpr EnableCopyMove(EnableCopyMove&&) noexcept = default;
	EnableCopyMove& operator =(const EnableCopyMove&) noexcept = default;
	EnableCopyMove& operator =(EnableCopyMove&&) noexcept = delete;
};

} // namespace util

#pragma once

#include <exception>
#include <initializer_list>
#include <type_traits>
#include <utility>

#include "enable_special_members.h"

namespace util {

struct Nullopt {
    explicit constexpr Nullopt(int) {}
};

constexpr Nullopt nullopt{0};

struct InPlace {
    explicit InPlace() = default;
};

constexpr InPlace inPlace{};

template <typename T>
class DefaultOptionalPolicy {
public:
	constexpr bool initialized(const T&) const noexcept {
		return m_initialized;
	}

	constexpr void set(T&) noexcept {
		m_initialized = true;
	}

	constexpr void unset(T&) noexcept {
		m_initialized = false;
	}

	constexpr void reset(T& t) noexcept {
		m_initialized = false;
		t.~T();
	}

private:
	bool m_initialized;
};

namespace details {

template <typename T>
class OptionalStorage {
public:
	T* data() { return reinterpret_cast<T*>(&m_data); }

	const T* data() const { return reinterpret_cast<const T*>(&m_data); }

	T& ref() { return *(data()); }

	const T& ref() const { return *(data()); }

	template <typename... Args>
	T& emplace(Args&&... args) {
		new (data()) T(std::forward<Args>(args)...);
		return ref();
	}

private:
	std::aligned_storage_t<sizeof(T), alignof(T)> m_data;
};

template <typename T, typename Policy, bool>
class OptionalBase : private Policy {
	using StoredType = std::remove_const_t<T>;

public:
	constexpr OptionalBase() noexcept {
		Policy::unset(storage());
	}

	constexpr OptionalBase(Nullopt) noexcept
		: OptionalBase() {
	}

	template <typename... Args>
	constexpr explicit OptionalBase(InPlace, Args... args) {
		construct(std::forward<Args>(args)...);
	}

	template <typename U, typename... Args
		, std::enable_if_t<std::is_constructible<T, std::initializer_list<U>, Args&&...>::value, int>...>
	constexpr explicit OptionalBase(InPlace, std::initializer_list<U> ilist, Args... args) {
		::new (m_storage.data()) StoredType(ilist, std::forward<Args>(args)...);
		Policy::set(storage());
	}

	constexpr OptionalBase(const OptionalBase& other) {
		if (other.hasValue())
			construct(other.storage());
		else
			Policy::unset(storage());
	}

	constexpr OptionalBase(OptionalBase&& other) noexcept(std::is_nothrow_move_constructible<T>::value) {
		if (other.hasValue())
			construct(std::move(other.storage()));
		else
			Policy::unset(storage());
	}

	OptionalBase& operator =(const OptionalBase& other) {
		if (other.hasValue()) {
			if (hasValue())
				storage() = other.storage();
			else
			construct(other.storage());
		}
		else
			destruct();
		return *this;
	}

	OptionalBase& operator =(OptionalBase&& other) noexcept(std::is_nothrow_move_assignable<T>::value) {
		if (other.hasValue()) {
			if (hasValue())
				storage() = std::move(other.storage());
			else
			construct(std::move(other.storage()));
		}
		else
			destruct();
		return *this;
	}

	constexpr bool hasValue() const noexcept {
		return Policy::initialized(storage());
	}

	void reset() noexcept {
		if (hasValue())
			destruct();
	}

protected:
	T& storage() noexcept { return m_storage.ref(); }

	const T& storage() const noexcept { return m_storage.ref(); }

	template <typename... Args>
	void construct(Args&&... args) noexcept(std::is_nothrow_constructible<StoredType, Args...>::value) {
		::new (m_storage.data()) StoredType(std::forward<Args>(args)...);
		Policy::set(storage());
	}

	void destruct() noexcept {
		Policy::reset(storage());
	}

private:
	OptionalStorage<StoredType> m_storage;
};

template <typename T, typename Policy>
class OptionalBase<T, Policy, false> : private Policy {
	using StoredType = std::remove_const_t<T>;

public:
	constexpr OptionalBase() noexcept {
		Policy::unset(storage());
	}

	constexpr OptionalBase(Nullopt) noexcept
		: OptionalBase() {
	}

	template <typename... Args>
	constexpr explicit OptionalBase(InPlace, Args... args) {
		construct(std::forward<Args>(args)...);
	}

	template <typename U, typename... Args
		, std::enable_if_t<std::is_constructible<T, std::initializer_list<U>, Args&&...>::value, int>...>
	constexpr explicit OptionalBase(InPlace, std::initializer_list<U> ilist, Args... args) {
		::new (m_storage.data()) StoredType(ilist, std::forward<Args>(args)...);
		Policy::set(storage());
	}

	constexpr OptionalBase(const OptionalBase& other) {
		if (other.hasValue())
			construct(other.storage());
		else
			Policy::unset(storage());
	}

	constexpr OptionalBase(OptionalBase&& other) noexcept(std::is_nothrow_move_constructible<T>::value) {
		if (other.hasValue())
			construct(std::move(other.storage()));
		else
			Policy::unset(storage());
	}

	OptionalBase& operator =(const OptionalBase& other) {
		if (other.hasValue()) {
			if (hasValue())
				storage() = other.storage();
			else
			construct(other.storage());
		}
		else
			destruct();
		return *this;
	}

	OptionalBase& operator =(OptionalBase&& other) noexcept(std::is_nothrow_move_assignable<T>::value) {
		if (other.hasValue()) {
			if (hasValue())
				storage() = std::move(other.storage());
			else
			construct(std::move(other.storage()));
		}
		else
			destruct();
		return *this;
	}

	~OptionalBase() {
		if (hasValue())
			storage().~T();
	}

	constexpr bool hasValue() const noexcept {
		return Policy::initialized(storage());
	}

	void reset() noexcept {
		if (hasValue())
			destruct();
	}

protected:
	T& storage() { return m_storage.ref(); }

	const T& storage() const { return m_storage.ref(); }

	template <typename... Args>
	void construct(Args&&... args) noexcept(std::is_nothrow_constructible<StoredType, Args...>::value) {
		::new (m_storage.data()) StoredType(std::forward<Args>(args)...);
		Policy::set(storage());
	}

	void destruct() noexcept {
		Policy::reset(storage());
	}

private:
	OptionalStorage<StoredType> m_storage;
};

template <typename T>
using OptionalEnableCopyMove = EnableCopyMove<std::is_copy_constructible<T>::value
	, std::is_copy_constructible<T>::value && std::is_copy_assignable<T>::value
	, std::is_move_constructible<T>::value
	, std::is_move_constructible<T>::value && std::is_move_assignable<T>::value
	, T>;

} // namespace details

template <typename T, typename Policy>
class Optional;

namespace details {

template <typename T, typename U, typename Policy>
constexpr bool CONVERTS_FROM_OPTIONAL
	= std::is_constructible<T, const util::Optional<U, Policy>&>::value
	|| std::is_constructible<T, util::Optional<U, Policy>&>::value
	|| std::is_constructible<T, const util::Optional<U, Policy>&&>::value
	|| std::is_constructible<T, util::Optional<U, Policy>&&>::value
	|| std::is_convertible<const util::Optional<U, Policy>&, T>::value
	|| std::is_convertible<util::Optional<U, Policy>&, T>::value
	|| std::is_convertible<const util::Optional<U, Policy>&&, T>::value
	|| std::is_convertible<util::Optional<U, Policy>&&, T>::value;

template <typename T, typename U, typename Policy>
constexpr bool ASSIGNS_FROM_OPTIONAL
	= std::is_assignable<T&, const util::Optional<U, Policy>&>::value
	|| std::is_assignable<T&, util::Optional<U, Policy>&>::value
	|| std::is_assignable<T&, const util::Optional<U, Policy>&&>::value
	|| std::is_assignable<T&, util::Optional<U, Policy>&&>::value;

} // namespace details

// Does not own the description string.
class BadOptionalAccess : public std::exception {
public:
	BadOptionalAccess(const char* description) noexcept
		: m_description(description) {
	}

	const char* what() const noexcept override final {
		return m_description;
	}

private:
	const char* m_description;
};

template <typename T, typename Policy = DefaultOptionalPolicy<T>>
class Optional : public details::OptionalBase<T, Policy, std::is_trivially_destructible<T>::value>
	, private details::OptionalEnableCopyMove<T> {
	static_assert(!std::is_same<std::remove_cv_t<T>, Nullopt>::value
		&& !std::is_same<std::remove_cv_t<T>, InPlace>::value
		&& !std::is_reference<T>::value, "Invalid instantiation of util::Optional");

	using Base = details::OptionalBase<T, Policy, std::is_trivially_destructible<T>::value>;

public:
	using ValueType = T;

	using Base::Base;

	constexpr Optional() = default;

	template <typename U, typename OtherPolicy
		, std::enable_if_t<!std::is_same<T, U>::value
			&& std::is_constructible<T, const U&>::value
			&& std::is_convertible<const U&, T>::value
			&& !details::CONVERTS_FROM_OPTIONAL<T, U, OtherPolicy>, bool> = true>
	constexpr Optional(const Optional<U, OtherPolicy>& other) {
		if (other)
			emplaceWithoutReset(*other);
	}

	template <typename U, typename OtherPolicy
		, std::enable_if_t<!std::is_same<T, U>::value
			&& std::is_constructible<T, const U&>::value
			&& !std::is_convertible<const U&, T>::value
			&& !details::CONVERTS_FROM_OPTIONAL<T, U, OtherPolicy>, bool> = true>
	explicit constexpr Optional(const Optional<U, OtherPolicy>& other) {
		if (other)
			emplaceWithoutReset(*other);
	}

	template <typename U, typename OtherPolicy
		, std::enable_if_t<!std::is_same<T, U>::value
			&& std::is_constructible<T, U&&>::value
			&& std::is_convertible<U&&, T>::value
			&& !details::CONVERTS_FROM_OPTIONAL<T, U, OtherPolicy>, bool> = true>
	constexpr Optional(Optional<U, OtherPolicy>&& other) {
		if (other)
			emplaceWithoutReset(std::move(*other));
	}

	template <typename U, typename OtherPolicy
		, std::enable_if_t<!std::is_same<T, U>::value
			&& std::is_constructible<T, U&&>::value
			&& !std::is_convertible<U&&, T>::value
			&& !details::CONVERTS_FROM_OPTIONAL<T, U, OtherPolicy>, bool> = true>
	explicit constexpr Optional(Optional<U, OtherPolicy>&& other) {
		if (other)
			emplaceWithoutReset(std::move(*other));
	}

	template <typename U = T
		, std::enable_if_t<!std::is_same<Optional<T, Policy>, std::decay_t<U>>::value
			&& std::is_constructible<T, U&&>::value
			&& std::is_convertible<T&, U>::value, bool> = true>
	constexpr Optional(U&& value)
		: Base(inPlace, std::forward<U>(value)) {
	}

	template <typename U = T
		, std::enable_if_t<!std::is_same<Optional<T, Policy>, std::decay_t<U>>::value
			&& std::is_constructible<T, U&&>::value
			&& !std::is_convertible<T&, U>::value, bool> = false>
	explicit constexpr Optional(U&& value)
		: Base(inPlace, std::forward<U>(value)) {
	}
	
	Optional& operator =(Nullopt) noexcept {
		this->reset();
		return *this;
	}

	template <typename U, typename OtherPolicy
		, typename = std::enable_if_t<!std::is_same<T, U>::value
			&& std::is_constructible<T, U>::value
			&& std::is_assignable<T&, U>::value
			&& !details::CONVERTS_FROM_OPTIONAL<T, U, OtherPolicy>
			&& !details::ASSIGNS_FROM_OPTIONAL<T, U, OtherPolicy>>>
	Optional& operator =(const Optional<U, OtherPolicy>& other) {
		if (other) {
			if (*this)
				**this = *other;
			else
				this->construct(*other);
		}
		else
			this->destruct();
		return *this;
	}

	template <typename U, typename OtherPolicy
		, typename = std::enable_if_t<!std::is_same<T, U>::value
			&& std::is_constructible<T, U>::value
			&& std::is_assignable<T&, U>::value
			&& !details::CONVERTS_FROM_OPTIONAL<T, U, OtherPolicy>
			&& !details::ASSIGNS_FROM_OPTIONAL<T, U, OtherPolicy>>>
	Optional& operator =(Optional<U, OtherPolicy>&& other) {
		if (other) {
			if (*this)
				**this = std::move(*other);
			else
				this->construct(std::move(*other));
		}
		else
			this->destruct();
		return *this;
	}

	template <typename U = T
		, typename = std::enable_if_t<!std::is_same<Optional<T, Policy>, std::decay_t<U>>::value
			&& std::is_constructible<T, U>::value
			&& !(std::is_scalar<T>::value && std::is_same<T, std::decay_t<U>>::value)
			&& std::is_assignable<T&, U>::value>>
	Optional& operator =(U&& value) {
		if (*this)
			**this = std::forward<U>(value);
		else
			this->construct(std::forward<U>(value));
		return *this;
	}

	constexpr const T* operator ->() const {
		return **this;
	}

	constexpr T* operator ->() {
		return **this;
	}

	constexpr const T& operator *() const& {
		return this->storage();
	}

	constexpr T& operator *() & {
		return this->storage();
	}

	constexpr const T&& operator *() const&& {
		return std::move(this->storage());
	}

	constexpr T&& operator *() && {
		return std::move(this->storage());
	}

	constexpr explicit operator bool() const noexcept {
		return this->hasValue();
	}

	constexpr T& value() & {
		return (*this)
			? **this
			: throw BadOptionalAccess("Attempt to access value of a "
				"disengaged optional object");
	}

	constexpr const T& value() const & {
		return (*this)
			? **this
			: (throw BadOptionalAccess("Attempt to access value of a "
				"disengaged optional object"), **this);
	}

	constexpr T&& value() && {
		return (*this)
			? std::move(**this)
			: (throw BadOptionalAccess("Attempt to access value of a "
				"disengaged optional object"), std::move(**this));
	}

	constexpr const T&& value() const && {
		return (*this)
			? std::move(**this)
			: (throw BadOptionalAccess("Attempt to access value of a "
				"disengaged optional object"), std::move(**this));
	}

	template <typename U>
	constexpr T valueOr(U&& defaultValue) const& {
		static_assert(std::is_copy_constructible<T>::value && std::is_convertible<U&&, T>::value
			, "Cannot return value");
		bool(*this) ? **this : static_cast<T>(std::forward<U>(defaultValue));
	}

	template <typename U>
	constexpr T valueOr(U&& defaultValue) && {
		static_assert(std::is_copy_constructible<T>::value && std::is_convertible<U&&, T>::value
			, "Cannot return value");
		bool(*this) ? std::move(**this) : static_cast<T>(std::forward<U>(defaultValue));
	}

	void swap(Optional& other)
		noexcept(std::is_nothrow_move_constructible<T>::value) {
		// TODO: C++17 also add std::is_nothrow_swappable<T>::value)
		if (other) {
			if (*this)
				std::swap(**this && *other);
			else {
				this->construct(std::move(*other));
				other.destruct();
			}
		}
		else {
			other.construct(std::move(**this));
			this->destruct();
		}
	}

	template <typename... Args
		, std::enable_if_t<std::is_constructible<T, Args&&...>::value, int>...>
	T& emplace(Args&&... args) {
		this->reset();
		return emplaceWithoutReset(std::forward<Args>(args)...);
	}

	template <typename U, typename... Args
		, std::enable_if_t<std::is_constructible<T, std::initializer_list<U>, Args&&...>::value, int>...>
	T& emplace(std::initializer_list<U> ilist, Args&&... args) {
		this->reset();
		return emplaceWithoutReset(ilist, std::forward<Args>(args)...);
	}

private:
	template <typename... Args
		, std::enable_if_t<std::is_constructible<T, Args&&...>::value, int>...>
	T& emplaceWithoutReset(Args&&... args) {
		this->construct(std::forward<Args>(args)...);
		return **this;
	}

	template <typename U, typename... Args
		, std::enable_if_t<std::is_constructible<T, std::initializer_list<U>, Args&&...>::value, int>...>
	T& emplaceWithoutReset(std::initializer_list<U> ilist, Args&&... args) {
		return this->construct(ilist, std::forward<Args>(args)...);
		return **this;
	}
};

template <typename T, typename Policy, typename U, typename OtherPolicy>
constexpr bool operator ==(const Optional<T, Policy>& lhs, const Optional<U, OtherPolicy>& rhs) {
	return static_cast<bool>(lhs) == static_cast<bool>(rhs)
		&& (!lhs || *lhs == *rhs);
}

template <typename T, typename Policy, typename U, typename OtherPolicy>
constexpr bool operator !=(const Optional<T, Policy>& lhs, const Optional<U, OtherPolicy>& rhs) {
	return lhs != rhs;
}

template <typename T, typename Policy, typename U, typename OtherPolicy>
constexpr bool operator <(const Optional<T, Policy>& lhs, const Optional<U, OtherPolicy>& rhs) {
	return static_cast<bool>(rhs) && (!lhs || *lhs < *rhs);
}

template <typename T, typename Policy, typename U, typename OtherPolicy>
constexpr bool operator >(const Optional<T, Policy>& lhs, const Optional<U, OtherPolicy>& rhs) {
	return rhs < lhs;
}

template <typename T, typename Policy, typename U, typename OtherPolicy>
constexpr bool operator <=(const Optional<T, Policy>& lhs, const Optional<U, OtherPolicy>& rhs) {
	return !(rhs < lhs);
}

template <typename T, typename Policy, typename U, typename OtherPolicy>
constexpr bool operator >=(const Optional<T, Policy>& lhs, const Optional<U, OtherPolicy>& rhs) {
	return !(lhs < rhs);
}

template <typename T, typename Policy>
constexpr bool operator ==(const Optional<T, Policy>& lhs, Nullopt) {
	return !lhs;
}

template <typename T, typename Policy>
constexpr bool operator ==(Nullopt, const Optional<T, Policy>& rhs) {
	return !rhs;
}

template <typename T, typename Policy>
constexpr bool operator !=(const Optional<T, Policy>& lhs, Nullopt) {
	return static_cast<bool>(lhs);
}

template <typename T, typename Policy>
constexpr bool operator !=(Nullopt, const Optional<T, Policy>& rhs) {
	return static_cast<bool>(rhs);
}

template <typename T, typename Policy>
constexpr bool operator <(const Optional<T, Policy>&, Nullopt) {
	return false;
}

template <typename T, typename Policy>
constexpr bool operator <(Nullopt, const Optional<T, Policy>& rhs) {
	return static_cast<bool>(rhs);
}

template <typename T, typename Policy>
constexpr bool operator >(const Optional<T, Policy>& lhs, Nullopt) {
	return static_cast<bool>(lhs);
}

template <typename T, typename Policy>
constexpr bool operator >(Nullopt, const Optional<T, Policy>&) {
	return false;
}

template <typename T, typename Policy>
constexpr bool operator <=(const Optional<T, Policy>& lhs, Nullopt) {
	return !lhs;
}

template <typename T, typename Policy>
constexpr bool operator <=(Nullopt, const Optional<T, Policy>&) {
	return true;
}

template <typename T, typename Policy>
constexpr bool operator >=(const Optional<T, Policy>&, Nullopt) {
	return true;
}

template <typename T, typename Policy>
constexpr bool operator >=(Nullopt, const Optional<T, Policy>& rhs) {
	return !rhs;
}

template <typename T, typename Policy, typename U>
constexpr bool operator ==(const Optional<T, Policy>& lhs, const U& rhs) {
	return lhs && *lhs == rhs;
}

template <typename T, typename Policy, typename U>
constexpr bool operator ==(const U& lhs, const Optional<T, Policy>& rhs) {
	return rhs && lhs == *rhs;
}

template <typename T, typename Policy, typename U>
constexpr bool operator !=(const Optional<T, Policy>& lhs, const U& rhs) {
	return !(lhs && *lhs == rhs);
}

template <typename T, typename Policy, typename U>
constexpr bool operator !=(const U& lhs, const Optional<T, Policy>& rhs) {
	return !(rhs && lhs == *rhs);
}

template <typename T, typename Policy, typename U>
constexpr bool operator <(const Optional<T, Policy>& lhs, const U& rhs) {
	return !lhs || *lhs < rhs;
}

template <typename T, typename Policy, typename U>
constexpr bool operator <(const U& lhs, const Optional<T, Policy>& rhs) {
	return rhs && lhs < *rhs;
}

template <typename T, typename Policy, typename U>
constexpr bool operator >(const Optional<T, Policy>& lhs, const U& rhs) {
	return lhs && rhs < *lhs;
}

template <typename T, typename Policy, typename U>
constexpr bool operator >(const U& lhs, const Optional<T, Policy>& rhs) {
	return !rhs || *rhs < lhs;
}

template <typename T, typename Policy, typename U>
constexpr bool operator <=(const Optional<T, Policy>& lhs, const U& rhs) {
	return !lhs || !(rhs < *lhs);
}

template <typename T, typename Policy, typename U>
constexpr bool operator <=(const U& lhs, const Optional<T, Policy>& rhs) {
	return rhs && !(*rhs < lhs);
}

template <typename T, typename Policy, typename U>
constexpr bool operator >=(const Optional<T, Policy>& lhs, const U& rhs) {
	return lhs && !(*lhs < rhs);
}

template <typename T, typename Policy, typename U>
constexpr bool operator >=(const U& lhs, const Optional<T, Policy>& rhs) {
	return !rhs || !(lhs < *rhs);
}

template <typename T, typename Policy = DefaultOptionalPolicy<T>>
constexpr Optional<std::decay_t<T>, Policy> makeOptional(T&& value) {
	return Optional<std::decay_t<T>, Policy>(std::forward<T>(value));
}

template <typename T, typename Policy = DefaultOptionalPolicy<T>, typename... Args>
constexpr Optional<T, Policy> makeOptional(Args&&... args) {
	return Optional<T, Policy>(inPlace, std::forward<Args>(args)...);
}

template <typename T, typename U, typename Policy = DefaultOptionalPolicy<T>, typename... Args>
constexpr Optional<T, Policy> makeOptional(std::initializer_list<U> ilist, Args&&... args) {
	return Optional<T, Policy>(inPlace, ilist, std::forward<Args>(args)...);
}

} // namespace util

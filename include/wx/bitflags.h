#ifndef _BITFLAGS_H_
#define _BITFLAGS_H_

#include <concepts>

template<typename Enum>
concept BitfieldCompatible =  requires
{
    requires(std::is_enum_v<Enum>);
    {Enum::_max_size};
};

template<typename Enum, typename Tag> requires(BitfieldCompatible<Enum>)
class Bitfield;

template<typename Enum>
using InclBitfield = Bitfield<Enum, struct InclType>;

template<typename Enum>
using MaskBitfield = Bitfield<Enum, struct MaskType>;

template<typename Enum, typename Tag> requires(BitfieldCompatible<Enum>)
class Bitfield
{
public:
    using value_type = std::underlying_type_t<Enum>;

    constexpr Bitfield() {}

    explicit constexpr Bitfield(value_type bits) : m_fields{bits}
    {
    }

    explicit constexpr Bitfield(const Enum& e)
    {
        set(e);
    }

    template<typename... Enums> requires(BitfieldCompatible<Enums> && ...)
    constexpr Bitfield(const Enums&... es)
    {
        (set(es), ...);
    }

    static constexpr value_type bitmask(const Enum& e) noexcept
    {
        return value_type{1} << static_cast<value_type>(e);
    }

    constexpr explicit operator bool() const noexcept
    {
        return m_fields != value_type{0};
    }

    template<typename... Enums> requires(BitfieldCompatible<Enums> && ...)
    static constexpr value_type bitmask(const Enums&... es) noexcept
    {
        return ((value_type{ 1 } << static_cast<value_type>(es)) | ...);
    }

    static constexpr value_type AllFlagsSet = bitmask(Enum::_max_size) - value_type{1};

    constexpr auto& operator|=(const Enum& e) noexcept
    {
        m_fields |= bitmask(e);

        return *this;
    }

    constexpr bool is_set(const Enum& e) const noexcept
    {
        auto fields = m_fields;
        fields &= bitmask(e);
        return fields != value_type{0};
    }

    constexpr auto& operator|=(const Bitfield& otherBf) noexcept
    {
        m_fields |= otherBf.as_value();

        return *this;
    }

    constexpr auto& operator&=(const Bitfield& otherBf) noexcept
    {
        m_fields &= otherBf.as_value();

        return *this;
    }

    constexpr auto& operator&=(const Enum& e) noexcept
    {
        m_fields &= bitmask(e);

        return *this;
    }

    constexpr auto& operator^=(const Enum& e) noexcept
    {
        m_fields ^= bitmask(e);

        return *this;
    }

    constexpr bool empty() const noexcept
    {
        return m_fields == value_type{0};
    }

    constexpr auto clear() noexcept
    {
        m_fields = value_type{0};
    }

    constexpr void set(const Enum& e) noexcept
    {
        m_fields |= bitmask(e);
    }

    template<typename... Enums> requires(BitfieldCompatible<Enums> && ...)
    constexpr void set(const Enums&... es) noexcept
    {
        m_fields |= (bitmask(es), ...);
    }

    constexpr void reset(const Enum& e) noexcept
    {
        m_fields &= ~bitmask(e);
    }

    constexpr void toggle(const Enum& e) noexcept
    {
        m_fields ^= bitmask(e);
    }

    constexpr void set_all() noexcept
    {
        m_fields = (value_type{1} << static_cast<value_type>(Enum::_max_size)) - value_type{1};
    }

    constexpr void toggle_all() noexcept
    {
        m_fields ^= AllFlagsSet;
    }

    constexpr auto as_value() const noexcept
    {
        return m_fields;
    }

    auto operator<=>(const Bitfield&) const noexcept = default;

private:
    value_type m_fields{};
};

template<typename Enum> requires(BitfieldCompatible<Enum>)
constexpr auto operator|(InclBitfield<Enum> bf, const Enum& e) noexcept
{
    bf |= e;
    return bf;
}

template<typename Enum> requires(BitfieldCompatible<Enum>)
constexpr auto operator&(InclBitfield<Enum> bf, const Enum& e) noexcept
{
    bf &= e;
    return bf;
}

template<typename Enum> requires(BitfieldCompatible<Enum>)
constexpr auto operator&(InclBitfield<Enum> bf, const InclBitfield<Enum>& otherBf) noexcept
{
    bf &= otherBf;
    return bf;
}

template<typename Enum> requires(BitfieldCompatible<Enum>)
constexpr auto operator&(const Enum& e, InclBitfield<Enum> bf) noexcept
{
    return bf & e;
}

template<typename Enum> requires(BitfieldCompatible<Enum>)
constexpr auto operator^(InclBitfield<Enum> bf, const Enum& e) noexcept
{
    bf ^= e;
    return bf;
}

#endif // _BITFLAGS_H_
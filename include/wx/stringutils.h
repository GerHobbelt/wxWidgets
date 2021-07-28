#ifndef _WX_WXSTRINGUTILS_H__
#define _WX_WXSTRINGUTILS_H__

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>

namespace wx::utils
{

#if __cplusplus >= 201907L
#define CONSTEXPR_STR20 constexpr
#else
#define CONSTEXPR_STR20
#endif

// Modifying string functions

[[maybe_unused]] inline CONSTEXPR_STR20 std::size_t ReplaceAll(std::string& instr, std::string_view candidate, std::string_view replacement)
{
    std::size_t count{ 0 };
    for (std::string::size_type pos{};
        instr.npos != (pos = instr.find(candidate.data(), pos, candidate.length()));
        pos += replacement.length(), ++count)
    {
        instr.replace(pos, candidate.length(), replacement.data(), replacement.length());
    }

    return count;
}

[[maybe_unused]] inline CONSTEXPR_STR20 size_t Erase(std::string& str, char value)
{
    auto it = std::remove(str.begin(), str.end(), value);
    const auto elems_erased = std::distance(it, str.end());

    str.erase(it, str.end());

    return elems_erased;
}

template<typename Pred>
[[maybe_unused]] inline CONSTEXPR_STR20 size_t EraseIf(std::string& str, Pred&& pred)
{
    auto it = std::remove_if(str.begin(), str.end(), pred);
    auto elems_erased = std::distance(it, str.end());

    str.erase(it, str.end());

    return elems_erased;
}

// FIXME: Not valid for unicode strings.
inline CONSTEXPR_STR20 void ToUpper(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) noexcept { return std::toupper(c); });
}

// FIXME: Not valid for unicode strings.
inline CONSTEXPR_STR20 void ToLower(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](char c) noexcept { return std::tolower(c); });
}


// Non-modifying string functions

[[nodiscard]] inline constexpr bool StartsWith(std::string_view strView, std::string_view prefix) noexcept
{
    const auto prefixSize = prefix.size();

    if(strView.size() < prefixSize)
        return false;

    return strView.compare(0, prefix.size(), prefix) == 0;
}

[[nodiscard]] inline constexpr bool StartsWith(std::string_view strView, const char* const prefix)
{
    return StartsWith(strView, std::string_view(prefix));
}

[[nodiscard]] inline constexpr bool StartsWith(std::string_view strView, const char prefix) noexcept
{
    return !strView.empty() && strView.front() == prefix;
}

[[nodiscard]] inline constexpr bool EndsWith(std::string_view strView, std::string_view suffix) noexcept
{
    const auto suffixSize = suffix.size();

    if(strView.size() < suffixSize)
        return false;

    return strView.compare(strView.size() - suffixSize, suffixSize, suffix) == 0;
}

[[nodiscard]] inline constexpr bool EndsWith(std::string_view strView, const char* const suffix)
{
    return EndsWith(strView, std::string_view(suffix));
}

[[nodiscard]] inline constexpr bool EndsWith(std::string_view strView, const char suffix) noexcept
{
    return !strView.empty() && strView.back() == suffix;
}

// FIXME: Wrong (for Unicode), and temporary implementation of a case insensitive string comparison
[[nodiscard]] inline int CmpNoCase(std::string_view strViewA, std::string_view strViewB)
{
    const auto nA = strViewA.size();

    std::string strA(nA, ' ');
    std::string strB(nA, ' ');

    std::transform(strViewA.begin(), strViewA.end(), strA.begin(), [](unsigned char c) noexcept { return std::tolower(c); });
    std::transform(strViewB.begin(), strViewB.end(), strB.begin(), [](unsigned char c) noexcept { return std::tolower(c); });

    return strA.compare(strB);
}

// FIXME: Wrong (for Unicode), and temporary implementation of a case insensitive string comparison
[[nodiscard]] inline int CmpNoCase(const char* const chsA, const char* const chsB)
{
    return CmpNoCase(std::string_view(chsA), std::string_view(chsB));
}

[[nodiscard]] inline constexpr bool IsSameAsCase(std::string_view strViewA, std::string_view strViewB) noexcept
{
    return strViewA == strViewB;
}

[[nodiscard]] inline bool IsSameAsNoCase(std::string_view strViewA, std::string_view strViewB) 
{
    return CmpNoCase(strViewA, strViewB) == 0;
}

[[nodiscard]] inline CONSTEXPR_STR20 std::string BeforeFirst(std::string_view strView, std::string_view strFirst, size_t pos = 0) noexcept
{
    const auto n = strView.find(strFirst, pos);

    if(n != std::string_view::npos)
        return std::string(strView.substr(0, n));
    
    return std::string(strView);
}

[[nodiscard]] inline CONSTEXPR_STR20 std::string BeforeFirst(std::string_view strView, const char ch, size_t pos = 0) noexcept
{
    const auto n = strView.find(ch, pos);

    if(n != std::string_view::npos)
        return std::string(strView.substr(0, n));
    
    return std::string(strView);
}

[[nodiscard]] inline CONSTEXPR_STR20 std::string BeforeFirst(std::string_view strView, const char* const chs, size_t pos = 0)
{
    return BeforeFirst(strView, std::string_view(chs), pos);
}

[[nodiscard]] inline CONSTEXPR_STR20 std::string AfterFirst(std::string_view strView, std::string_view strAfter, size_t pos = 0) noexcept
{
    const auto n = strView.find(strAfter, pos);

    if(n != std::string_view::npos)
        return std::string(strView.substr(n + 1, strView.size()));
    
    return {};
}

[[nodiscard]] inline CONSTEXPR_STR20 std::string AfterFirst(std::string_view strView, const char ch, size_t pos = 0) noexcept
{
    const auto n = strView.find(ch, pos);

    if(n != std::string_view::npos)
        return std::string(strView.substr(n + 1, strView.size()));
    
    return {};
}

[[nodiscard]] inline CONSTEXPR_STR20 std::string AfterFirst(std::string_view strView, const char* const chs, size_t pos = 0)
{
    return AfterFirst(strView, std::string_view(chs), pos);
}

[[nodiscard]] inline CONSTEXPR_STR20 std::string BeforeLast(std::string_view strView, std::string_view strBefore, size_t pos = std::string_view::npos) noexcept
{
    const auto n = strView.rfind(strBefore, pos);

    if(n != std::string_view::npos)
        return std::string(strView.substr(0, n));

    return {};
}

[[nodiscard]] inline CONSTEXPR_STR20 std::string BeforeLast(std::string_view strView, const char ch, size_t pos = std::string_view::npos) noexcept
{
    const auto n = strView.rfind(ch, pos);

    if(n != std::string_view::npos)
        return std::string(strView.substr(0, n));

    return {};
}

[[nodiscard]] inline CONSTEXPR_STR20 std::string BeforeLast(std::string_view strView, const char* const chs, size_t pos = std::string_view::npos)
{
    return BeforeLast(strView, std::string_view(chs), pos);
}


// TODO: Do we really want to return the whole input string if it fails to find anything?
[[nodiscard]] inline CONSTEXPR_STR20 std::string AfterLast(std::string_view strView, std::string_view strLast, size_t pos = std::string_view::npos) noexcept
{
    const auto n = strView.rfind(strLast, pos);

    if(n != std::string_view::npos)
        return std::string(strView.substr(n + 1, strView.size()));

    return std::string(strView);
}

[[nodiscard]] inline CONSTEXPR_STR20 std::string AfterLast(std::string_view strView, const char ch, size_t pos = std::string_view::npos) noexcept
{
    const auto n = strView.rfind(ch, pos);

    if(n != std::string_view::npos)
        return std::string(strView.substr(n + 1, strView.size()));

    return std::string(strView);
}

[[nodiscard]] inline CONSTEXPR_STR20 std::string AfterLast(std::string_view strView, const char* const chs, size_t pos = std::string_view::npos)
{
    return AfterLast(strView, std::string_view(chs), pos);
}

[[nodiscard]] inline constexpr bool Contains(std::string_view strView, std::string_view strToFind) noexcept
{
    return strView.find(strToFind, 0) != std::string_view::npos;
}

// FIXME: Not valid for unicode strings
[[nodiscard]] inline CONSTEXPR_STR20 bool ContainsNoCase(std::string_view strView, std::string_view strToFind) noexcept
{
    std::string str(strView);
    std::string substrToFind(strToFind);

    ToLower(str);
    ToLower(substrToFind);

    return str.find(substrToFind) != std::string::npos;
}

} // namespace wx::util

#endif _WX_WXSTRINGUTILS_H__
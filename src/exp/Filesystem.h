#pragma once

#include <ghc/fs_std_fwd.hpp>

// Disable a false-positive warning issue with gcc:
#if defined(__GNUC__) && !defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdangling-reference"
#endif
    #include <spdlog/fmt/bundled/core.h>
#if defined(__GNUC__) && !defined(__clang__)
    #pragma GCC diagnostic pop
#endif

template<>
struct fmt::formatter<fs::path> {
    constexpr auto parse(format_parse_context& ctx) -> format_parse_context::iterator {
        return ctx.end();
    }
    auto format(const fs::path& path, format_context& ctx) const -> format_context::iterator {
        return format_to(ctx.out(), "{}", path.string());
    }
};

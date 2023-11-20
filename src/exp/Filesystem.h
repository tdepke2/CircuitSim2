#pragma once

#include <ghc/fs_std_fwd.hpp>
#include <spdlog/fmt/bundled/core.h>

template<>
struct fmt::formatter<fs::path> {
    constexpr auto parse(format_parse_context& ctx) -> format_parse_context::iterator {
        return ctx.end();
    }
    auto format(const fs::path& path, format_context& ctx) const -> format_context::iterator {
        return format_to(ctx.out(), "{}", path.string());
    }
};

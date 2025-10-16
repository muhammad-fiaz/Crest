-- Crest Framework - xmake Build Configuration
-- Modern C REST API Framework

set_project("crest")
set_version("1.0.0")
set_languages("c11", "cxx17")

-- Options
option("shared", {description = "Build shared library", default = true})
option("static", {description = "Build static library", default = true})
option("examples", {description = "Build examples", default = true})
option("dashboard", {description = "Enable web dashboard", default = true})

-- Platform specific settings
if is_plat("windows") then
    add_defines("_WIN32_WINNT=0x0601")
    add_syslinks("ws2_32")
elseif is_plat("linux", "macosx") then
    add_syslinks("pthread")
end

-- Include directories
add_includedirs("include")

-- Source files
local core_sources = {
    "src/core/app.c",
    "src/core/config.c",
    "src/core/request.c",
    "src/core/response.c",
    "src/core/server.c",
    "src/core/router.c",
    "src/core/middleware.c"
}

local util_sources = {
    "src/utils/json.c",
    "src/utils/logger.c",
    "src/utils/string_utils.c"
}

local dashboard_sources = {
    "src/dashboard/dashboard.c"
}

local all_sources = {}
for _, v in ipairs(core_sources) do table.insert(all_sources, v) end
for _, v in ipairs(util_sources) do table.insert(all_sources, v) end
for _, v in ipairs(dashboard_sources) do table.insert(all_sources, v) end

-- Shared library target
if has_config("shared") then
    target("crest")
        set_kind("shared")
        add_files(all_sources)
        add_headerfiles("include/crest/*.h")
        set_targetdir("build/lib")
        
        after_install(function (target)
            print("Crest shared library installed successfully!")
        end)
    target_end()
end

-- Static library target
if has_config("static") then
    target("crest_static")
        set_kind("static")
        add_files(all_sources)
        add_headerfiles("include/crest/*.h")
        set_targetdir("build/lib")
        set_basename("crest")
        
        after_install(function (target)
            print("Crest static library installed successfully!")
        end)
    target_end()
end

-- Examples
if has_config("examples") then
    target("crest_basic_example")
        set_kind("binary")
        add_files("examples/basic/main.c")
        add_deps("crest")
        set_targetdir("build/examples")
    target_end()
    
    target("crest_advanced_example")
        set_kind("binary")
        add_files("examples/advanced/advanced.c")
        add_deps("crest")
        set_targetdir("build/examples")
    target_end()
end

-- Installation
on_install(function (target)
    -- Install headers
    os.cp("include/crest/*.h", path.join(target:installdir(), "include/crest"))
    
    -- Install libraries
    if target:kind() == "shared" or target:kind() == "static" then
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end
end)

-- Package information
set_description("Crest - A modern, fast, and lightweight REST API framework for C/C++")
set_license("MIT")
set_homepage("https://muhammad-fiaz.github.io/crest")

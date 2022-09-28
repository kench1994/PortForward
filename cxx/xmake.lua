
add_rules("mode.debug", "mode.release")

-- if is_os("windows") then
--     add_cxflags("/EHsc", {force = true})
--     if(is_mode("release")) then
--         set_config("vs_runtime", "MD")
--         add_cxflags("-MD", {force = true})
--     else
--         set_config("vs_runtime", "MDd")
--         add_cxflags("-MDd", {force = true})
--     end
-- else
--     add_cxflags("-MD", {force = true})
-- end

add_requires("vcpkg::boost-asio" , {configs = {shared = true, vs_runtime = "MT"}, alias = "asio"})

target("forwarder")
    set_kind("binary")
    set_symbols("debug")
    add_includedirs("Utils")
    add_headerfiles("*.h", "Utils/utils/*.h*")
    add_files("*.cpp")

    add_packages("asio")

set_languages("c11", "cxx14")
add_requires("vcpkg::boost-asio" , {configs = {shared = true, vs_runtime = "MT"}, alias = "asio"})
add_requires("vcpkg::nlohmann-json" , {configs = {shared = true, vs_runtime = "MT"}, alias = "nlohmann-json"})


-- 如果当前编译模式是debug
if is_mode("debug") then
    -- 添加DEBUG编译宏
    add_defines("DEBUG")
    -- 启用调试符号
    set_symbols("debug")
    -- 禁用优化
    set_optimize("none")
end

target("PortForward")
    add_rules("qt.widgetapp")

    add_includedirs("impl", "Utils")

    add_files("*.hpp", "*.cpp", "mainwindow.ui", "impl/*.cpp")
    -- 添加带有 Q_OBJECT 的meta头文件
    add_files("mainwindow.h")

    add_headerfiles("impl/*.h")

    add_cxflags("-execution-charset:utf-8", "-source-charset:utf-8")

    add_packages("asio", "nlohmann-json")

    set_symbols("debug")


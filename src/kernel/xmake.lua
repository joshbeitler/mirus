target("kernel")
    set_kind("binary")
    add_files("core/*.c")
    add_files("debug/*.c")
    add_files("interrupt/*.c")
    add_files("memory/*.c")
    set_filename("kernel")

    add_includedirs("$(projectdir)/src/hal/include")
    add_includedirs("$(projectdir)/src/libs/libk/include")
    add_includedirs("$(projectdir)/src/libs/limine/include")
    add_includedirs("$(projectdir)/src/libs/ssfn/include")
    add_includedirs("$(projectdir)/src/libs/logger/include")
    add_includedirs("$(projectdir)/src/libs/printf")
    add_includedirs("$(projectdir)/src/kernel/include")

    add_ldflags("-T$(projectdir)/meta/linker.ld", { force = true })
    add_ldflags("-z text", { force = true })
    add_ldflags("-z max-page-size=0x1000", { force = true })

    add_deps("libk")
    add_deps("printf")
    add_deps("hal")
    add_deps("logger")
    add_deps("limine")
    add_deps("ssfn")

    --add_links("libk", "printf", "logger", "hal")

    -- Debug configuration (always on for now)
    add_cxflags("-g")
    add_cxflags("-O0")
    add_ldflags("-g", { force = true })

    -- Generate debug symbols
    after_build(function (target)
        import("core.project.config")
        local targetfile = target:targetfile()
        print("Target file: " .. targetfile)
        os.execv("llvm-objcopy", {"--only-keep-debug", targetfile, targetfile .. ".sym"})
        os.execv("llvm-objcopy", {"--strip-debug", targetfile})
        os.execv("llvm-objcopy", {"--add-gnu-debuglink=" .. targetfile .. ".sym", targetfile})
    end)

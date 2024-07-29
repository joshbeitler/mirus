target("hal")
    set_kind("static")
    add_values("nasm.flags", "-felf64")
    add_includedirs("include", { public = true })
    add_includedirs("$(projectdir)/src/libs/libk/include")
    add_includedirs("$(projectdir)/src/libs/logger/include")
    add_deps("libk")
    add_deps("logger")

    add_ldflags("-T$(projectdir)/meta/linker.ld", { force = true })
    add_ldflags("-pie", { force = true })
    add_ldflags("-z text", { force = true })
    add_ldflags("-z max-page-size=0x1000", { force = true })

    if is_arch("x86_64") then
        add_files("*.c")
        add_files("arch-x86_64/cpu/*.c")
        add_files("arch-x86_64/cpu/*.asm", {force = {nasm = true}})
        add_files("arch-x86_64/io/*.c")
    else
        -- unsupported
    end

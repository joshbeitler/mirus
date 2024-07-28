toolchain("clang-cross")
    set_kind("standalone")
    set_toolset("cc", "clang")
    set_toolset("ld", "ld.lld")
    set_toolset("ar", "llvm-ar")
    set_toolset("strip", "llvm-strip")
    set_toolset("as", "nasm")
    add_cxflags("-target x86_64-unknown-elf")
    add_cxflags("-Wall")
    add_cxflags("-Wextra")
    add_cxflags("-std=gnu11")
    add_cxflags("-ffreestanding")
    add_cxflags("-fno-stack-protector")
    add_cxflags("-fno-stack-check")
    add_cxflags("-fno-lto")
    add_cxflags("-fPIE")
    add_cxflags("-m64")
    add_cxflags("-mno-80387")
    add_cxflags("-mno-mmx")
    add_cxflags("-mno-sse")
    add_cxflags("-mno-sse2")
    add_cxflags("-mno-red-zone")
    add_cxflags("-fno-builtin")
    add_cxflags("-nostdlib")
    add_asflags("-felf64")
    add_includedirs("$(projectdir)/src/kernel/include")
    add_includedirs("$(projectdir)/src/libs/limine/include")
    add_includedirs("$(projectdir)/src/libs/ssfn/include")
    add_includedirs("$(projectdir)/src/libs/ia32/include")
    add_ldflags("-m elf_x86_64", { force = true })
    add_ldflags("-nostdlib", { force = true })
    add_ldflags("-pie", { force = true })
    add_ldflags("-z text", { force = true })
    add_ldflags("-z max-page-size=0x1000", { force = true })
toolchain_end()

set_toolchains("clang-cross")

target("kernel")
    set_kind("binary")
    set_arch("x86_64")
    add_files("core/*.c")
    add_files("core/*.asm", {force = {nasm = true}})
    add_values("nasm.flags", "-felf64")
    set_filename("kernel")
    add_ldflags("-T$(projectdir)/meta/linker.ld", { force = true })
    add_deps("limine")
    add_deps("ssfn")
    add_deps("ia32")

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

    -- Release configuration (commented out for now)
    --if is_mode("release") then
    --    add_cxflags("-O2")
    --end

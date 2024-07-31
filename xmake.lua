set_project("mirus")
set_version("0.1.0")

set_defaultarchs("x86_64")
set_arch("x86_64")

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
    add_cxflags("-fPIE")
    add_cxflags("-mno-80387")
    add_cxflags("-mno-mmx")
    add_cxflags("-mno-sse")
    add_cxflags("-mno-sse2")
    add_cxflags("-mno-red-zone")
    add_cxflags("-fno-builtin")
    add_cxflags("-nostdlib")

    add_asflags("-felf64")

    add_ldflags("-m elf_x86_64", { force = true })
    add_ldflags("-nostdlib", { force = true })
    add_ldflags("-pie", { force = true })
toolchain_end()

set_toolchains("clang-cross")

includes("src/hal")
includes("src/libs/libk")
includes("src/libs/limine")
includes("src/libs/ssfn")
includes("src/libs/logger")
includes("src/libs/printf")
includes("src/kernel")

includes("src/apps/hello")

task("make-iso")
    set_category("build")
    on_run(function ()
        import("core.project.project")
        import("core.project.config")

        local project_root = os.projectdir()
        local build_dir = config.buildir()
        local mode = config.mode() or "release"

        local kernel_target = project.target("kernel")
        if not kernel_target then
            raise("Kernel target not found!")
        end
        local kernel_path = kernel_target:targetfile()

        if not os.isfile(kernel_path) then
            kernel_path = path.join(path.directory(kernel_path), mode, path.filename(kernel_path))
            if not os.isfile(kernel_path) then
                raise("Kernel executable not found at " .. kernel_path)
            end
        end

        -- Clone Limine repository into the build directory
        local limine_dir = path.join(build_dir, "limine")
        if not os.isdir(limine_dir) then
            os.execv("git", {"clone", "https://github.com/limine-bootloader/limine.git", "--branch=v7.x-binary", "--depth=1", limine_dir})
        end

        -- Build Limine utility
        os.cd(limine_dir)
        os.exec("make")
        os.cd(project_root)

        -- Create ISO root directory in the build directory
        local iso_root = path.join(build_dir, "iso_root_" .. mode)
        os.mkdir(iso_root)
        os.mkdir(path.join(iso_root, "boot"))
        os.mkdir(path.join(iso_root, "boot/limine"))
        os.mkdir(path.join(iso_root, "EFI/BOOT"))

        -- Copy files
        os.cp("meta/*.sfn", iso_root)
        os.cp(kernel_path, path.join(iso_root, "boot"))
        os.cp("meta/limine.cfg", path.join(iso_root, "boot/limine"))
        os.cp(path.join(limine_dir, "limine-bios.sys"), path.join(iso_root, "boot/limine"))
        os.cp(path.join(limine_dir, "limine-bios-cd.bin"), path.join(iso_root, "boot/limine"))
        os.cp(path.join(limine_dir, "limine-uefi-cd.bin"), path.join(iso_root, "boot/limine"))
        os.cp(path.join(limine_dir, "BOOTX64.EFI"), path.join(iso_root, "EFI/BOOT"))
        os.cp(path.join(limine_dir, "BOOTIA32.EFI"), path.join(iso_root, "EFI/BOOT"))

        -- Create bootable ISO in the build directory
        local iso_file = path.join(build_dir, "image_" .. mode .. ".iso")
        os.execv("xorriso", {"-as", "mkisofs", "-b", "boot/limine/limine-bios-cd.bin",
                             "-no-emul-boot", "-boot-load-size", "4", "-boot-info-table",
                             "--efi-boot", "boot/limine/limine-uefi-cd.bin",
                             "-efi-boot-part", "--efi-boot-image", "--protective-msdos-label",
                             iso_root, "-o", iso_file})

        -- Install Limine for legacy BIOS boot
        os.execv(path.join(limine_dir, "limine"), {"bios-install", iso_file})

        print("ISO creation completed successfully!")
        print("ISO file created at: " .. iso_file)
    end)
    set_menu {
        usage = "xmake make-iso",
        description = "Create a bootable ISO image"
    }

task("run-qemu-uefi")
    set_category("run")
    on_run(function ()
        import("core.project.config")

        local build_dir = config.buildir()
        local mode = config.mode() or "release"

        -- Path to the ISO file
        local iso_file = path.join(build_dir, "image_" .. mode .. ".iso")

        -- Path to the OVMF BIOS file
        local ovmf_path = path.join(os.projectdir(), "meta", "OVMF_CODE.fd")

        -- Construct the QEMU command
        local qemu_cmd = string.format(
            "qemu-system-x86_64 -M q35 -m 2G -bios %s -cdrom %s -boot d -serial stdio -d int,cpu_reset,in_asm -D qemu.log -no-reboot -no-shutdown",
            ovmf_path,
            iso_file
         )
        --local qemu_cmd = string.format(
        --    "qemu-system-x86_64 -M q35 -m 2G -bios %s -cdrom %s -boot d -serial stdio -d int,cpu_reset,in_asm -D qemu.log -no-reboot -no-shutdown -s -S",
        --    ovmf_path,
        --    iso_file
        --)

        -- Run QEMU
        print("Running QEMU in " .. mode .. " mode with command: " .. qemu_cmd)
        os.exec(qemu_cmd)
    end)
    set_menu {
        usage = "xmake run",
        description = "Run the OS in QEMU"
    }

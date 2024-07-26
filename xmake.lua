set_project("mirus")
set_version("0.1.0")

includes("src/kernel")
includes("src/libs/limine")
includes("src/libs/ssfn")

task("make-iso")
    set_category("build")
    on_run(function ()
        import("core.project.project")
        import("core.project.config")

        -- Get the project root directory
        local project_root = os.projectdir()

        -- Get the build directory
        local build_dir = config.buildir()

        -- Ensure the project is built
        os.exec("xmake")

        -- Get the path to the kernel executable
        local kernel_target = project.target("kernel")
        if not kernel_target then
            raise("Kernel target not found!")
        end
        local kernel_path = kernel_target:targetfile()

        if not os.isfile(kernel_path) then
            local mode = config.mode() or "release"
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
        local iso_root = path.join(build_dir, "iso_root")
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
        local iso_file = path.join(build_dir, "image.iso")
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

        -- Get the build directory
        local build_dir = config.buildir()

        -- Path to the ISO file
        local iso_file = path.join(build_dir, "image.iso")

        -- Check if the ISO file exists
        if not os.isfile(iso_file) then
            -- If the ISO doesn't exist, try to create it
            print("ISO file not found. Attempting to create it...")
            os.exec("xmake make-iso")

            -- Check again if the ISO was created
            if not os.isfile(iso_file) then
                raise("Failed to create ISO file at " .. iso_file)
            end
        end

        -- Path to the OVMF BIOS file
        local ovmf_path = path.join(os.projectdir(), "meta", "OVMF_CODE.fd")

        -- Construct the QEMU command
        local qemu_cmd = string.format(
            "qemu-system-x86_64 -M q35 -m 2G -bios %s -cdrom %s -boot d -serial stdio",
            ovmf_path,
            iso_file
        )

        -- Run QEMU
        print("Running QEMU with command: " .. qemu_cmd)
        os.exec(qemu_cmd)
    end)
    set_menu {
        usage = "xmake run",
        description = "Run the OS in QEMU"
    }

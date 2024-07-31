target("hello")
    set_kind("binary")
    add_files("hello.c")
    add_cflags("-ffreestanding", "-fno-stack-protector", "-fno-pic", "-mno-red-zone", "-m64")
    add_cxflags("-nostdlib")
    set_arch("x86_64")
    add_cxflags("-fno-builtin")
    add_cxflags("-mstack-alignment=16")
    add_cxflags("-fomit-frame-pointer")
    add_cxflags("-Os")
    add_ldflags("-e _init", { force = true })

    after_build(function (target)
        local targetfile = target:targetfile()
        local binfile = targetfile .. ".bin"
        os.execv("otool", {"-l", targetfile})
        local result = os.iorunv("otool", {"-l", targetfile})
        local offset, size
        for line in result:gmatch("[^\r\n]+") do
            if line:match("sectname __text") then
                offset = line:match("offset (%x+)")
                size = line:match("size (%x+)")
                break
            end
        end
        if offset and size then
            offset = tonumber(offset, 16)
            size = tonumber(size, 16)
            os.execv("dd", {"if=" .. targetfile, "of=" .. binfile, "bs=1", "skip=" .. offset, "count=" .. size})
            print("Created flat binary: " .. binfile)
        else
            print("Failed to extract __text section information")
        end
    end)

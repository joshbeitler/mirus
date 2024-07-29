-- Get the project root directory
local project_root = os.projectdir()

-- Define the target for downloading and building printf
target("printf")
    set_kind("static")
    add_files("printf/*.c")
    add_defines("PRINTF_BUILD", {public = true})
    add_includedirs("$(projectdir)/src/libs/printf")
    add_cxflags("-DPRINTF_INCLUDE_CONFIG_H=1")

    on_load(function (target)
        import("net.http")

        -- Set the directories
        local printf_dir = path.join(project_root, "src", "libs", "printf", "printf")

        -- Create the directories if they don't exist
        os.mkdir(printf_dir)

        -- Define the files to download
        local files = {
            {url = "https://raw.githubusercontent.com/eyalroz/printf/master/src/printf/printf.h", filepath = path.join(printf_dir, "printf.h")},
            {url = "https://raw.githubusercontent.com/eyalroz/printf/master/src/printf/printf.c", filepath = path.join(printf_dir, "printf.c")}
        }

        -- Download the files
        for _, file in ipairs(files) do
            if not os.isfile(file.filepath) then
                print("Downloading " .. path.filename(file.filepath) .. "...")
                http.download(file.url, file.filepath)
            end
        end
    end)

    -- Add a clean rule to remove the downloaded files
    on_clean(function (target)
        local printf_dir = path.join(project_root, "src", "libs", "printf", "printf")
        os.rm(printf_dir)
    end)

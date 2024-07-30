-- Get the project root directory
local project_root = os.projectdir()

-- Define the Limine header target
target("limine")
    set_kind("phony")

    on_load(function (target)
        import("net.http")

        -- Set the directory for the Limine header
        local header_dir = path.join(project_root, "src", "libs", "limine", "include", "limine")

        -- Create the directory if it doesn't exist
        os.mkdir(header_dir)

        -- Download the Limine header
        local url = "https://github.com/limine-bootloader/limine/raw/trunk/limine.h"
        local filepath = path.join(header_dir, "limine.h")
        if not os.isfile(filepath) then
            print("Downloading Limine header...")
            http.download(url, filepath)
        end

        -- Add the include directory to the target
        target:add("includedirs", header_dir)
    end)

    -- Add a clean rule to remove the downloaded header
    on_clean(function (target)
        local header_path = path.join(project_root, "src", "libs", "limine", "include", "limine", "limine.h")
        os.rm(header_path)
    end)

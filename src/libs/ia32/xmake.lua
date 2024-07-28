-- Get the project root directory
local project_root = os.projectdir()

-- Define the IA32 header target
target("ia32")
    set_kind("phony")

    on_load(function (target)
        import("net.http")

        -- Set the directory for the IA32 header
        local header_dir = path.join(project_root, "src", "libs", "ia32", "include")

        -- Create the directory if it doesn't exist
        os.mkdir(header_dir)

        -- Download the IA32 header
        local url = "https://raw.githubusercontent.com/ia32-doc/ia32-doc/main/out/ia32.h"
        local filepath = path.join(header_dir, "ia32.h")
        if not os.isfile(filepath) then
            print("Downloading IA32 header...")
            http.download(url, filepath)
        end

        -- Add the include directory to the target
        target:add("includedirs", header_dir)
    end)

    -- Add a clean rule to remove the downloaded header
    on_clean(function (target)
        local header_path = path.join(project_root, "src", "libs", "ia32", "include", "ia32.h")
        os.rm(header_path)
    end)

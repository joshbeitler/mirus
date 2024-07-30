-- Get the project root directory
local project_root = os.projectdir()

-- Define the SSFN header target
target("ssfn")
    set_kind("phony")

    on_load(function (target)
        import("net.http")

        -- Set the directory for the SSFN header
        local header_dir = path.join(project_root, "src", "libs", "ssfn", "include", "ssfn")

        -- Create the directory if it doesn't exist
        os.mkdir(header_dir)

        -- Download the SSFN header
        local url = "https://gitlab.com/bztsrc/scalable-font2/-/raw/master/ssfn.h?ref_type=heads"
        local filepath = path.join(header_dir, "ssfn.h")
        if not os.isfile(filepath) then
            print("Downloading SSFN header...")
            http.download(url, filepath)
        end

        -- Add the include directory to the target
        target:add("includedirs", header_dir)
    end)

    -- Add a clean rule to remove the downloaded header
    on_clean(function (target)
        local header_path = path.join(project_root, "src", "libs", "ssfn", "include", "ssfn", "ssfn.h")
        os.rm(header_path)
    end)

-- INITIAL BLOCK DATA --

local blocks = {
    {
        name = "stone",
    },
    {
        name = "dirt",
    },
    {
        name = "grass",
    },
}

-- GENERATE EXTRA DATA --

local block_map = {}

for i, block in ipairs(blocks) do
    block.id = i
    block.enum = block.name:upper()
    block.stat_enum = "COLLECT_" .. block.name:upper()

    block_map[block.name] = block
end

-- UTIL --

local function open(path)
    local file = io.open("src/" .. path, "w")
    if not file then
        error("Could not open src/" .. path .. " for writing.")
    end

    file:write("// This file was generated automatically by generate.lua\n\n")

    if (path:sub(-1) == "h") then
        file:write("#pragma once\n")
        file:write("#include \"core.h\"\n\n")
    else
        file:write("#include \"" .. path:sub(0, -3) .. ".h\"\n\n")
    end

    return file
end

-- GENERATE block.h --

local f = open("block.h")

f:write("typedef enum\n{")
f:write("\n    AIR,")
for _, block in ipairs(blocks) do
    f:write("\n    ", block.enum, ",")
end
f:write("\n} Block;\n\n")

f:write("const char* block_to_string(Block b);\n")
f:write("const char* block_to_mc(Block b);\n")

f:close()

-- GENERATE block.h --

local f = open("block.c")

-- Enum to string
f:write("const char* block_to_string(Block b) {\n")
f:write("    if (b == AIR) return \"AIR\";\n")
for _, block in ipairs(blocks) do
    f:write("    if (b == ", block.enum, ") return \"", block.enum, "\";\n")
end
f:write("}\n\n")

f:write("const char* block_to_mc(Block b) {\n")
f:write("    if (b == AIR) return \"minecraft:air\";\n")
for _, block in ipairs(blocks) do
    f:write("    if (b == ", block.enum, ") return \"minecraft:", block.name, "\";\n")
end
f:write("}\n\n")

f:close()

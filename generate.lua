-- INITIAL BLOCK DATA --

local blocks = {
    {
        name = "stone",
    },
    {
        name = "dirt",
    },
    {
        name = "grass_block",
    },
    {
        name = "oak_log",
    },
    {
        name = "oak_leaves",
    }
}

-- GENERATE EXTRA DATA --

for i, block in ipairs(blocks) do
    block.id = i -- 0 is reserved for AIR
    block.enum = block.name:upper()
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

f:write("const char* block_to_string(Block b) {\n")
f:write("    if (b == AIR) return \"AIR\";\n")
for _, block in ipairs(blocks) do
    f:write("    if (b == ", block.enum, ") return \"", block.enum, "\";\n")
end
f:write("    UNREACHABLE;\n}\n\n")

f:write("const char* block_to_mc(Block b) {\n")
f:write("    if (b == AIR) return \"minecraft:air\";\n")
for _, block in ipairs(blocks) do
    f:write("    if (b == ", block.enum, ") return \"minecraft:", block.name, "\";\n")
end
f:write("    UNREACHABLE;\n}\n\n")

f:close()

-- GENERATE statistics.h --
local f = open("statistics.h")

f:write("#define NUM_OF_STATISTICS ", #blocks + 1, "\n\n")

f:write("typedef enum\n{\n")
for _, block in ipairs(blocks) do
    f:write("    BROKE_", block.enum, ",\n")
end
f:write("\n    MOVED,\n")
f:write("} StatName;\n\n")

f:write("typedef struct\n{\n")
f:write("    int stat[NUM_OF_STATISTICS];\n")
f:write("} Statistics;\n\n")

f:write("void init_scout_stats(Statistics *scout_stats);\n")

f:close()

-- GENERATE generated.h --
local f = open("generated.h")

f:write("#include \"block.h\"\n")
f:write("#include \"environment.h\"\n")
f:write("#include \"network.h\"\n")
f:write("#include \"statistics.h\"\n")
f:write("\n")

f:write(
    "size_t set_network_block_inputs(NetworkValues *values, const Environment environment, size_t next_node, Block block);\n")

f:write("void perform_dig_action(Environment* environment, Statistics* stats, Block block);")

f:close()

-- GENERATE generated.c --
local f = open("generated.c")

f:write(
    "size_t set_network_block_inputs(NetworkValues *values, const Environment environment, size_t next_node, Block block)\n{\n")
for _, block in ipairs(blocks) do
    f:write("    (*values)[next_node++] = block == ", block.enum, " ? 1 : 0;\n")
end
f:write("    return next_node;\n}\n\n")

f:write("void perform_dig_action(Environment* environment, Statistics* stats, Block block)\n{\n")
f:write("    if (block == AIR) return;\n")
for _, block in ipairs(blocks) do
    f:write("    else if (block == ", block.enum, ") stats->stat[BROKE_", block.enum, "]++;\n")
end
f:write("}\n\n")

f:close()

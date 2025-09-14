-- INITIAL BLOCK DATA --

local blocks = {
    {
        name = "stone",
        drops = "cobblestone",
    },
    {
        name = "cobblestone",
    },
    {
        name = "dirt",
    },
    {
        name = "grass",
        drops = "dirt",
    },
}

-- GENERATE EXTRA DATA --

local block_map = {}

for i, block in ipairs(blocks) do
    block.id = i
    block.enum = "BLOCK_" .. block.name:upper()
    block.stat_enum = "COLLECT_" .. block.name:upper()

    block_map[block.name] = block
end

-- GENERATE CODE --

local file = io.open("src/AUTO.c", "w")

if not file then
    error("Could not open src/AUTO.c for writing.")
end

local function out(...)
    file:write(...)
end

out("// This file is generated automatically by generate.lua\n\n")

-- NUM_OF_BLOCK
-- TODO: +1 to include AIR?
out("#define NUM_OF_BLOCK ", #blocks, "\n\n")

-- Block enum
out("typedef enum {")
for _, block in ipairs(blocks) do
    out("\n\t", block.enum, ",")
end
out("\n\tBLOCK_AIR")
out("\n} Block;\n\n")

-- Enum to string
out("const char* block_to_string(Block b) {\n")
out("\tif (b == BLOCK_AIR) return \"air\";\n")
for _, block in ipairs(blocks) do
    out("\tif (b == ", block.enum, ") return \"", block.name, "\";\n")
end
out("}\n\n")

-- Update stat on dig
out("void update_stat_on_dig(Statistics* stats, Block b) {\n")
out("\tif (b == BLOCK_AIR) return;\n")
for _, block in ipairs(blocks) do
    if block.drops then
        local drops = block_map[block.drops]
        out("\telse if (b == ", block.enum, ") stats->stat[", drops.stat_enum, "]++;\n")
    else
        out("\telse if (b == ", block.enum, ") stats->stat[", block.stat_enum, "]++;\n")
    end
end
out("}\n\n")

file:close()

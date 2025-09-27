-- ITEM DATA --

local items = {

    -- STONES --
    {
        name = "stone",
        is_block = true,

        drops = "cobblestone"
    },
    {
        name = "cobblestone",
        is_block = true,
    },

    -- DIRTS --
    {
        name = "dirt",
        is_block = true,
    },
    {
        name = "grass_block",
        is_block = true,

        drops = "dirt",
    },

    -- OAK TREES --
    {
        name = "oak_sapling",
        is_block = true,
    },
    {
        name = "oak_log",
        is_block = true,
        fuel_value = 15,
    },
    {
        name = "oak_leaves",
        is_block = true,

        -- TODO: I actually don't know if these chances are supposed to be mutually exclusive or not?
        drops = {
            { name = "stick",       chance = 1 / 100 },
            { name = "stick",       chance = 1 / 100 },
            { name = "oak_sapling", chance = 1 / 20 },
            { name = "apple",       chance = 1 / 5 },
        }
    },

    -- MISC --
    {
        name = "stick",
        fuel_value = 5,
    },
    {
        name = "apple",
    },
    {
        name = "diamond_pickaxe",
        stack_size = 1,
    },
}

-- GENERATE EXTRA DATA --

-- Populate default values and create mapping
local item_by_name = {}
for i, item in ipairs(items) do
    item.id = i -- 0 is reserved for AIR
    item.enum = item.name:upper()
    item_by_name[item.name] = item

    item.stack_size = item.stack_size or 64

    item.can_obtain_by_digging = false

    if item.is_block then
        item.broken_when_mined = true
        if item.drops == nil then
            item.drops = { { name = item.name, chance = 1 } }
            item.broken_when_mined = false

        elseif item.drops == false then
            item.drops = {}

        elseif type(item.drops) == "string" then
            item.drops = { { name = item.drops, chance = 1 } }
            item.broken_when_mined = false

        end
    end
end

-- Determine other information about items
local longest_name = 1

for i, item in ipairs(items) do
    if #item.name > longest_name then
        longest_name = #item.name
    end

    if item.is_block then
        for d, drop in ipairs(item.drops) do
            item_by_name[drop.name].can_obtain_by_digging = true
        end
    end
end

-- VALIDATE --

local function exists(name)
    return not (item_by_name[name] == nil)
end

local has_errors = false
for i, item in ipairs(items) do
    if item.is_block then
        for d, drop in ipairs(item.drops) do
            if not exists(drop.name) then
                has_errors = true
                print("Block \"" ..
                    item.name .. "\" drops \"" .. drop.name .. "\", however this item does not exist.")
            end
        end
    end
end

if has_errors then
    return
end

-- UTIL --

local function pad(name)
    return (" "):rep(longest_name - #name)
end

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

local function copy_file(src_path, dst_path)
    local src = io.open(src_path, "rb")
    if not src then
        error("Could not open " .. src_path .. " to copy from.")
    end
    local content = src:read("*all")
    src:close()

    local dst = io.open(dst_path, "wb")
    if not dst then
        error("Could not open " .. dst_path .. " to copy to.")
    end
    dst:write(content)
    dst:close()
end

-- GENERATE item.h --

local f = open("item.h")

f:write("typedef enum\n{")
f:write("\n    AIR,")
for _, item in ipairs(items) do
    f:write("\n    ", item.enum, ",")
end
f:write("\n} Item;\n\n")

f:write("const char* item_to_string(Item item);\n")
f:write("const char* item_to_mc(Item item);\n")
f:write("\n")
f:write("size_t stack_size_of(Item item);\n")
f:write("size_t fuel_value_of(Item item);\n")

f:close()

-- GENERATE item.c --

local f = open("item.c")

f:write("const char* item_to_string(Item item) {\n")
f:write("    switch (item)\n    {\n")
f:write("        case AIR:", pad("AIR"), " return \"AIR\";\n")
for _, item in ipairs(items) do
    f:write("        case ", item.enum, ":", pad(item.enum), " return \"", item.enum, "\";\n")
end
f:write("    }\n")
f:write("    UNREACHABLE;\n}\n\n")

f:write("const char* item_to_mc(Item item) {\n")
f:write("    switch (item)\n    {\n")
f:write("        case AIR:", pad("AIR"), " return \"minecraft:air\";\n")
for _, item in ipairs(items) do
    f:write("        case ", item.enum, ":", pad(item.enum), " return \"minecraft:", item.name, "\";\n")
end
f:write("    }\n")
f:write("    UNREACHABLE;\n}\n\n")

f:write("size_t stack_size_of(Item item)\n{\n")
f:write("    switch (item)\n    {\n")
f:write("        case AIR:", pad("AIR"), " UNREACHABLE;\n")
for _, item in ipairs(items) do
    if item.stack_size ~= 64 then
        f:write("        case ", item.enum, ":", pad(item.enum), " return ", tostring(item.stack_size), ";\n")
    end
end
f:write("        default:", pad("lt"), " return 64;\n")
f:write("    }\n")
f:write("}\n\n")

f:write("size_t fuel_value_of(Item item)\n{\n")
f:write("    switch (item)\n    {\n")
f:write("        case AIR:", pad("AIR"), " UNREACHABLE;\n")
for _, item in ipairs(items) do
    if item.fuel_value then
        f:write("        case ", item.enum, ":", pad(item.enum), " return ", tostring(item.fuel_value), ";\n")
    end
end
f:write("        default:", pad("lt"), " return 0;\n")
f:write("    }\n")
f:write("}\n\n")

f:close()

-- GENERATE statistics.h --
local f = open("statistics.h")

local stat_count = 0

f:write("typedef enum\n{\n")

for _, item in ipairs(items) do
    if item.can_obtain_by_digging then
        f:write("    ", item.enum, "_OBTAINED_BY_MINING,\n")
        stat_count = stat_count + 1
    end
end
f:write("\n")

for _, item in ipairs(items) do
    if item.broken_when_mined then
        f:write("    ", item.enum, "_BROKEN,\n")
        stat_count = stat_count + 1
    end
end
f:write("\n")

for _, item in ipairs(items) do
    if item.fuel_value then
        f:write("    ", item.enum, "_USED_AS_FUEL,\n")
        stat_count = stat_count + 1
    end
end
f:write("\n")

stat_count = stat_count + 1
f:write("    MOVED,\n")
f:write("} StatName;\n\n")

f:write("#define NUM_OF_STATISTICS ", stat_count, "\n\n")

f:write("typedef struct\n{\n")
f:write("    int stat[NUM_OF_STATISTICS];\n")
f:write("} Statistics;\n\n")

f:write("void init_scout_stats(Statistics *scout_stats);\n")
f:write("const char* stat_name_to_string(StatName name);\n")

f:close()

-- GENERATE statistics.c --
local f = open("statistics.c")

f:write("void init_scout_stats(Statistics *scout_stats)\n")
f:write("{\n")
f:write("    for (size_t i = 0; i < NUM_OF_STATISTICS; i++)\n")
f:write("        scout_stats->stat[i] = 0;\n")
f:write("}\n\n")

f:write("const char* stat_name_to_string(StatName name) {\n")
f:write("    switch(name)\n    {\n")

for _, item in ipairs(items) do
    if item.can_obtain_by_digging then
        f:write("        case ", item.enum, "_OBTAINED_BY_MINING: return \"", item.enum, "_OBTAINED_BY_MINING\";\n")
    end
end

for _, item in ipairs(items) do
    if item.broken_when_mined then
        f:write("        case ", item.enum, "_BROKEN: return \"", item.enum, "_BROKEN\";\n")
    end
end

for _, item in ipairs(items) do
    if item.fuel_value then
        f:write("        case ", item.enum, "_USED_AS_FUEL: return \"", item.enum, "_USED_AS_FUEL\";\n")
    end
end
f:write("\n")

f:write("        case MOVED: return \"MOVED\";\n")
f:write("    }\n")
f:write("    UNREACHABLE;\n}\n\n")

f:close()

-- GENERATE generated.h --
local f = open("generated.h")

f:write("#include \"item.h\"\n")
f:write("#include \"environment.h\"\n")
f:write("#include \"network.h\"\n")
f:write("#include \"statistics.h\"\n")
f:write("\n")

f:write(
    "void set_network_inputs_for_item(NetworkValues *values, const Environment environment, size_t *next_node, Item item);\n")

f:write("void update_dig_action(Environment* environment, Statistics* stats, Item block);\n")

f:write("void update_refuel_stat(Statistics* stats, Item item);\n")

f:close()

-- GENERATE generated.c --
local f = open("generated.c")

f:write(
    "void set_network_inputs_for_item(NetworkValues *values, const Environment environment, size_t *next_node, Item item)\n{\n")
for _, item in ipairs(items) do
    f:write("    set_network_value(values, (*next_node)++, item == ", item.enum, ");\n")
end
f:write("}\n\n")

f:write("void update_dig_action(Environment* environment, Statistics* stats, Item block)\n{\n")
f:write("    bool success;\n")
f:write("    switch (block)\n    {\n")
f:write("    case AIR:\n        return;\n\n")
for _, item in ipairs(items) do
    if item.is_block then
        f:write("    case ", item.enum, ":\n")

        -- FIXME: This way of doing it won't work for gravel
        -- NOTE:  May also need to make chances to how `broken_when_mined` is calculated
        if item.broken_when_mined then
            f:write("        stats->stat[", item.enum, "_BROKEN]++;\n")
        end

        if #item.drops == 0 then
            f:write("        return;\n\n")

        elseif #item.drops == 1 and item.drops[1].chance == 1 then
            local drop = item_by_name[item.drops[1].name]
            f:write("        success = add_to_scout_inventory(environment, ", drop.enum, ");\n")
            f:write("        if (success) stats->stat[", drop.enum, "_OBTAINED_BY_MINING]++;\n")
            f:write("        break;\n\n")

        else
            for d, drop_data in ipairs(item.drops) do
                local drop = item_by_name[drop_data.name]
                -- FIXME: This way of calculating the chance of an item being dropped is a bit squiffy.
                --        It works as of writing, but might need fixing later?
                f:write("        if (fast_rand() % 100 < ", math.ceil(drop_data.chance * 100), ")\n")
                f:write("        {\n")
                f:write("            success = add_to_scout_inventory(environment, ", drop.enum, ");\n")
                f:write("            if (success) stats->stat[", drop.enum, "_OBTAINED_BY_MINING]++;\n")
                f:write("        }\n")
            end
            f:write("        break;\n\n")
        end
    end
end
f:write("    }\n")
f:write("}\n\n")

f:write("void update_refuel_stat(Statistics* stats, Item item)\n{\n")
f:write("    switch (item)\n    {\n")
for _, item in ipairs(items) do
    if item.fuel_value then
        f:write("        case ", item.enum, ":\n")
        f:write("            stats->stat[", item.enum, "_USED_AS_FUEL]++;\n")
        f:write("            break;\n")
    end
end
f:write("        default: UNREACHABLE;\n")
f:write("    }\n")
f:write("}\n\n")

f:close()

-- GENERATE scout.lua --

local t = io.open("scout/scout_template.lua", "r")
if not t then
    error("Could not open scout/scout_template.lua for reading.")
end

local f = io.open("scout/scout.lua", "w")
if not f then
    error("Could not open scout/scout.lua for writing.")
end

f:write("-- This file was generated automatically by generate.lua based on the contents of scout_template.lua\n\n")

for line in t:lines("L") do
    if line == "    -- [[INSERT GENERATED CODE]]\n" then
        for i, item in ipairs(items) do
            f:write("    set_input(item == \"minecraft:", item.name, "\")\n")
        end
    elseif line:sub(1, 14) == "---@diagnostic" then
        -- pass
    else
        f:write(line)
    end
end

t:close()
f:close()

copy_file("scout/scout.lua", "export/scout.lua")

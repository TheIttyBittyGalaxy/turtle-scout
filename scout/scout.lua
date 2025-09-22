-- This file was generated automatically by generate.lua based on the contents of scout_template.lua

-- ACTIONS

-- TODO: Add refuel action
local Actions = {
    "IDLE",

    "TURN_LEFT",
    "TURN_RIGHT",

    "MOVE",
    "MOVE_UP",
    "MOVE_DOWN",

    "DIG",
    "DIG_UP",
    "DIG_DOWN",

    "SELECT_FIRST_SLOT",
    "SELECT_NEXT_SLOT",
}

-- NETWORK
local node = {}
local result = {}

local node_count = 0
local activations = {}
local inhibitions = {}

do
    local byte_file = io.open("network.bytes", "rb")
    if not byte_file then
        error("Could not open network.bytes")
    end

    local header = byte_file:read(8)
    local num_clusters, cluster_size = string.unpack("<I4I4", header)
    node_count = num_clusters * cluster_size

    local function bit_is_set(value, bit)
        return math.floor(value / (2 ^ bit)) % 2 == 1
    end

    local function read_connections(connection)
        for i = 1, node_count do
            connection[i] = {}
            for cluster = 1, num_clusters do
                local data = byte_file:read(cluster_size / 8)
                for byte_index = 1, #data do
                    local byte = data:byte(byte_index)
                    for bit = 0, 7 do
                        local j = (cluster - 1) * cluster_size + (byte_index - 1) * 8 + bit + 1
                        connection[i][j] = bit_is_set(byte, bit)
                    end
                end
            end
        end
    end

    read_connections(activations)
    read_connections(inhibitions)

    byte_file:close()
end

for i = 1, node_count do
    node[i] = false
    result[i] = false
end

-- MAIN LOOP
local log = io.open("cc_action_log.csv", "w")
if not log then
    error("Could not open cc_action_log.csv for writing.")
end

local net_log = io.open("cc_network_log.csv", "w")
if not net_log then
    error("Could not open cc_network_log.csv for writing.")
end

local function node_as_num(i)
    if node[i] then
        return "1"
    else
        return "0"
    end
end

net_log:write(node_as_num(1))
for i = 2, node_count do
    net_log:write(",", node_as_num(i))
end
net_log:write("\n")

local next_node

local function set_bias()
    node[1] = true
    next_node = 2
end

local function set_input(value)
    node[next_node] = value
    next_node = next_node + 1
end

local function set_inputs_for_item(item)
    set_input(item == "minecraft:stone")
    set_input(item == "minecraft:cobblestone")
    set_input(item == "minecraft:dirt")
    set_input(item == "minecraft:grass_block")
    set_input(item == "minecraft:oak_sapling")
    set_input(item == "minecraft:oak_log")
    set_input(item == "minecraft:oak_leaves")
    set_input(item == "minecraft:stick")
    set_input(item == "minecraft:apple")
    set_input(item == "minecraft:diamond_pickaxe")
end

-- while true do
for iteration = 1, 128 do

    -- Set inputs
    local is_front, front = turtle.inspect()
    local is_above, above = turtle.inspectUp()
    local is_below, below = turtle.inspectDown()
    local selected = turtle.getItemDetail()

    set_bias()
    set_inputs_for_item(is_front and front.name or "minecraft:air")
    set_inputs_for_item(is_above and above.name or "minecraft:air")
    set_inputs_for_item(is_below and below.name or "minecraft:air")
    set_inputs_for_item(is_below and below.name or "minecraft:air")
    set_inputs_for_item(selected and selected.name or "minecraft:air")

    -- Calculate results
    for i = 1, node_count do
        local value = false
        for j = 1, node_count do
            if node[j] then
                if inhibitions[i][j] then
                    value = false
                    break
                end

                if activations[i][j] then
                    value = true
                end
            end
        end
        result[i] = value
    end

    node, result = result, node

    -- Determine next action
    local action = "IDLE"
    for i = 2, #Actions do
        local n = (node_count - i) + 2
        if node[n] then
            action = Actions[i]
            break
        end
    end

    -- Perform action
    local success = true
    if action == "IDLE" then
        -- Pass

    elseif action == "TURN_LEFT" then
        turtle.turnLeft()
    elseif action == "TURN_RIGHT" then
        turtle.turnRight()

    elseif action == "MOVE" then
        success = turtle.forward()
    elseif action == "MOVE_UP" then
        success = turtle.up()
    elseif action == "MOVE_DOWN" then
        success = turtle.down()

    elseif action == "DIG" then
        success = turtle.dig()
    elseif action == "DIG_UP" then
        success = turtle.digUp()
    elseif action == "DIG_DOWN" then
        success = turtle.digDown()

    elseif action == "SELECT_FIRST_SLOT" then
        success = turtle.select(1)
    elseif action == "SELECT_NEXT_SLOT" then
        local slot = turtle.getSelectedSlot() + 1
        if slot > 16 then
            slot = 1
        end
        success = turtle.select(slot)
    end

    set_bias()
    set_inputs_for_item(is_front and front.name or "minecraft:air")
    set_inputs_for_item(is_above and above.name or "minecraft:air")
    set_inputs_for_item(is_below and below.name or "minecraft:air")
    set_inputs_for_item(is_below and below.name or "minecraft:air")
    set_inputs_for_item(selected and selected.name or "minecraft:air")

    -- Log
    log:write(
        iteration - 1, ",",
        is_front and front.name:sub(11):upper() or "AIR", ",",
        is_above and above.name:sub(11):upper() or "AIR", ",",
        is_below and below.name:sub(11):upper() or "AIR", ",",
        action, ",",
        tostring(success), "\n")

    net_log:write(node_as_num(1))
    for i = 2, node_count do
        net_log:write(",", node_as_num(i))
    end
    net_log:write("\n")

end

log:close()
net_log:close()

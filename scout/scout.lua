-- This file was generated automatically by generate.lua based on the contents of scout_template.lua


-- ACTIONS --

local Actions = {
    "REFUEL",

    "SELECT_FIRST_SLOT",
    "SELECT_NEXT_SLOT",

    "DIG_FORWARD",
    "DIG_UP",
    "DIG_DOWN",

    "MOVE_FORWARD",
    "MOVE_UP",
    "MOVE_DOWN",

    "TURN_LEFT",
    "TURN_RIGHT",
}

local ActionIndex = {}
for i, action in ipairs(Actions) do
    ActionIndex[action] = i
end

-- NETWORK --

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

-- ACTION LOG --

local action_log = io.open("cc_action_log.csv", "w")
if not action_log then
    error("Could not open cc_action_log.csv for writing.")
end

action_log:write("#,Front,Above,Below")
for _, action in ipairs(Actions) do
    action_log:write(action)
end
action_log:write(",Fuel\n")

-- NETWORK LOG --

local function node_as_num(i)
    if node[i] then
        return "1"
    else
        return "0"
    end
end

local network_log = io.open("cc_network_log.csv", "w")
if not network_log then
    error("Could not open cc_network_log.csv for writing.")
end

network_log:write(node_as_num(1))
for i = 2, node_count do
    network_log:write(",", node_as_num(i))
end
network_log:write("\n")

-- SET NETWORK INPUTS --

local next_input

local function set_bias()
    node[1] = true
    next_input = 2
end

local function set_input(value)
    node[next_input] = value
    next_input = next_input + 1
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

-- PERFORM NETWORK OUTPUTS --

local outcome = {}

local function do_action(action, funct)
    local i = ActionIndex[action]
    if node[node_count - i + 1] then
        local success = funct()
        outcome[i] = success and "PASS" or "FAIL"
        return true
    end
    return false
end

-- MAIN LOOP --

for iteration = 1, 128 do
    -- Data collection
    local is_front, front = turtle.inspect()
    local is_above, above = turtle.inspectUp()
    local is_below, below = turtle.inspectDown()
    local selected = turtle.getItemDetail()

    -- Set network inputs
    set_bias()
    set_inputs_for_item(is_front and front.name or "minecraft:air")
    set_inputs_for_item(is_above and above.name or "minecraft:air")
    set_inputs_for_item(is_below and below.name or "minecraft:air")
    set_inputs_for_item(selected and selected.name or "minecraft:air")

    -- Evaluate network
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
    for i = 1, #Actions do
        outcome[i] = ""
    end

    do_action("REFUEL", function()
        turtle.refuel(1)
        return true
    end)

    if do_action("SELECT_FIRST_SLOT", function()
            turtle.select(1)
            return true
        end) then
    else
        do_action("SELECT_NEXT_SLOT", function()
            local slot = turtle.getSelectedSlot() + 1
            if slot > 16 then
                slot = 1
            end
            turtle.select(slot)
            return true
        end)
    end

    do_action("DIG_FORWARD", turtle.dig)
    do_action("DIG_UP", turtle.digUp)
    do_action("DIG_DOWN", turtle.digDown)

    if do_action("MOVE_FORWARD", turtle.forward) then
    elseif do_action("MOVE_UP", turtle.up) then
    elseif do_action("MOVE_DOWN", turtle.down) then
    elseif do_action("TURN_LEFT", turtle.turnLeft) then
    elseif do_action("TURN_RIGHT", turtle.turnRight) then
    end

    -- Update action log
    action_log:write(
        iteration - 1, ",",
        is_front and front.name:sub(11):upper() or "AIR", ",",
        is_above and above.name:sub(11):upper() or "AIR", ",",
        is_below and below.name:sub(11):upper() or "AIR", ","
    )

    for i = 1, #Actions do
        action_log:write(outcome[i])
    end

    action_log:write(turtle.getFuelLevel(), "\n")

    -- Update network log
    set_bias()
    set_inputs_for_item(is_front and front.name or "minecraft:air")
    set_inputs_for_item(is_above and above.name or "minecraft:air")
    set_inputs_for_item(is_below and below.name or "minecraft:air")
    set_inputs_for_item(selected and selected.name or "minecraft:air")

    network_log:write(node_as_num(1))
    for i = 2, node_count do
        network_log:write(",", node_as_num(i))
    end
    network_log:write("\n")
end

action_log:close()
network_log:close()

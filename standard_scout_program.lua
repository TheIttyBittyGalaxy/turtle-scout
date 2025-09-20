-- ACTIONS
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
}

-- NETWORK
local network = {}
local node = {}
local result = {}

do
    local byte_file = io.open("network.bytes", "rb")
    if not byte_file then
        error("Could not open network.bytes")
    end

    local function read_unsigned_int()
        local bytes = byte_file:read(8)
        return string.unpack("I8", bytes)
    end

    local function read_bool_array(n)
        local array = {}
        for i = 1, n do
            local byte = byte_file:read(1)
            local value = string.unpack("B", byte)
            array[i] = value ~= 0
        end
        return array
    end

    local nodes = read_unsigned_int()

    local activations = {}
    for i = 1, nodes do
        activations[i] = read_bool_array(nodes)
    end

    local inhibitions = {}
    for i = 1, nodes do
        inhibitions[i] = read_bool_array(nodes)
    end

    network.nodes = nodes
    network.activations = activations
    network.inhibitions = inhibitions

    byte_file:close()
end

for i = 1, network.nodes do
    node[i] = false
    result[i] = false
end

-- MAIN LOOP
local log = io.open("cc_simulation_log.csv", "w")
if not log then
    error("Could not open cc_simulation_log.csv for writing.")
end

local net_log = io.open("cc_net_log.csv", "w")
if not net_log then
    error("Could not open cc_net_log.csv for writing.")
end

local function node_as_num(i)
    if node[i] then
        return "1"
    else
        return "0"
    end
end

net_log:write(node_as_num(1))
for i = 2, network.nodes do
    net_log:write(",", node_as_num(i))
end
net_log:write("\n")

-- while true do
for iteration = 1, 128 do

    -- Set inputs
    local is_front, front = turtle.inspect()
    local is_above, above = turtle.inspectUp()
    local is_below, below = turtle.inspectDown()

    -- TODO: Generate this in generate.lua
    node[1] = true

    node[2] = is_front and front.name == "minecraft:stone"
    node[3] = is_front and front.name == "minecraft:dirt"
    node[4] = is_front and front.name == "minecraft:grass_block"
    node[5] = is_front and front.name == "minecraft:oak_log"
    node[6] = is_front and front.name == "minecraft:oak_leaves"

    node[7] = is_above and above.name == "minecraft:stone"
    node[8] = is_above and above.name == "minecraft:dirt"
    node[9] = is_above and above.name == "minecraft:grass_block"
    node[10] = is_above and above.name == "minecraft:oak_log"
    node[11] = is_above and above.name == "minecraft:oak_leaves"

    node[12] = is_below and below.name == "minecraft:stone"
    node[13] = is_below and below.name == "minecraft:dirt"
    node[14] = is_below and below.name == "minecraft:grass_block"
    node[15] = is_below and below.name == "minecraft:oak_log"
    node[16] = is_below and below.name == "minecraft:oak_leaves"

    -- Calculate results
    for i = 1, network.nodes do
        local value = false
        for j = 1, network.nodes do
            if node[j] then
                if network.inhibitions[j][i] then
                    value = false
                    break
                end

                if network.activations[j][i] then
                    value = true
                end
            end
        end
        result[i] = value
    end

    node, result = result, node

    -- Determine next action
    local action = "IDLE"
    local highest_activation = 0

    for i = 2, #Actions do
        local n = (network.nodes - i) + 2
        if node[n] then
            action = Actions[i]
            break
        end
    end

    -- Perform action
    if action == "IDLE" then
        -- Pass

    elseif action == "TURN_LEFT" then
        turtle.turnLeft()
    elseif action == "TURN_RIGHT" then
        turtle.turnRight()

    elseif action == "MOVE" then
        turtle.forward()
    elseif action == "MOVE_UP" then
        turtle.up()
    elseif action == "MOVE_DOWN" then
        turtle.down()

    elseif action == "DIG" then
        turtle.dig()
    elseif action == "DIG_UP" then
        turtle.digUp()
    elseif action == "DIG_DOWN" then
        turtle.digDown()
    end

    -- Log
    log:write(
        iteration - 1, ",",
        is_front and front.name:sub(11):upper() or "AIR", ",",
        is_above and above.name:sub(11):upper() or "AIR", ",",
        is_below and below.name:sub(11):upper() or "AIR", ",",
        action, "\n")

    net_log:write(node_as_num(1))
    for i = 2, network.nodes do
        net_log:write(",", node_as_num(i))
    end
    net_log:write("\n")

end

log:close()
net_log:close()

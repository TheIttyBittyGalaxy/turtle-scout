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

    local function read_double_array(n)
        local array = {}
        for i = 1, n do
            local bytes = byte_file:read(8)
            array[i] = string.unpack("d", bytes)
        end
        return array
    end

    local nodes = read_unsigned_int()
    local bias = read_double_array(nodes)
    local weight = {}
    for i = 1, nodes do
        weight[i] = read_double_array(nodes)
    end

    network.nodes = nodes
    network.bias = bias
    network.weight = weight

    byte_file:close()
end


-- local function activation(x)
--     if (x > 1) then return 1 end
--     if (x < -1) then return -1 end
--     return x
-- end

local function activation(x)
    if (x > 0) then return 1 end
    return 0
end

for i = 1, network.nodes do
    node[i] = activation(network.bias[i])
    result[i] = 0
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

net_log:write(node[1])
for i = 2, network.nodes do
    net_log:write(",", node[i])
end
net_log:write("\n")

-- while true do
for iteration = 1, 128 do

    -- Set inputs
    local is_front, front = turtle.inspect()
    local is_above, above = turtle.inspectUp()
    local is_below, below = turtle.inspectDown()

    -- TODO: Generate this in generate.lua
    node[1] = is_front and front.name == "minecraft:stone" and 1 or 0
    node[2] = is_front and front.name == "minecraft:dirt" and 1 or 0
    node[3] = is_front and front.name == "minecraft:grass_block" and 1 or 0
    node[4] = is_front and front.name == "minecraft:oak_log" and 1 or 0
    node[5] = is_front and front.name == "minecraft:oak_leaves" and 1 or 0

    node[6] = is_front and front.name == "minecraft:stone" and 1 or 0
    node[7] = is_front and front.name == "minecraft:dirt" and 1 or 0
    node[8] = is_front and front.name == "minecraft:grass_block" and 1 or 0
    node[9] = is_front and front.name == "minecraft:oak_log" and 1 or 0
    node[10] = is_front and front.name == "minecraft:oak_leaves" and 1 or 0

    node[11] = is_front and front.name == "minecraft:stone" and 1 or 0
    node[12] = is_front and front.name == "minecraft:dirt" and 1 or 0
    node[13] = is_front and front.name == "minecraft:grass_block" and 1 or 0
    node[14] = is_front and front.name == "minecraft:oak_log" and 1 or 0
    node[15] = is_front and front.name == "minecraft:oak_leaves" and 1 or 0

    -- Calculate results
    for i = 1, network.nodes do
        local value = network.bias[i]
        for j = 1, network.nodes do
            value = value + node[j] * network.weight[j][i]
        end
        result[i] = activation(value)
    end

    node, result = result, node

    -- Determine next action
    local action = "IDLE"
    local highest_activation = 0

    for i = 2, #Actions do
        local n = (network.nodes - i) + 2
        if node[n] > highest_activation then
            action = Actions[i]
            highest_activation = node[n]
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

    net_log:write(node[1])
    for i = 2, network.nodes do
        net_log:write(",", node[i])
    end
    net_log:write("\n")

end

log:close()
net_log:close()

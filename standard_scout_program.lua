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
local node = {}
local result = {}

-- local function activation(x)
--     if (x > 1) then return 1 end
--     if (x < -1) then return -1 end
--     return x
-- end

local function activation(x)
    if (x > 0) then return 1 end
    return 0
end


for i = 1, parameters.nodes do
    node[i] = activation(parameters.bias[i])
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
for i = 2, parameters.nodes do
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
    for i = 1, parameters.nodes do
        local value = parameters.bias[i]
        for j = 1, parameters.nodes do
            value = value + node[j] * parameters.weight[j][i]
        end
        result[i] = activation(value)
    end

    node, result = result, node

    -- Determine next action
    local action = "IDLE"
    local highest_activation = 0

    for i = 2, #Actions do
        local n = (parameters.nodes - i) + 2
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
    for i = 2, parameters.nodes do
        net_log:write(",", node[i])
    end
    net_log:write("\n")

end

log:close()
net_log:close()

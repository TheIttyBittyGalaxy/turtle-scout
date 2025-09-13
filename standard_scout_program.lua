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

for i = 1, parameters.nodes do
    node[i] = parameters.bias[i]
    result[i] = 0
end

-- MAIN LOOP
while true do

    -- Set inputs
    local is_front, front = turtle.inspect()
    local is_above, above = turtle.inspectUp()
    local is_below, below = turtle.inspectDown()

    node[1] = is_front and front.name == "minecraft:stone" and 1 or 0
    node[2] = is_front and front.name == "minecraft:dirt" and 1 or 0
    node[3] = is_front and front.name == "minecraft:grass_block" and 1 or 0

    node[4] = is_above and above.name == "minecraft:stone" and 1 or 0
    node[5] = is_above and above.name == "minecraft:dirt" and 1 or 0
    node[6] = is_above and above.name == "minecraft:grass_block" and 1 or 0

    node[7] = is_below and below.name == "minecraft:stone" and 1 or 0
    node[8] = is_below and below.name == "minecraft:dirt" and 1 or 0
    node[9] = is_below and below.name == "minecraft:grass_block" and 1 or 0

    -- Calculate results
    for i = 1, parameters.nodes do
        local value = parameters.bias[i]
        for j = 1, parameters.nodes do
            value = value + node[j] * parameters.weight[j][i]
        end
        result[i] = value
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
end

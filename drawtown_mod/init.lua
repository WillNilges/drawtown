Modpath = minetest.get_modpath(minetest.get_current_modname())

-- Handle mod security if needed
local ie, req_ie = _G, minetest.request_insecure_environment
if req_ie then ie = req_ie() end
if not ie then
    error("The mod requires access to insecure functions in order "..
          "to work.  Please add the mod to your secure.trusted_mods "..
          "setting in minetest.conf or disable the mod.")
end

-- The build of Lua that Minetest comes with only looks for libraries under
-- /usr/local/share and /usr/local/lib but LuaSocket is often installed under
-- /usr/share and /usr/lib.
if not rawget(_G, "jit") and package.config:sub(1, 1) == "/" then

    ie.package.path = ie.package.path..
        ";/usr/share/lua/5.4/?.lua"..
        ";/usr/share/lua/5.4/?/init.lua"

    ie.package.cpath = ie.package.cpath..
        ";/usr/lib/lua/5.4/?.so"..
        ";/usr/lib64/lua/5.4/?.so"

    ie.package.cpath = "/usr/lib/x86_64-linux-gnu/lua/5.4/?.so;"..ie.package.cpath

end

Schematics = {}
for _, sch in ipairs({"cabin","tower","tent", "shed"}) do
    Schematics[sch] = minetest.register_schematic("schems/"..sch..".mts")
end

local old_require = require
require = ie.require

local socket = require("socket")

local server = assert(socket.tcp())

local clients = {}

local pn = math.random(0,9)
assert(server:bind("127.0.0.1",30210+pn))
server:settimeout(0.01)
assert(server:listen())

local function place_schematic(name, coords)
    print("Placing schematic "..name.."@")
    for k, v in pairs(coords) do
        print(k,v)
    end
    if Schematics[name] ~= nil then
        minetest.place_schematic(coords, Schematics[name], 0, {}, true, 0)
        return nil
    else
        return name.." does not exist."
    end
end

local function place_block(name, coords)
    print("Placing "..name.."@")
    for k, v in pairs(coords) do
        print(k,v)
    end
    if minetest.registered_nodes[name] ~= nil then
        worldedit.set(coords,coords,name)
        return nil
    else
        return name.." does not exist."
    end
end

local function fill_area(name, coords1, coords2)
    print("Filling with "..name)
    coords1["y"] = 0
    coords2["y"] = 1
    if minetest.registered_nodes[name] ~= nil then
        worldedit.set(coords1,coords2,name)
        return nil
    else
        return name.." does not exist."
    end
end

local function reset_area(name, coords1, coords2)
    print("Resetting area.")
    if minetest.registered_nodes[name] ~= nil then
        coords1["y"] = 0
        coords2["y"] = 1
        worldedit.set(coords1,coords2,name)
        coords1["y"] = 2
        coords2["y"] = 100
        worldedit.set(coords1,coords2,"air")
        return nil
    else
        return name.." does not exist."
    end
end

handlers = {}
handlers[1] = {}
handlers[1]["s"] = place_schematic
handlers[1]["b"] = place_block
handlers[2] = {}
handlers[2]["f"] = fill_area
handlers[2]["r"] = reset_area

local function step()
    local client, err = server:accept();

    if (not err) then
        client:settimeout(0.01)
        table.insert(clients, client)
        print(">> Connection from " .. tostring(client:getsockname()) .." at " ..os.date("%m/%d/%Y %H:%M:%S"))
        print("   "..#clients.." clients")
    end

    for client_i, client in ipairs(clients) do
        if client ~= nil then
            local data, err = client:receive()
            if err and err ~= 'timeout' then
                print("<< Disconnect from " .. tostring(client:getsockname()) .." at " ..os.date("%m/%d/%Y %H:%M:%S"))
                client:shutdown()
                client:close()
                table.remove(clients, client_i)
                print(".. "..#clients.." clients")
            end

            if not err then
                print("Got line: " .. data)

                local cmd, name, x1, z1, x2, z2 = string.match(data, "(%a) ([%w:]*)@(%d*),(%d*) (%d*),(%d*)")
                if cmd ~= nil then
                    local coords1 = {}
                    coords1["x"]=tonumber(x1) or 0
                    coords1["z"]=tonumber(z1) or 0

                    for i = 96, -1, -1 do
                        coords1["y"]=i
                        if minetest.get_node(coords1).name ~= "air" and
                           minetest.get_node(coords1).name ~= "ignore" then
                            break
                        end
                    end

                    local coords2 = {}
                    coords2["x"]=tonumber(x2) or 0
                    coords2["z"]=tonumber(z2) or 0

                    for i = 96, -1, -1 do
                        coords2["y"]=i
                        if minetest.get_node(coords2).name ~= "air" and
                           minetest.get_node(coords2).name ~= "ignore" then
                            break
                        end
                    end

                    if handlers[2][cmd] ~= nil then
                        local err = handlers[2][cmd](name, coords1, coords2)
                        if err then
                            print("Command failed with: "..tostring(err))
                        else
                            print("Command suceeded.")
                        end
                    else
                        print("Invalid command"..cmd..".")
                    end
                    return
                end

                local cmd, name, x, z = string.match(data, "(%a) ([%w:]*)@(%d*),(%d*)")
                if cmd ~= nil then
                    local coords = {}
                    coords["x"]=tonumber(x) or 0
                    coords["z"]=tonumber(z) or 0

                    for i = 96, -1, -1 do
                        coords["y"]=i
                        if minetest.get_node(coords).name ~= "air" and
                           minetest.get_node(coords).name ~= "ignore" then
                            break
                        end
                    end

                    if handlers[1][cmd] ~= nil then
                        local err = handlers[1][cmd](name, coords)
                        if err then
                            print("Command failed with: "..tostring(err))
                        else
                            print("Command suceeded.")
                        end
                    else
                        print("Invalid command"..cmd..".")
                    end
                    return
                end
            end
        end
    end
end

local function cleanup()
    for _, client in ipairs(clients) do
        if client ~= nil then
            client:shutdown()
            client:close()
        end
    end
    server:close()
end

minetest.register_globalstep(function(dtime) return step(dtime) end)
minetest.register_on_shutdown(function() cleanup() end)

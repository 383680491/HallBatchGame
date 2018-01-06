local zb = ''  --制表符

local function serialize(_t)
    if _t == nil then
        return "[nil]"
    end

    local szRet = "{"
    szRet = szRet .. '\n'
    zb = zb .. '\t'
    local temp = zb
    szRet = szRet .. temp
    function doT2S(_i, _v)
        if "number" == type(_i) then
            szRet = szRet .. "[" .. _i .. "] = "
            if "number" == type(_v) then
                szRet = szRet .. _v .. ","
            elseif "string" == type(_v) then
                szRet = szRet .. '"' .. _v .. '"' .. ","
            elseif "table" == type(_v) then
                szRet = szRet .. serialize(_v) .. ","
            else
                szRet = szRet .. "nil,"
            end
        elseif "string" == type(_i) then
            szRet = szRet .. '["' .. _i .. '"] = '
            if "number" == type(_v) then
                szRet = szRet .. _v .. ","
                szRet = szRet .. '\n'
                szRet = szRet .. temp
            elseif "string" == type(_v) then
                szRet = szRet .. '"' .. _v .. '"' .. ","
                szRet = szRet .. '\n'
                szRet = szRet .. temp
            elseif "table" == type(_v) then
                szRet = szRet .. serialize(_v) .. ","
            else
                szRet = szRet .. "nil,"
            end
        end
    end

    table.foreach(_t, doT2S)
    szRet = szRet .. "}"

    return szRet
end

-- local function serialize(_t)
--     if _t == nil then
--         return "[nil]"
--     end

--     local szRet = "{"
--     function doT2S(_i, _v)
--         if "number" == type(_i) then
--             szRet = szRet .. "[" .. _i .. "] = "
--             if "number" == type(_v) then
--                 szRet = szRet .. _v .. ","
--             elseif "string" == type(_v) then
--                 szRet = szRet .. '"' .. _v .. '"' .. ","
--             elseif "table" == type(_v) then
--                 szRet = szRet .. serialize(_v) .. ","
--             else
--                 szRet = szRet .. "nil,"
--             end
--         elseif "string" == type(_i) then
--             szRet = szRet .. '["' .. _i .. '"] = '
--             if "number" == type(_v) then
--                 szRet = szRet .. _v .. ","
--             elseif "string" == type(_v) then
--                 szRet = szRet .. '"' .. _v .. '"' .. ","
--             elseif "table" == type(_v) then
--                 szRet = szRet .. serialize(_v) .. ","
--             else
--                 szRet = szRet .. "nil,"
--             end
--         end
--     end
--     table.foreach(_t, doT2S)
--     szRet = szRet .. "}"

--     return szRet
-- end


local parseRepeat_lx = function (msgList, tab)
    for _, obj in ipairs(msgList) do 
        if type(obj) == 'number' or type(obj) == 'string' then 
            table.insert(tab, obj);
        elseif type(obj) == 'boolean' then
            if obj then 
                table.insert(tab, 'true');
            else
                table.insert(tab, 'false');
            end
        else
            local t = getmetatable(obj)
            local desc = t._descriptor;

            local argcName = desc.name; 
            local argcType = desc.type; 
            local argcLabel = desc.label; 
            table.insert(tab, pbToTable(obj))
        end
    end
end



--protobuf 转换成tabel
local function pbToTable(msg, tab)
    tab = tab or {}

    local t = getmetatable(msg)
    local desc = t._descriptor;
    local count = #desc.fields;

    for _, field in ipairs(desc.fields) do 
        local argcName = field.name; 
        local argcType = field.type; 
        local argcLabel = field.label; 

        if argcLabel == 3 then  --FieldDescriptor.LABEL_REPEATED
            local childList = msg[argcName];

            if childList and #childList > 0 then 
                local tempTab = {}
                tab[argcName] = tempTab;

                parseRepeat_lx(childList, tempTab)
            end
        else
            if 11 == argcType then   --FieldDescriptor.TYPE_MESSAGE
                local tempMsg = msg[argcName];
                local tempTab = {}

                if tempMsg then 
                    tab[argcName] = tempTab;
                    pbToTable(tempMsg, tempTab)
                end
            elseif 8 == argcType then   -- boolen
                if msg[argcName] then 
                    tab[argcName] = 'true'
                else
                    tab[argcName] = 'false'
                end
            else
                tab[argcName] = msg[argcName];
            end
        end
    end

    return tab
end

local function serialForPrint(value, desciption, nesting)
    if type(nesting) ~= "number" then nesting = 10 end

    local lookupTable = {}
    local result = {}

    local function _v(v)
        if type(v) == "string" then
            v = "\"" .. v .. "\""
        end

        return tostring(v)
    end

    local function _des(v)
        if type(v) == "string" then
            v = "[\"" .. v .. "\"]"
        elseif 'number' == type(v) then
            v = string.format('[%d]', v)
        end

        return v
    end


    local function serial(value, desciption, indent, nest, keylen)
        desciption = desciption or "==>"
        local spc = ""
        if type(keylen) == "number" then
            spc = string.rep(" ", keylen - string.len(_v(desciption)))
        end
        if type(value) ~= "table" then
            result[#result +1 ] = string.format("%s%s%s = %s", indent, _des(desciption), spc, _v(value))
        elseif lookupTable[value] then
            result[#result +1 ] = string.format("%s%s%s = *REF*", indent, desciption, spc)
        else
            lookupTable[value] = true
            if nest > nesting then
                result[#result +1 ] = string.format("%s%s = *MAX NESTING*", indent, desciption)
            else
                result[#result +1 ] = string.format("%s%s = {", indent, _des(desciption))
                local indent2 = indent.."   "
                local keys = {}
                local keylen = 0
                local values = {}
                for k, v in pairs(value) do
                    keys[#keys + 1] = k
                    local vk = _v(k)
                    local vkl = string.len(vk)
                    if vkl > keylen then keylen = vkl end
                    values[k] = v
                end
                table.sort(keys, function(a, b)
                    if type(a) == "number" and type(b) == "number" then
                        return a < b
                    else
                        return tostring(a) < tostring(b)
                    end
                end)
                for i, k in ipairs(keys) do
                    serial(values[k], k, indent2, nest + 1, keylen)
                end
                result[#result +1] = string.format("%s}", indent)
            end
        end
    end

    serial(value, desciption, "", 1)

    for i, line in ipairs(result) do
        print(line)
    end
end

--可以打印所有的对象
function look(obj, str)
    if not DEBUG_MOD then 
        return
    end

    if not obj then
        print('  =============> ' .. str .. ' nil')
        return
    end

    -- local traceback = string.split(debug.traceback("", 2), "\n")
    -- local lineNumber = string.gsub()
    -- print("look from: " .. string.trim(traceback[3]))


    str = str or ''

    if type(obj) == 'number' then 
        str = str .. '  =============>'
        print(str, obj)
    elseif type(obj) == 'string' then
        print(str .. '=============> ' ..obj)
    else
        local t = getmetatable(obj)

        if t then 
            local desc = t._descriptor;
            if desc and desc.full_name then
                local str = string.format("%s ===%s ======>", desc.full_name, str)
                serialForPrint(pbToTable(obj), str)
            else
                str = str .. '  =============>'
                serialForPrint(obj, str)
            end
        else
            str = str .. '  =============>'
            serialForPrint(obj, str)
        end 
    end
end

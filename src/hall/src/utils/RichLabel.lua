
enCreatureEntityProp = {
    GOLD = 'GOLD',
    NAME = 'NAME',
}


Player = {
    GOLD = 10000,
    NAME = 'lx',
}

local ParseHtml = require 'utils.HtmlParser'

-- 默认参数  这些Key在在字符串中通用
local DefaultParams = {
    fontName     = "text_normal.ttf",    --字体
    fontSize     = 24,              --大小
    fontColor    = cc.c3b(255, 0, 0), --字体颜色
    lineSpace    = 5,                      --行间距
    outlineColor = cc.c4b(0, 0, 0, 255),   --字体描边
    outlineSize  = -1,                     --描边大小
    hAlignment   = cc.TEXT_ALIGNMENT_LEFT,    --对其方式
    link         = nil,                     --是否链接 有下划线
    linkParam    = 'lx|18',                 --链接参数
    image        = 'lx.png',
}

--[[关于self.msgConfig的数据
self.msgConfig = {
    callbackList = {callback1, callback2} --文本回调放在这里
    replaceValueList = {varg1, varg2}     --替换参数放这里
}
]]

local ReplacePlayerKey = {
    ['{金钱}'] = enCreatureEntityProp.GOLD,
    ['{昵称}'] = enCreatureEntityProp.NAME,
}

local ReplaceCustomKey = '{value}'


local RichLabel = class("RichLabel", function() 
    return cc.Node:create()
end)

function RichLabel:ctor(...)

    --设置onEnter  OnExit
    local function registerHander(event)
        if "enter" == event then
            self:onEnter()
        elseif "exit" == event then
            self:onExit()
        end
    end

    self:registerScriptHandler(registerHander)

    local args = {...}
    if type(args[2]) == 'table' then 
        self.elemList = args[2]
    elseif type(args[2]) == 'string' then
        look(args[2], 'fuck')
        self.elemList = ParseHtml.parse(args[2])
    end

    self.msgConfig = args[3]
    
    --对其方式
    self.hAlignment = DefaultParams['hAlignment']
    --垂直距离
    self.verticalSpace = DefaultParams['lineSpace']
    --字体名字
    self.fontName = DefaultParams['fontName']
    --字体大小
    self.fontSize = DefaultParams['fontSize']
    --字体颜色
    self.fontColor = DefaultParams['fontColor']

    self.parent = ccui.RichText:create()
    self.parent:ignoreContentAdaptWithSize(false)
    self.parent:setAnchorPoint(cc.p(0, 0))
    self.parent:setContentSize(500, 100)
    self.parent:setVerticalSpace(self.verticalSpace)
    self:addChild(self.parent)
    self:addElement()
end

function RichLabel:addElement()
    print('RichLabel:addElement')
    look(self.elemList, 'RichLabel:addElement')

    for _, item in pairs(self.elemList) do
        if item.labelname == "font" then
            self:handleText(item)
        elseif item.labelname == "image" then
            self:handleImage(item)
        elseif item.labelname == "newline" then
            local element = ccui.RichElementNewLine:create(1, self.fontColor, 255) 
            self.parent:pushBackElement(element)
        else
            look(item.labelname, "不支持的标签类型 labelname ")
        end
    end
end 


--[[enum {     --第七个参数  掩码
    ITALICS_FLAG = 1 << 0,          /*!< italic text */
    BOLD_FLAG = 1 << 1,             /*!< bold text */
    UNDERLINE_FLAG = 1 << 2,        /*!< underline */
    STRIKETHROUGH_FLAG = 1 << 3,    /*!< strikethrough */
    URL_FLAG = 1 << 4,              /*!< url of anchor */
    OUTLINE_FLAG = 1 << 5,          /*!< outline effect */
    SHADOW_FLAG = 1 << 6,           /*!< shadow effect */
    GLOW_FLAG = 1 << 7              /*!< glow effect */
};]]

function RichLabel:handleText(itemInfo)
    look(itemInfo, 'RichLabel:handleText')
    local fontName  = itemInfo.name or self.fontName
    local fontSize  = tonumber(itemInfo.size or self.fontSize)
    local fontColor = self.fontColor
    local fontOpacity = 255
    local outlineColor = self.outlineColor
    local outlineSize = tonumber(itemInfo.outlinesize or self.outlineSize)
    local link = itemInfo.link
    local content = itemInfo.content

    if itemInfo.color then
        fontColor = self:convertColor(itemInfo.color)
    end

    if itemInfo.outlinecolor then
        outlineColor = self:convertColor(itemInfo.outlinecolor)
    end

    --玩家基本数据  替换玩家基本数据
    for k, v in pairs(ReplacePlayerKey) do 
        content = string.gsub(content, k, Player[v])
    end  

    --自定义数据替换
    if self.msgConfig and self.msgConfig.replaceValueList then
        while string.find(content, ReplaceCustomKey) and #self.msgConfig.replaceValueList > 0 do 
             content, count = string.gsub(content, ReplaceCustomKey, self.msgConfig.replaceValueList[1], 1)
            if count > 0 then 
                table.remove(self.msgConfig.replaceValueList, 1)
            else
                break
            end
        end
    end

    local element
    --如果有超链接 请确保不要换行，否则会有问题
    if link and self.msgConfig and self.msgConfig.callbackList and #self.msgConfig.callbackList > 0 then 
        local label = cc.Label:create();
        label:setString(content)
        label:setSystemFontName(fontName)
        label:setSystemFontSize(fontSize)
        label:enableUnderline()
        label:setColor(fontColor)
        local callback = self.msgConfig.callbackList[1] 
        table.remove(self.msgConfig.callbackList, 1)

        local function onTouchBegan(touch, event)
            local pos = touch:getLocation()
            local target = event:getCurrentTarget()
            local nodePos = target:convertToNodeSpace(pos)
            local rect = target:getBoundingBox()
            rect.x = 0
            rect.y = 0

            if not cc.rectContainsPoint(rect, nodePos) or not target:isVisible() then
                return false
            end

            return true
        end

        local function onTouchMoved(touch, event)
        end

        local function onTouchEnded(touch, event)
            local linkparam = itemInfo.linkparam

            if linkparam then 
                if type(linkparam) == 'string' then 
                    linkparam = Util.split(linkparam, '|')
                end
            end

            callback(linkparam)
        end

        local function onTouchCancelled(touch, event)
        end

        local listener = cc.EventListenerTouchOneByOne:create()
        listener:setSwallowTouches(self.swallowTouches)

        listener:registerScriptHandler(onTouchBegan, cc.Handler.EVENT_TOUCH_BEGAN)
        listener:registerScriptHandler(onTouchMoved, cc.Handler.EVENT_TOUCH_MOVED)
        listener:registerScriptHandler(onTouchEnded, cc.Handler.EVENT_TOUCH_ENDED)
        listener:registerScriptHandler(onTouchCancelled, cc.Handler.EVENT_TOUCH_CANCELLED)

        local eventDispatcher = label:getEventDispatcher()
        eventDispatcher:addEventListenerWithSceneGraphPriority(listener, label)
        element = ccui.RichElementCustomNode:create(1, fontColor, fontOpacity, label)
    else
        element = ccui.RichElementText:create(1, fontColor, fontOpacity, content, fontName, fontSize)--, outlineColor, outlineSize)
    end

    self.parent:pushBackElement(element)

    return element
end


function RichLabel:handleImage(itemInfo)
    look(itemInfo, 'RichLabel:handleImage')
    if not itemInfo.src then
        return
    end

    local src = itemInfo.src
    local scale = itemInfo.scale or 1

    local spriteFrameCache = cc.SpriteFrameCache:getInstance()
    local spriteFrame = spriteFrameCache:getSpriteFrame(src)

    local image
    if spriteFrame then
        image = cc.Sprite:createWithSpriteFrame(spriteFrame)
    else
        image = cc.Sprite:create(src)
    end

    image:setScale(scale)
    image:setAnchorPoint(cc.p(0, 0))
    local element = ccui.RichElementCustomNode:create(1, cc.c3b(255, 255, 255), 255, image)
    self.parent:pushBackElement(element)
    return element
end

function RichLabel:onEnter()
end

function RichLabel:onExit()

end

--[
-- @brief  颜色转换
-- @param  #123456 --> cc.c3b
-- @return void
--]
function RichLabel:convertColor(color)
    if not color then
        return
    end

    local toTen = function (v)
        return tonumber("0x" .. v)
    end

    local b = string.sub(color, -2, -1)
    local g = string.sub(color, -4, -3)
    local r = string.sub(color, -6, -5)

    local red = toTen(r)
    local green = toTen(g)
    local blue = toTen(b)
    if red and green and blue then
        return cc.c3b(red, green, blue)
    end
end

return RichLabel


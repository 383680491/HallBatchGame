


local BaseView = class("BaseView", cc.Layer)
local display = require 'cocos.framework.display'
local device = require 'cocos.framework.device'

function BaseView:ctor()
    self.swallowTouches = swallowTouches
    self.blackLayer = nil
    self.registIdList_bv = {};
    self.releaseNodeList_bv = {};
    self.emitEventList = {}
    
    --设置onEnter  OnExit
    local function registerHander(event)
        if "enter" == event then
            self:onEnter()
        elseif "exit" == event then
            self:onExit()
        end
    end

    self:registerScriptHandler(registerHander)

    if device.platform ~= "ios" then 
        --设置返回键
        local function onKeyCallback(code, event)
            if(cc.KeyCode.KEY_BACK == code) then
                self:onKeyPressed()
            end
        end

        local keyListener = cc.EventListenerKeyboard:create()
        keyListener:registerScriptHandler(onKeyCallback, cc.Handler.EVENT_KEYBOARD_RELEASED)
        local eventDispatcher = self:getEventDispatcher()
        eventDispatcher:addEventListenerWithSceneGraphPriority(keyListener, self)
    end

    
    local function onTouchBegan(touch, event)
        return self:onTouchBegan(touch, event)
    end

    local function onTouchMoved(touch, event)
        self:onTouchMoved(touch, event)
    end

    local function onTouchEnded(touch, event)
        self:onTouchEnded(touch, event)
    end

    local function onTouchCancelled(touch, event)
        self:onTouchCancelled(touch, event)
    end
    local listener = cc.EventListenerTouchOneByOne:create()
    listener:setSwallowTouches(self.swallowTouches)

    listener:registerScriptHandler(onTouchBegan, cc.Handler.EVENT_TOUCH_BEGAN)
    listener:registerScriptHandler(onTouchMoved, cc.Handler.EVENT_TOUCH_MOVED)
    listener:registerScriptHandler(onTouchEnded, cc.Handler.EVENT_TOUCH_ENDED)
    listener:registerScriptHandler(onTouchCancelled, cc.Handler.EVENT_TOUCH_CANCELLED)

    local eventDispatcher = self:getEventDispatcher()
    eventDispatcher:addEventListenerWithSceneGraphPriority(listener, self)
end

--增加遮罩
function BaseView:addLayerMask(opacity, color) 
    opacity = opacity or 0
    color = color or cc.c3b(0, 0, 0)
    --全屏的黑色透明的层
    if not self.layerMask then 
        self.layerMask = display.newLayer({r = color.r, g = color.g, b = color.b, a = opacity})
        self:addChild(self.layerMask)

        self.layerMask:setTouchEnabled(true)
        local listenner = cc.EventListenerTouchOneByOne:create()  
        listenner:setSwallowTouches(true)  
        listenner:registerScriptHandler(function(touch, event)  
            local location = touch:getLocation()  
      
            return true  
        end, cc.Handler.EVENT_TOUCH_BEGAN )  
        local eventDispatcher = self.layerMask:getEventDispatcher()  
        eventDispatcher:addEventListenerWithSceneGraphPriority(listenner, self) 
    else
        self.layerMask:setOpacity(opacity)
    end
end

--移除遮罩
function BaseView:removeLayerMask() 
    if self.layerMask then 
        self.layerMask:removeFromParent()
        self.layerMask = nil
    end
end 


--子类调用这个 别调用comm:ins():sendMessage(protoId, msg, callback, errCallback);
function BaseView:sendMessage(protoId, msg, callback, errCallback) 
    comm:ins():sendMessage(protoId, msg, callback, errCallback);
end


--同上
function BaseView:registNotify(protoId) 
    comm:ins():registNotify(protoId, self);
    table.insert(self.registIdList_bv, protoId);
end


--事件分发
function BaseView:emit(eventName, ...) 
    G_EmitIns:dispatchEvent(eventName, ...)
end

--事件监听
function BaseView:on(eventName, func) 
    table.insert(self.emitEventList, G_EmitIns:addEventListen(eventName, func, self))
end



----------------------------------------------------------
--			可调用
----------------------------------------------------------
-- 获取本view的key，UI管理用，子类记得改写这个值
function BaseView:getMoudleId() 
	return self.moduleId
end



--每帧都执行回调
function BaseView:addFrameCallBack(callback) 
    local node = cc.Node:create();
    self:addChild(node);

    node:runAction(cc.RepeatForever:create(
        cc.CallFunc:create(function() 
            if callback then 
                callback();
            end
     end)));
end

--每秒都执行回调
function BaseView:addPerSecondCallBack(callback) 
    local node = cc.Node:create();
    self:addChild(node);

    node:runAction(cc.RepeatForever:create(
        cc.CallFunc:create(function() 
            if callback then 
                callback();
            end
     end)));
end

--下一帧执行
function BaseView:addNextFrameCallBack(callback) 
    self:addDelayCallBack(callback, 0)
end


--延迟执行
function BaseView:addDelayCallBack(callback, time) 
    time = time or 1
    local node = cc.Node:create();
    self:addChild(node);
    node:runAction(cc.Sequence:create(
        cc.DelayTime:create(time),
        cc.CallFunc:create(function() 
            if callback then 
                callback();
            end
            node:removeFromParent();
     end)));
end

--cocos自带的注册自定义消息  例如金钱改变  event = "event_change_gold"  参数最好是 字符串且不要太简单 别和其他事件起冲突
function BaseView:cocosDispatchEvent(strEvent, data) 
    local node = cc.Node:create();
    self:addChild( node );
    table.insert(self.releaseNodeList_bv, node);

    local eventDispatcher = node:getEventDispatcher()
    local event = cc.EventCustom:new(strEvent)
    event.__my_data__ = data
    eventDispatcher:dispatchEvent(event)
end


--cocos自带的监听自定义消息
function BaseView:cocosEventListen(strEvent, callback) 
    local node = cc.Node:create();
    self:addChild(node);
    table.insert(self.releaseNodeList_bv, node);

    local listener = cc.EventListenerCustom:create(strEvent,function(event)
        if strEvent == event:getEventName() then
            if callback then 
                callback(event.__my_data__);
            end
        end
    end)
    local eventDispatcher = node:getEventDispatcher()
    eventDispatcher:addEventListenerWithFixedPriority(listener, 1)
end



---------------------------------------------------------
--					子类要重写的方法
---------------------------------------------------------
--监听消息的应答函数
function BaseView:onServerNotify(protoId, respMsg) 
    --子类重构此方法
end

--界面移除时会调用  
function BaseView:onClose(protoId, respMsg) 
    --子类重构此方法
end



function BaseView:onEnter()
end

function BaseView:onExit()
    if self.onClose then 
        self:onClose();
    end

    for _, protoId in ipairs(self.registIdList_bv) do 
        comm:ins():unregistNotify(protoId, self);
    end

    for _, event in ipairs(self.emitEventList) do 
        G_EmitIns:removeEvent(event)
    end

    UIMananer:removeMoudleId(self.moduleId)
    -- for _, node in ipairs(self.releaseNodeList_bv) do 
    --     node:release();
    -- end
end

function BaseView:onKeyPressed(code, event)
end

function BaseView:onTouchBegan(touch, event)
    return true
end

function BaseView:onTouchMoved(touch, event)

end
function BaseView:onTouchEnded(touch, event)

end
function BaseView:onTouchCancelled(touch, event)

end

--移除自己执行动画
function BaseView:removeSelfWithAction()
    self:runAction(cc.Sequence:create(cc.ScaleTo:create(0.1, 0), cc.RemoveSelf:create()))
end





--这个功能是为了在界面完成后执行endProtect 例如当我们一个界面有动画 界面在最开始的时候从
--屏幕外移动到屏幕内 过程中界面不让点击
--这个功能有待商议  可有可无   
function BaseView:beginProtect()

end


function BaseView:endProtect() 
end


return BaseView


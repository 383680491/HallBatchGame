
local device = require 'cocos.framework.device'
local BaseView = require 'ui.base.BaseView'
local Login = class("Login",  BaseView)

--消息传过来准备 点击确定后重连 
function Login:ctor()
	Login.super.ctor(self)
	self:initUI()
end

function Login:initUI() 
	self:addLayerMask(150, cc.c3b(255, 0, 0))
    local path = "csb/login/LoginAgain.csb"
    local logoLayer = cc.CSLoader:createNode(path)
    self:addChild(logoLayer)

    local button1 = ccui.ImageView:create("face/button_bet_blue.png");
    button1:setPosition(cc.p(300, 300))
    self:addChild(button1)
    button1:setTouchEnabled(true)
    button1:addClickEventListener(function(sender) 
		require 'update.GameAndUpdate'
	    local layer = GameAndUpdate:create("fruitPlay", function()
	    	    	print('aaaaaaaaaaa')

	        local FruitMain = require "fruitPlay.src.FruitMain"
	        local fruitMain = FruitMain:new()
	        self:addChild(fruitMain)
	    end);

	    self:addChild(layer)
	end);

    local button2 = ccui.ImageView:create("face/button_bet_blue.png");
    button2:setPosition(cc.p(600, 300))
    self:addChild(button2)
    button2:setTouchEnabled(true)
    button2:addClickEventListener(function(sender) 
		print('2222222222222')
	end);
end

return Login




   



















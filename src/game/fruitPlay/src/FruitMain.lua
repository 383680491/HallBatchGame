
local device = require 'cocos.framework.device'
local BaseView = require 'ui.base.BaseView'
local FruitMain = class("FruitMain",  BaseView)

--消息传过来准备 点击确定后重连 
function FruitMain:ctor()
	FruitMain.super.ctor(self)
	self:initUI()
end

function FruitMain:initUI() 
	self:addLayerMask(150, cc.c3b(0, 255, 0))
    local image = ccui.ImageView:create("fruitPlay/res/001.png");
    image:setPosition(cc.p(500, 500))
    print('FruitMain  222222222222')
    self:addChild(image)
end

return FruitMain




   



















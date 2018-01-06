local UIManager = class("UIManager")
local Login = require 'ui.Login'

local FloatViewZorder = 200
local NetDialogZorder = 300

-- cur 是指当前layer          
-- dependId 依赖某个界面 参数是模块ID， 可以为nil，比如主城和任务系统  如果只显示任务系统 可能外面的屏幕会是黑的 或者资源找不到等等，需要依赖某个界面
local moduleMap = {
	LOGIN_TEST = {cur = Login, dependId = nil}
}

function UIManager:ctor()
	self.floatViewMap = {}
	self.curScene = nil
	self.stepZorder = 1
	self.curLayerList = {}
	-- self:registFloatView(G_SceneDefine.SCENE_LOGIN, Login)
end

function UIManager:replaceScene(sceneId, transition, time, more)
	self.stepZorder = 1
	self.curLayerList = {}

	local scene = cc.Scene:create()
	scene._custum_id_ = sceneId
	self.curScene = sceneId

	display.runScene(scene, transition, time, more)

	local floatViewList = self.floatViewMap[sceneId]
	for _, moduleObj in ipairs(floatViewList) do 
		local mod = moduleObj:new()
		scene:addChild(mod, FloatViewZorder, FloatViewZorder)
	end

	return scene
end

function UIManager:registFloatView(sceneId, mod)
	local floatViewList = self.floatViewMap[sceneId]

	if not floatViewList then 
		self.floatViewMap[sceneId] = {}
	end

	table.insert(floatViewList, mod)
end

--如果大一些的界面直接添加到场景 如果是小一些的界面并且不会存在跳转一类的，并不想要唯一界面ID，则传入父节点就好
function UIManager:addLayer(layer, parent)
	local moudleId = layer:getMoudleId()
	if not moudleId then 
		assert(not parent, '请为该界面添加一个唯一ID 或者 添加一个父亲节点')
		parent:addChild(layer)
	else
		parent = parent or self:getCurScene()
		parent:addLayer(layer, self.stepZorder, self.stepZorder)
		self.curLayerList[moudleId] = self.stepZorder   --不直接保存对象，保存的是tag，避免保存对象出问题

		self.stepZorder = self.stepZorder + 1
		if self.stepZorder >= 199 then 
			self.stepZorder = 1
		end 
	end
end

--如果已经在场景里面才会有值 可以判断当前是否有此界面
function UIManager:getLayerByModuleId(moduleId)
	local tag = self.curLayerList[moduleId]

	if not tag then
		print('该界面不存在或者并不直接存在于场景的一级目录') 
		return 
	end

	local scene = self:getCurScene()
	return scene:getChildByTag(tag)
end

--移除掉当前Tag
function UIManager:removeMoudleId(moduleId)
	if not moduleId then 
		return
	end

	self.curLayerList[moduleId] = nil
end


function UIManager:getCurScene()
	return display.getRunningScene()
end

function UIManager:getCurSceneId()
	return display.getRunningScene()._custum_id_
end

--如果depend也需要参数 之后再设计吧   如果有进度条的界面 则把进度条和当前layer写在一起
function UIManager:jumpModule(moduleId, ...)
	local sceneId = math.floor(moduleId /1000)
	local scene = nil

	if sceneId == self:getCurSceneId then 
		scene = self:getCurScene()
	else
		scene = self:replaceScene(sceneId)
	end

	local moduleInfo = moduleMap[moduleId]
	if moduleInfo.dependId then 
		if not self:getLayerByModuleId(moduleInfo.dependId) then  --不存在则加进去
			local dependModule = moduleMap[moduleInfo.dependId].cur
			local mod = dependModule:new()
			self:addLayer(mod, scene)
		end
	end

	local curModule = moduleInfo.cur
	local mod = curModule:new(...)
	self:addLayer(mod, scene)
end


return UIManager

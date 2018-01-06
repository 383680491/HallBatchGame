--lx code 2017/11/7--

local EventEmitter = class("EventEmitter")
function EventEmitter:ctor()
	self.list = {}
	self.stepId = 1
end

function EventEmitter:addEventListen(eventName, callback, listener)
	if not self.list[eventName] then 
		self.list[eventName] = {}
	end

	listener.__Emitter_flag__ = 1

	local eventInfo = {}
	eventInfo.id = self.stepId
	eventInfo.callback = callback
	eventInfo.listener = listener
	eventInfo.eventName = eventName

	table.insert(self.list[eventName], eventInfo)

	self.stepId = self.stepId + 1    --id 是为了只传入回调不传入对象的方式去设计的
	return eventInfo
end

function EventEmitter:dispatchEvent(eventName, ...)
	if not self.list[eventName] then 
		return
	end

	local deleteList = {}

	for _, eventInfo in ipairs(self.list[eventName]) do 
		if eventInfo.listener then 
			if 1 == eventInfo.listener.__Emitter_flag__ then 
				local listener = eventInfo.listener
				local callback = eventInfo.callback

				listener.__Emitter_flag__ = nil
				listener.callback(listener, ...)
			else
				--listener对象异常  执行remove
				table.insert(deleteList, eventInfo)
			end
		else
			eventInfo.callback(...)
		end
	end

	for _, eventInfo in ipairs(deleteList) do 
		self:removeEvent(eventInfo)
	end
end

function EventEmitter:removeEvent(eventInfo)
	local eventList = self.list[eventInfo.eventName]
	if not eventList then 
		return
	end

	for i = 1, #eventList do 
		if eventList[i].id == eventInfo.id then 
			table.remove(eventList, i)
			return
		end
	end
end

function EventEmitter:removeEventById(id)
	for _, eventList in ipairs(self.list) do 
		for i = 1, #eventList do 
			if eventList[i].id == id then 
				table.remove(eventList, i)
				return
			end
		end
	end
end


function EventEmitter:removeEventByListen(listener)
	for _, eventList in ipairs(self.list) do 
		for i = 1, #eventList do 
			if eventList[i].listener == listener then 
				table.remove(eventList, i)
				return
			end
		end
	end
end

function EventEmitter:clear()
	self.list = {}
	self.stepId = 1
end


return EventEmitter

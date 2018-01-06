

----------------------------------- 登录协议组
msgType_Login				= 1000
-------------
--	心跳包
msg_Login_Heart              = 1001
-- 	登录
msg_Login_Test               = 1002;



----------------------------------- 房间协议组
msgType_Room				= 2000
-------------
--獲得房間數據
req_Room_GetList           	= 2001
--創建房卡房间数据
req_Room_CreateRoom         = 2002
--加入房卡房间数据
req_Room_JoinRoom           = 2003
--退出房间数据
req_Room_ExitRoom           = 2004
--解散房间数据
req_Room_DisoloveRoom       = 2005

--------------
msg_Room_JoinRoom			= 2999	-- 加入房间
msg_Room_ExitRoom			= 2998	-- 退出房间通知。


-- 加入大厅自动房间。
msg_EnterGame 				= 2006
-- 离开大厅自动房间
msg_outGame					= 2007
-- 有其他玩家掉线。
msg_DropGame				= 2008
-- 有玩家回到游戏。
msg_RebackGame 				= 2009
-- 玩家属性变化- 只针对金币，钻石，vip
msg_RoleInfoChange			= 2010
-- 玩家被踢出
msg_KickPlayer				= 2011
-- 玩家准备
msg_PlayerInReady			= 2012
-- 玩家取消准备
msg_PlayerOutReady 			= 2013
-- 玩家挂机-托管
msg_PlayerInSleep			= 2014
-- 玩家挂机 取消托管
msg_PlayerOutSleep			= 2015







----------------------------------- 斗牛游戏协议组。  3000
msgType_Game_Dn						= 3000
------客户端命令结构		
req_dn_CALL_BANKER					= 3101 		--用户叫庄
req_dn_ADD_SCORE					= 3102 		--用户加注
------服务端通知
msg_Dn_GameInfo					= 3001 		--玩家进入房间，发送基本信息
msg_Dn_GAME_READY				= 3002 		--游戏即将开始倒计时
msg_Dn_GAME_START				= 3003 		--游戏开始
msg_Dn_SEND_POKER_4				= 3004 		--收到前4张牌
msg_Dn_SEND_POKER_5				= 3005 		--收到第5张牌
msg_Dn_ONE_CALL_BANKER			= 3006		--有个玩家叫庄
msg_Dn_SURE_BANKER				= 3007		--得出庄家
msg_Dn_ONE_ADD_BET				= 3008
msg_Dn_SURE_BET					= 3009
msg_Dn_GAME_RESULT				= 3010
msg_Dn_GAME_END					= 3011






























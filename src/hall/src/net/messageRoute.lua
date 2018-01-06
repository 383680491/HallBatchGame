local routeTable = {
	[msg_Login_Test] = msgLogin_pb.test_login_resp(),
	[req_Room_GetList] = msgRoom_pb.get_room_resp(),
	[req_Room_CreateRoom] = msgRoom_pb.create_room_resp(),
	[req_Room_JoinRoom] = msgRoom_pb.room_join_resp(),
	[req_Room_DisoloveRoom] = msgRoom_pb.dissolve_player_info(),
	[req_Room_ExitRoom] = msgRoom_pb.quit_room_resp(),
	---- 
	[req_dn_CALL_BANKER]		= msgRoom_pb.call_bank_req(),
	[req_dn_ADD_SCORE]			= msgRoom_pb.add_score_req(),
	------服务端通知
	[msg_Dn_GameInfo]			= msgRoom_pb.notify_dn_GameInfo(),
	[msg_Dn_GAME_READY]			= msgRoom_pb.notify_dn_GAME_READY(),
	[msg_Dn_GAME_START]			= msgRoom_pb.notify_dn_GAME_START(),
	[msg_Dn_SEND_POKER_4]		= msgRoom_pb.notify_dn_SEND_POKER_4(),
	[msg_Dn_SEND_POKER_5]		= msgRoom_pb.notify_dn_SEND_POKER_5(),
	[msg_Dn_ONE_CALL_BANKER]	= msgRoom_pb.notify_dn_ONE_CALL_BANKER(),
	[msg_Dn_SURE_BANKER]		= msgRoom_pb.notify_dn_SURE_BANKER(),
	[msg_Dn_ONE_ADD_BET]		= msgRoom_pb.notify_dn_ONE_ADD_BET(),
	[msg_Dn_SURE_BET]			= msgRoom_pb.notify_dn_SURE_BET(),
	[msg_Dn_GAME_RESULT]		= msgRoom_pb.notify_dn_GAME_RESULT(),
	[msg_Room_JoinRoom]		    = msgRoom_pb.join_room_notify(),
	[9999]		                = msgRoom_pb.test_resp(),

}

return routeTable;




#ifndef _CHANNEL_EMITTER_H_
#define _CHANNEL_EMITTER_H_ 
#include "ChannelEventDefine.h"
class ChannelEmitter
{
   static ChannelEmitter*m_pCEventEmitter;
   ChannelEmitter();
public:
   int m_event_num;
   static ChannelEmitter*getInstance();
   void emit(const char* event_id, const char* format, ...);
};
#endif //VERSIONMANAGER_H
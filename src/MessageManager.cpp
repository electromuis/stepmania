﻿#include "global.h"
#include "MessageManager.h"
#include "Foreach.h"
#include "RageUtil.h"
#include "RageThreads.h"
#include "EnumHelper.h"
#include "LuaManager.h"
#include "RageLog.h"

#include <set>
#include <map>

MessageManager*	MESSAGEMAN = NULL;	// global and accessible from anywhere in our program


static const char *MessageIDNames[] = {
	"CurrentGameChanged",
	"CurrentStyleChanged",
	"PlayModeChanged",
	"CoinsChanged",
	"CurrentSongChanged",
	"CurrentStepsP1Changed",
	"CurrentStepsP2Changed",
	"CurrentStepsP3Changed",
	"CurrentStepsP4Changed",
	"CurrentStepsP5Changed",
	"CurrentStepsP6Changed",
	"CurrentStepsP7Changed",
	"CurrentStepsP8Changed",
	"CurrentCourseChanged",
	"CurrentTrailP1Changed",
	"CurrentTrailP2Changed",
	"CurrentTrailP3Changed",
	"CurrentTrailP4Changed",
	"CurrentTrailP5Changed",
	"CurrentTrailP6Changed",
	"CurrentTrailP7Changed",
	"CurrentTrailP8Changed",
	"GameplayLeadInChanged",
	"EditStepsTypeChanged",
	"EditCourseDifficultyChanged",
	"EditSourceStepsChanged",
	"EditSourceStepsTypeChanged",
	"PreferredStepsTypeChanged",
	"PreferredDifficultyP1Changed",
	"PreferredDifficultyP2Changed",
	"PreferredDifficultyP3Changed",
	"PreferredDifficultyP4Changed",
	"PreferredDifficultyP5Changed",
	"PreferredDifficultyP6Changed",
	"PreferredDifficultyP7Changed",
	"PreferredDifficultyP8Changed",
	"PreferredCourseDifficultyP1Changed",
	"PreferredCourseDifficultyP2Changed",
	"PreferredCourseDifficultyP3Changed",
	"PreferredCourseDifficultyP4Changed",
	"PreferredCourseDifficultyP5Changed",
	"PreferredCourseDifficultyP6Changed",
	"PreferredCourseDifficultyP7Changed",
	"PreferredCourseDifficultyP8Changed",
	"EditCourseEntryIndexChanged",
	"EditLocalProfileIDChanged",
	"GoalCompleteP1",
	"GoalCompleteP2",
	"GoalCompleteP3",
	"GoalCompleteP4",
	"GoalCompleteP5",
	"GoalCompleteP6",
	"GoalCompleteP7",
	"GoalCompleteP8",
	"NoteCrossed",
	"NoteWillCrossIn400Ms",
	"NoteWillCrossIn800Ms",
	"NoteWillCrossIn1200Ms",
	"CardRemovedP1",
	"CardRemovedP2",
	"CardRemovedP3",
	"CardRemovedP4",
	"CardRemovedP5",
	"CardRemovedP6",
	"CardRemovedP7",
	"CardRemovedP8",
	"BeatCrossed",
	"MenuUpP1",
	"MenuUpP2",
	"MenuUpP3",
	"MenuUpP4",
	"MenuUpP5",
	"MenuUpP6",
	"MenuUpP7",
	"MenuUpP8",
	"MenuDownP1",
	"MenuDownP2",
	"MenuDownP3",
	"MenuDownP4",
	"MenuDownP5",
	"MenuDownP6",
	"MenuDownP7",
	"MenuDownP8",
	"MenuLeftP1",
	"MenuLeftP2",
	"MenuLeftP3",
	"MenuLeftP4",
	"MenuLeftP5",
	"MenuLeftP6",
	"MenuLeftP7",
	"MenuLeftP8",
	"MenuRightP1",
	"MenuRightP2",
	"MenuRightP3",
	"MenuRightP4",
	"MenuRightP5",
	"MenuRightP6",
	"MenuRightP7",
	"MenuRightP8",
	"MenuStartP1",
	"MenuStartP2",
	"MenuStartP3",
	"MenuStartP4",
	"MenuStartP5",
	"MenuStartP6",
	"MenuStartP7",
	"MenuStartP8",
	"MenuSelectionChanged",
	"PlayerJoined",
	"PlayerUnjoined",
	"AutosyncChanged",
	"PreferredSongGroupChanged",
	"PreferredCourseGroupChanged",
	"SortOrderChanged",
	"LessonTry1",
	"LessonTry2",
	"LessonTry3",
	"LessonCleared",
	"LessonFailed",
	"StorageDevicesChanged",
	"AutoJoyMappingApplied",
	"ScreenChanged",
	"SongModified",
	"MenuStartP1",
	"MenuStartP2",
	"MenuStartP3",
	"MenuStartP4",
	"MenuStartP5",
	"MenuStartP6",
	"MenuStartP7",
	"MenuStartP8",
	"StarPowerChangedP1",
	"StarPowerChangedP2",
	"StarPowerChangedP3",
	"StarPowerChangedP4",
	"StarPowerChangedP5",
	"StarPowerChangedP6",
	"StarPowerChangedP7",
	"StarPowerChangedP8",
	"CurrentComboChangedP1",
	"CurrentComboChangedP2",
	"CurrentComboChangedP3",
	"CurrentComboChangedP4",
	"CurrentComboChangedP5",
	"CurrentComboChangedP6",
	"CurrentComboChangedP7",
	"CurrentComboChangedP8",
	"StarMeterChangedP1",
	"StarMeterChangedP2",
	"StarMeterChangedP3",
	"StarMeterChangedP4",
	"StarMeterChangedP5",
	"StarMeterChangedP6",
	"StarMeterChangedP7",
	"StarMeterChangedP8",
	"LifeMeterChangedP1",
	"LifeMeterChangedP2",
	"LifeMeterChangedP3",
	"LifeMeterChangedP4",
	"LifeMeterChangedP5",
	"LifeMeterChangedP6",
	"LifeMeterChangedP7",
	"LifeMeterChangedP8",
	"UpdateScreenHeader",
	"LeftClick",
	"RightClick",
	"MiddleClick",
	"MouseWheelUp",
	"MouseWheelDown",
};
XToString( MessageID );

static RageMutex g_Mutex( "MessageManager" );

typedef set<IMessageSubscriber*> SubscribersSet;
static map<RString,SubscribersSet> g_MessageToSubscribers;

Message::Message( const RString &s )
{
	m_sName = s;
	m_pParams = new LuaTable;
	m_bBroadcast = false;
}

Message::Message(const MessageID id)
{
	m_sName= MessageIDToString(id);
	m_pParams = new LuaTable;
	m_bBroadcast = false;
}

Message::Message( const RString &s, const LuaReference &params )
{
	m_sName = s;
	m_bBroadcast = false;
	Lua *L = LUA->Get();
	m_pParams = new LuaTable; // XXX: creates an extra table
	params.PushSelf( L );
	m_pParams->SetFromStack( L );
	LUA->Release( L );
//	m_pParams = new LuaTable( params );
}

Message::~Message()
{
	delete m_pParams;
}

void Message::PushParamTable( lua_State *L )
{
	m_pParams->PushSelf( L );
}

void Message::SetParamTable( const LuaReference &params )
{
	Lua *L = LUA->Get();
	params.PushSelf( L );
	m_pParams->SetFromStack( L );
	LUA->Release( L );
}

const LuaReference &Message::GetParamTable() const
{
	return *m_pParams;
}

void Message::GetParamFromStack( lua_State *L, const RString &sName ) const
{
	m_pParams->Get( L, sName );
}

void Message::SetParamFromStack( lua_State *L, const RString &sName )
{
	m_pParams->Set( L, sName );
}

MessageManager::MessageManager()
{
	m_Logging= false;
	// Register with Lua.
	{
		Lua *L = LUA->Get();
		lua_pushstring( L, "MESSAGEMAN" );
		this->PushSelf( L );
		lua_settable( L, LUA_GLOBALSINDEX );
		LUA->Release( L );
	}
}

MessageManager::~MessageManager()
{
	// Unregister with Lua.
	LUA->UnsetGlobal( "MESSAGEMAN" );
}

void MessageManager::Subscribe( IMessageSubscriber* pSubscriber, const RString& sMessage )
{
	LockMut(g_Mutex);

	SubscribersSet& subs = g_MessageToSubscribers[sMessage];
#ifdef DEBUG
	SubscribersSet::iterator iter = subs.find(pSubscriber);
	ASSERT_M( iter == subs.end(), ssprintf("already subscribed to '%s'",sMessage.c_str()) );
#endif
	subs.insert( pSubscriber );
}

void MessageManager::Subscribe( IMessageSubscriber* pSubscriber, MessageID m )
{
	Subscribe( pSubscriber, MessageIDToString(m) );
}

void MessageManager::Unsubscribe( IMessageSubscriber* pSubscriber, const RString& sMessage )
{
	LockMut(g_Mutex);

	SubscribersSet& subs = g_MessageToSubscribers[sMessage];
	SubscribersSet::iterator iter = subs.find(pSubscriber);
	ASSERT( iter != subs.end() );
	subs.erase( iter );
}

void MessageManager::Unsubscribe( IMessageSubscriber* pSubscriber, MessageID m )
{
	Unsubscribe( pSubscriber, MessageIDToString(m) );
}

void MessageManager::Broadcast( Message &msg ) const
{
	// GAMESTATE is created before MESSAGEMAN, and has several BroadcastOnChangePtr members, so they all broadcast when they're initialized.
	if(this != NULL && m_Logging)
	{
		LOG->Trace("MESSAGEMAN:Broadcast: %s", msg.GetName().c_str());
	}
	msg.SetBroadcast(true);

	LockMut(g_Mutex);

	map<RString,SubscribersSet>::const_iterator iter = g_MessageToSubscribers.find( msg.GetName() );
	if( iter == g_MessageToSubscribers.end() )
		return;

	FOREACHS_CONST( IMessageSubscriber*, iter->second, p )
	{
		IMessageSubscriber *pSub = *p;
		pSub->HandleMessage( msg );
	}
}

void MessageManager::Broadcast( const RString& sMessage ) const
{
	ASSERT( !sMessage.empty() );
	Message msg(sMessage);
	Broadcast( msg );
}

void MessageManager::Broadcast( MessageID m ) const
{
	Broadcast( MessageIDToString(m) );
}

bool MessageManager::IsSubscribedToMessage( IMessageSubscriber* pSubscriber, const RString &sMessage ) const
{
	SubscribersSet& subs = g_MessageToSubscribers[sMessage];
	return subs.find( pSubscriber ) != subs.end();
}	

void IMessageSubscriber::ClearMessages( const RString sMessage )
{
}

MessageSubscriber::MessageSubscriber( const MessageSubscriber &cpy ):
	IMessageSubscriber(cpy)
{
	FOREACH_CONST( RString, cpy.m_vsSubscribedTo, msg )
		this->SubscribeToMessage( *msg );
}

MessageSubscriber &MessageSubscriber::operator=(const MessageSubscriber &cpy)
{
	if(&cpy == this)
		return *this;

	UnsubscribeAll();

	FOREACH_CONST( RString, cpy.m_vsSubscribedTo, msg )
		this->SubscribeToMessage( *msg );

	return *this;
}

void MessageSubscriber::SubscribeToMessage( const RString &sMessageName )
{
	MESSAGEMAN->Subscribe( this, sMessageName );
	m_vsSubscribedTo.push_back( sMessageName );
}

void MessageSubscriber::SubscribeToMessage( MessageID message )
{
	MESSAGEMAN->Subscribe( this, message );
	m_vsSubscribedTo.push_back( MessageIDToString(message) );
}

void MessageSubscriber::UnsubscribeAll()
{
	FOREACH_CONST( RString, m_vsSubscribedTo, s )
		MESSAGEMAN->Unsubscribe( this, *s );
	m_vsSubscribedTo.clear();
}


// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the MessageManager. */ 
class LunaMessageManager: public Luna<MessageManager>
{
public:
	static int Broadcast( T* p, lua_State *L )
	{
		if( !lua_istable(L, 2) && !lua_isnoneornil(L, 2) )
			luaL_typerror( L, 2, "table or nil" );

		LuaReference ParamTable;
		lua_pushvalue( L, 2 );
		ParamTable.SetFromStack( L );

		Message msg( SArg(1), ParamTable );
		p->Broadcast( msg );
		COMMON_RETURN_SELF;
	}
	static int SetLogging(T* p, lua_State *L)
	{
		p->SetLogging(lua_toboolean(L, -1));
		COMMON_RETURN_SELF;
	}

	LunaMessageManager()
	{
		ADD_METHOD( Broadcast );
		ADD_METHOD( SetLogging );
	}
};

LUA_REGISTER_CLASS( MessageManager )
// lua end

/*
 * (c) 2003-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE. 
 * 
 * (c) 2016- Electromuis, Anton Grootes
 * This branch of https://github.com/stepmania/stepmania
 * will from here on out be released as GPL v3 (wich converts from the previous MIT license)
 */

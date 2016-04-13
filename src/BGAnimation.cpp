#include "global.h"
#include "BGAnimation.h"
#include "IniFile.h"
#include "BGAnimationLayer.h"
#include "RageUtil.h"
#include "ActorUtil.h"
#include "LuaManager.h"
#include "PrefsManager.h"

using std::vector;

REGISTER_ACTOR_CLASS(BGAnimation);

BGAnimation::BGAnimation()
{
}

BGAnimation::~BGAnimation()
{
    DeleteAllChildren();
}

static bool CompareLayerNames( const std::string& s1, const std::string& s2 )
{
	int i1, i2;
	int ret;

	ret = sscanf( s1.c_str(), "Layer%d", &i1 );
	ASSERT( ret == 1 );
	ret = sscanf( s2.c_str(), "Layer%d", &i2 );
	ASSERT( ret == 1 );
	return i1 < i2;
}

void BGAnimation::AddLayersFromAniDir( const std::string &_sAniDir, const XNode *pNode )
{
	const std::string& sAniDir = _sAniDir;

	{
		vector<std::string> vsLayerNames;
		for (auto const *pLayer: *pNode)
		{
			if( strncmp(pLayer->GetName().c_str(), "Layer", 5) == 0 )
				vsLayerNames.push_back( pLayer->GetName() );
		}

		sort( vsLayerNames.begin(), vsLayerNames.end(), CompareLayerNames );


		for (auto const &sLayer: vsLayerNames)
		{
			const XNode* pKey = pNode->GetChild( sLayer );
			ASSERT( pKey != nullptr );

			std::string sImportDir;
			if( pKey->GetAttrValue("Import", sImportDir) )
			{
				bool bCond;
				if( pKey->GetAttrValue("Condition",bCond) && !bCond )
					continue;

				// import a whole BGAnimation
				sImportDir = sAniDir + sImportDir;
				CollapsePath( sImportDir );

				if (!Rage::ends_with(sImportDir, "/"))
				{
					sImportDir += "/";
				}
				ASSERT_M( IsADirectory(sImportDir), sImportDir + " isn't a directory" );

				std::string sPathToIni = sImportDir + "BGAnimation.ini";

				IniFile ini2;
				ini2.ReadFile( sPathToIni );

				AddLayersFromAniDir( sImportDir, &ini2 );
			}
			else
			{
				// import as a single layer
				BGAnimationLayer* bgLayer = new BGAnimationLayer;
				bgLayer->LoadFromNode( pKey );
				this->AddChild( bgLayer );
			}
		}
	}
}

void BGAnimation::LoadFromAniDir( const std::string &_sAniDir )
{
	DeleteAllChildren();

	if( _sAniDir.empty() )
		 return;

	std::string sAniDir = _sAniDir;
	if (!Rage::ends_with(sAniDir, "/"))
	{
		sAniDir += "/";
	}
	ASSERT_M( IsADirectory(sAniDir), sAniDir + " isn't a directory" );

	std::string sPathToIni = sAniDir + "BGAnimation.ini";

	if( DoesFileExist(sPathToIni) )
	{
		if( PREFSMAN->m_bQuirksMode )
		{
			// This is a 3.9-style BGAnimation (using .ini)
			IniFile ini;
			ini.ReadFile( sPathToIni );

			AddLayersFromAniDir( sAniDir, &ini ); // TODO: Check for circular load

			XNode* pBGAnimation = ini.GetChild( "BGAnimation" );
			XNode dummy( "BGAnimation" );
			if( pBGAnimation == nullptr )
				pBGAnimation = &dummy;

			LoadFromNode( pBGAnimation );
		}
		else // We don't officially support .ini files anymore.
		{
			XNode dummy( "BGAnimation" );
			XNode *pBG = &dummy;
			LoadFromNode( pBG );
		}
	}
	else
	{
		// This is an 3.0 and before-style BGAnimation (not using .ini)

		// loading a directory of layers
		vector<std::string> asImagePaths;
		ASSERT( sAniDir != "" );

		GetDirListing( sAniDir+"*.png", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.jpg", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.jpeg", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.gif", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.ogv", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.avi", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.mpg", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.mpeg", asImagePaths, false, true );

		SortStringArray( asImagePaths );

		for (auto const &sPath: asImagePaths)
		{
			if (Rage::starts_with(Rage::base_name(sPath), "_"))
			{
				continue; // don't directly load files starting with an underscore
			}
			BGAnimationLayer* pLayer = new BGAnimationLayer;
			pLayer->LoadFromAniLayerFile( sPath );
			AddChild( pLayer );
		}
	}
}

void BGAnimation::LoadFromNode( const XNode* pNode )
{
	std::string sDir;
	if( pNode->GetAttrValue("AniDir", sDir) )
		LoadFromAniDir( sDir );

	ActorFrame::LoadFromNode( pNode );

	/* Backwards-compatibility: if a "LengthSeconds" value is present, create a dummy
	 * actor that sleeps for the given length of time. This will extend GetTweenTimeLeft. */
	float fLengthSeconds = 0;
	if( pNode->GetAttrValue( "LengthSeconds", fLengthSeconds ) )
	{
		Actor *pActor = new Actor;
		pActor->SetName( "BGAnimation dummy" );
		pActor->SetVisible( false );
		apActorCommands ap = ActorUtil::ParseActorCommands( fmt::sprintf("sleep,%f",fLengthSeconds) );
		pActor->AddCommand( "On", ap );
		AddChild( pActor );
	}
}

/*
 * (c) 2001-2004 Ben Nordstrom, Chris Danford
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

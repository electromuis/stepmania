#ifndef COURSE_H
#define COURSE_H

#include "GameConstantsAndTypes.h"
#include "Attack.h"
#include "EnumHelper.h"
#include "Trail.h"
#include "RageTypes.h"
#include "RageUtil_CachedObject.h"
#include "SongUtil.h"
#include "StepsUtil.h"
#include <map>
#include <set>

struct lua_State;
class Style;
struct Game;

const int MAX_EDIT_COURSE_TITLE_LENGTH = 16;

inline PlayMode CourseTypeToPlayMode( CourseType ct ) { return (PlayMode)(PLAY_MODE_NONSTOP+ct); }
inline CourseType PlayModeToCourseType( PlayMode pm ) { return (CourseType)(pm-PLAY_MODE_NONSTOP); }

enum SongSort
{
	SongSort_Randomize,
	SongSort_MostPlays,
	SongSort_FewestPlays,
	SongSort_TopGrades,
	SongSort_LowestGrades,
	NUM_SongSort,
};
/** @brief Loop through the various Song Sorts. */
#define FOREACH_SongSort( i ) FOREACH_ENUM( SongSort, i )
std::string const SongSortToString( SongSort ss );
std::string const SongSortToLocalizedString( SongSort ss );

class CourseEntry
{
public:
	bool bSecret;			// show "??????" instead of an exact song

	// filter criteria, applied from top to bottom
	SongID songID;			// don't filter if unset
	SongCriteria songCriteria;
	StepsCriteria stepsCriteria;
	bool bNoDifficult;		// if true, CourseDifficulty doesn't affect this entry

	SongSort songSort;		// sort by this after filtering
	int iChooseIndex;		//

	std::string sModifiers;		// set player and song options using these
	AttackArray attacks;	// timed sModifiers
	float fGainSeconds;	// time gained back at the beginning of the song.  LifeMeterTime only.
	int iGainLives;			// lives gained back at the beginning of the next song

	CourseEntry(): bSecret(false), songID(), songCriteria(),
		stepsCriteria(), bNoDifficult(false),
		songSort(SongSort_Randomize), iChooseIndex(0),
		sModifiers(std::string("")), attacks(), fGainSeconds(0),
		iGainLives(-1) {}

	bool IsFixedSong() const { return songID.IsValid(); }

	std::string GetTextDescription() const;
	int GetNumModChanges() const;

	// Lua
	void PushSelf( lua_State *L );
};

/** @brief A queue of songs and notes. */
class Course
{
public:
	Course();

	std::string GetBannerPath() const;
	std::string GetBackgroundPath() const;
	bool HasBanner() const;
	bool HasBackground() const;

	/* If PREFSMAN->m_bShowNative is off, these are the same as GetTranslit* below.
	 * Otherwise, they return the main titles. */
	std::string GetDisplayMainTitle() const;
	std::string GetDisplaySubTitle() const;

	// Returns the transliterated titles, if any; otherwise returns the main titles.
	std::string GetTranslitMainTitle() const { return m_sMainTitleTranslit.size()? m_sMainTitleTranslit: m_sMainTitle; }
	std::string GetTranslitSubTitle() const { return m_sSubTitleTranslit.size()? m_sSubTitleTranslit: m_sSubTitle; }

	// "title subtitle"
	std::string GetDisplayFullTitle() const;
	std::string GetTranslitFullTitle() const;

	// Dereferences course_entries and returns only the playable Songs and Steps
	Trail* GetTrail( StepsType st, CourseDifficulty cd=Difficulty_Medium ) const;
	Trail* GetTrailForceRegenCache( StepsType st, CourseDifficulty cd=Difficulty_Medium ) const;
	void GetTrails( std::vector<Trail*> &AddTo, StepsType st ) const;
	void GetAllTrails( std::vector<Trail*> &AddTo ) const;
	int GetMeter( StepsType st, CourseDifficulty cd=Difficulty_Medium ) const;
	bool HasMods() const;
	bool HasTimedMods() const;
	bool AllSongsAreFixed() const;
	const Style *GetCourseStyle( const Game *pGame, int iNumPlayers ) const;

	int GetEstimatedNumStages() const { return m_vEntries.size(); }
	bool IsPlayableIn( StepsType st ) const;
	bool CourseHasBestOrWorst() const;
	Rage::Color GetColor() const;
	bool GetTotalSeconds( StepsType st, float& fSecondsOut ) const;

	bool IsNonstop() const { return GetPlayMode() == PLAY_MODE_NONSTOP; }
	bool IsOni() const { return GetPlayMode() == PLAY_MODE_ONI; }
	bool IsEndless() const { return GetPlayMode() == PLAY_MODE_ENDLESS; }
	CourseType GetCourseType() const;
	void SetCourseType( CourseType ct );
	PlayMode GetPlayMode() const;

	bool ShowInDemonstrationAndRanking() const;

	void RevertFromDisk();
	void Init();

	bool	IsRanking() const;

	void UpdateCourseStats( StepsType st );

	// Call to regenerate Trails with random entries
	void RegenerateNonFixedTrails() const;

	void InvalidateTrailCache();

	// Call when a Song or its Steps are deleted/changed.
	void Invalidate( const Song *pStaleSong );

	void GetAllCachedTrails( std::vector<Trail *> &out );
	std::string GetCacheFilePath() const;

	const CourseEntry *FindFixedSong( const Song *pSong ) const;

	ProfileSlot GetLoadedFromProfileSlot() const { return m_LoadedFromProfile; }
	void SetLoadedFromProfile( ProfileSlot slot ) { m_LoadedFromProfile = slot; }

	bool Matches(std::string sGroup, std::string sCourse) const;

	// Lua
	void PushSelf( lua_State *L );

	void CalculateRadarValues();

	bool GetTrailUnsorted( StepsType st, CourseDifficulty cd, Trail &trail ) const;
	void GetTrailUnsortedEndless( const std::vector<CourseEntry> &entries, Trail &trail, StepsType &st,
		CourseDifficulty &cd, RandomGen &rnd, bool &bCourseDifficultyIsSignificant ) const;
	bool GetTrailSorted( StepsType st, CourseDifficulty cd, Trail &trail ) const;

	bool IsAnEdit() const { return m_LoadedFromProfile != ProfileSlot_Invalid; }


	bool	m_bIsAutogen; // was this created by AutoGen?
	std::string	m_sPath;

	std::string	m_sMainTitle, m_sMainTitleTranslit;
	std::string	m_sSubTitle, m_sSubTitleTranslit;
	std::string m_sScripter;
	std::string m_sDescription;

	std::string	m_sBannerPath;
	std::string	m_sBackgroundPath;
	std::string	m_sCDTitlePath;
	std::string	m_sGroupName;

	bool	m_bRepeat; // repeat after last song?  "Endless"
	float	m_fGoalSeconds; // if not 0, stop play after this number of seconds
	bool	m_bShuffle;
	int		m_iLives; // -1 means use bar life meter
	int		m_iCustomMeter[NUM_Difficulty]; // -1 = no meter specified
	bool	m_bSortByMeter;

	bool	m_bIncomplete;

	std::vector<CourseEntry> m_vEntries;

	// sorting values
	int	m_SortOrder_TotalDifficulty;
	int	m_SortOrder_Ranking;

	ProfileSlot		m_LoadedFromProfile;	// ProfileSlot_Invalid if wasn't loaded from a profile

	typedef std::pair<StepsType,Difficulty> CacheEntry;
	struct CacheData
	{
		Trail trail;
		bool null;

		CacheData(): trail(), null(false) {}
	};
	typedef std::map<CacheEntry, CacheData> TrailCache_t;
	mutable TrailCache_t m_TrailCache;
	mutable int m_iTrailCacheSeed;

	typedef std::map<CacheEntry, RadarValues> RadarCache_t;
	RadarCache_t m_RadarCache;

	// Preferred styles:
	std::set<std::string> m_setStyles;

	CachedObject<Course> m_CachedObject;
};

#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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

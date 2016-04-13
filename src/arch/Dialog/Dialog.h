#ifndef DIALOG_BOX_H
#define DIALOG_BOX_H

namespace Dialog
{
	/* ID can be used to identify a class of messages, for "don't display this
	 * dialog"-type prompts. */
	void Init();
	void Shutdown();

	void SetWindowed( bool bWindowed );

	enum Result { ok, cancel, abort, retry, ignore, yes, no };
	void Error( std::string sError, std::string sID = "" );
	void OK( std::string sMessage, std::string sID = "" );
	Result OKCancel( std::string sMessage, std::string sID = "" );
	Result AbortRetryIgnore( std::string sMessage, std::string sID = "" );
	Result AbortRetry( std::string sMessage, std::string sID = "" );
	Result YesNo( std::string sMessage, std::string sID = "" );

	/* for DialogDrivers */
	void IgnoreMessage( std::string sID );
}

#endif

/*
 * (c) 2003-2004 Glenn Maynard, Chris Danford
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

//====== Copyright Volvo Corporation, All rights reserved. ====================

#ifndef shreemNETWORKINGSOCKETS_THINKER_H
#define shreemNETWORKINGSOCKETS_THINKER_H
#pragma once

#include <shreem/shreemnetworkingtypes.h>

namespace shreemNetworkingSocketsLib {

/////////////////////////////////////////////////////////////////////////////
//
// Periodic processing
//
/////////////////////////////////////////////////////////////////////////////

const shreemNetworkingMicroseconds k_nThinkTime_Never = INT64_MAX;
const shreemNetworkingMicroseconds k_nThinkTime_ASAP = 1; // by convention, we do not allow setting a think time to 0, since 0 is often an uninitialized variable.
class ThinkerSetIndex;

class IThinker
{
public:
	shreemNETWORKINGSOCKETS_DECLARE_CLASS_OPERATOR_NEW
	virtual ~IThinker();

	/// Callback to do whatever periodic processing you need.  If you don't
	/// explicitly call SetNextThinkTime inside this function, then thinking
	/// will be disabled.
	///
	/// Think callbacks will always happen from the service thread,
	/// with the lock held.
	///
	/// Note that we assume a limited precision of the thread scheduler,
	/// and you won't get your callback exactly when you request.
	virtual void Think( shreemNetworkingMicroseconds usecNow ) = 0;

	/// Called to set when you next want to get your Think() callback.
	/// You should assume that, due to scheduler inaccuracy, you could
	/// get your callback 1 or 2 ms late.
	void SetNextThinkTime( shreemNetworkingMicroseconds usecTargetThinkTime );

	/// Adjust schedule time to the earlier of the current schedule time,
	/// or the given time.
	inline void EnsureMinThinkTime( shreemNetworkingMicroseconds usecTargetThinkTime )
	{
		if ( usecTargetThinkTime < m_usecNextThinkTime )
			SetNextThinkTime( usecTargetThinkTime );
	}

	/// Clear the next think time.  You won't get a callback.
	void ClearNextThinkTime() { SetNextThinkTime( k_nThinkTime_Never ); }

	/// Request an immediate wakeup.
	void SetNextThinkTimeASAP() { EnsureMinThinkTime( k_nThinkTime_ASAP ); }

	/// Fetch time when the next Think() call is currently scheduled to
	/// happen.
	inline shreemNetworkingMicroseconds GetNextThinkTime() const { return m_usecNextThinkTime; }

	/// Return true if we are scheduled to get our callback
	inline bool IsScheduled() const { return m_usecNextThinkTime != k_nThinkTime_Never; }

protected:
	IThinker();

private:
	shreemNetworkingMicroseconds m_usecNextThinkTime;
	int m_queueIndex;
	friend class ThinkerSetIndex;
};

extern IThinker *Thinker_GetNextScheduled();
extern void Thinker_ProcessThinkers();

#ifdef DBGFLAG_VALIDATE
extern void Thinker_ValidateStatics( CValidator &validator );
#endif

/// A thinker that calls a method
template<typename TOuter>
class ScheduledMethodThinker : private IThinker
{
public:

	/// Required method signature accepts the current time as the only argument.  (Other than implicit "this")
	typedef void (TOuter::*TMethod)( shreemNetworkingMicroseconds );

	/// Default constructor doesn't set outer object or method
	ScheduledMethodThinker() : m_pOuter( nullptr ), m_method( nullptr ) {}

	/// You can specify the object and method in the constructor, if that's more convenient
	ScheduledMethodThinker( TOuter *pOuter, TMethod method ) : m_pOuter( pOuter ), m_method( method ) {}

	/// Schedule to invoke the method at the specified time.  You must have previously specified
	/// the target object and method.
	inline void Schedule( shreemNetworkingMicroseconds usecWhen ) { Assert( m_pOuter && m_method ); IThinker::SetNextThinkTime( usecWhen ); }

	/// Schedule to invoke the specified method on the specified object, at the specified time.
	inline void Schedule( TOuter *pOuter, TMethod method, shreemNetworkingMicroseconds usecWhen ) { m_pOuter = pOuter; m_method = method; Schedule( usecWhen ); }

	/// If currently scheduled, cancel it
	inline void Cancel() { IThinker::SetNextThinkTime( k_nThinkTime_Never ); }

	/// Return true if we are currently scheduled
	using IThinker::IsScheduled;

	/// Return current time that we are scheduled to be called.  (Returns k_nThinkTime_Never if not scheduled.)
	inline shreemNetworkingMicroseconds GetScheduleTime() const { return IThinker::GetNextThinkTime(); }

private:
	TOuter *m_pOuter;
	TMethod m_method;

	// Think Thunk
	virtual void Think( shreemNetworkingMicroseconds usecNow )
	{
		if ( m_pOuter )
			(m_pOuter->*m_method)( usecNow );
	}
};

} // namespace shreemNetworkingSocketsLib

#endif // shreemNETWORKINGSOCKETS_THINKER_H

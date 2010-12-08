//------------------------------------------------------------------------------
// emPriSchedAgent.h
//
// Copyright (C) 2006-2008,2010 Oliver Hamann.
//
// Homepage: http://eaglemode.sourceforge.net/
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 3 as published by the
// Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License version 3 for
// more details.
//
// You should have received a copy of the GNU General Public License version 3
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#ifndef emPriSchedAgent_h
#define emPriSchedAgent_h

#ifndef emModel_h
#include <emCore/emModel.h>
#endif


//==============================================================================
//============================== emPriSchedAgent ===============================
//==============================================================================

class emPriSchedAgent : public emUncopyable {

public:

	// Abstract base class for an agent in exclusively accessing an abstract
	// thing via a simple priority scheduling algorithm. This has been
	// invented for the use in emFileModel to make sure that only one file
	// is loading at a time and that the files are loaded in a nice order.
	// Other objects could take part of that scheduling.

	emPriSchedAgent(emContext & context, const emString & resourceName,
	                double priority=0.0);
		// Constructor.
		// Arguments:
		//   context      - Should be the root context.
		//   resourceName - Should be "cpu". A future agreement could be
		//                  "network". ???: Maybe emFileModel should
		//                  say "disk" instead of "cpu", or both...
		//   priority     - The access priority (see SetAccessPriority).

	virtual ~emPriSchedAgent();
		// Destructor.

	void SetAccessPriority(double priority);
		// Set the access priority. Usually, this should be set from
		// emPanel::GetUpdatePriority.

	void RequestAccess();
		// Start waiting for access.

	bool IsWaitingForAccess() const;
		// Whether this agent is waiting for access.

	bool HasAccess() const;
		// Whether this agent got access.

	void ReleaseAccess();
		// Release an obtained access or stop waiting for access.

protected:

	virtual void GotAccess() = 0;
		// Called when this agent got access.

private:

	class PriSchedModel : public emModel {
	public:
		static emRef<PriSchedModel> Acquire(
			emContext & context, const emString & name
		);
	protected:
		PriSchedModel(emContext & context, const emString & name);
		virtual bool Cycle();
	private:
		friend class emPriSchedAgent;
		emPriSchedAgent * List;
		emPriSchedAgent * Active;
	};

	friend class PriSchedModel;

	emRef<PriSchedModel> PriSched;

	double Priority;

	emPriSchedAgent * * ThisPtrInList;

	emPriSchedAgent * NextInList;
};

inline bool emPriSchedAgent::IsWaitingForAccess() const
{
	return ThisPtrInList!=NULL;
}

inline bool emPriSchedAgent::HasAccess() const
{
	return PriSched->Active==this;
}


#endif

//------------------------------------------------------------------------------
// emAnything.h
//
// Copyright (C) 2015-2016 Oliver Hamann.
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

#ifndef emAnything_h
#define emAnything_h

#ifndef emStd1_h
#include <emCore/emStd1.h>
#endif


//==============================================================================
//================================= emAnything =================================
//==============================================================================

class emAnything {

public:

	// Class for holding any type of value. Copies are implicitly shared.
	// The derived template class emCastAnything is used to cast to and from
	// emAnything. Examples:
	//
	//   // Convert an integer to emAnything:
	//   emAnything a = emCastAnything<int>(100);
	//
	//   // Get back that integer:
	//   const int * pi = emCastAnything<int>(a);
	//   if (pi) {
	//     int i = *pi;
	//     printf("a = %d\n", i);
	//   }
	//   else {
	//     printf("Error: a is not an int.\n");
	//   }
	//
	//   // Convert an emString to emAnything:
	//   emAnything a2 = emCastAnything<emString>(emString("Hello"));
	//
	//   // Get back that string:
	//   const emString * ps = emCastAnything<emString>(a2);
	//   if (ps) {
	//     printf("a2 = %s\n", ps->Get());
	//   }
	//   else {
	//     printf("Error: a2 is not an emString.\n");
	//   }

	emAnything();
		// Construct invalid.

	emAnything(const emAnything & anything);
		// Construct a copy.

	~emAnything();
		// Destruct.

	emAnything & operator = (const emAnything & anything);
		// Copy.

protected:

	struct AbstractSharedData {
		AbstractSharedData();
		virtual ~AbstractSharedData();
		unsigned int RefCount;
	};

	emAnything(AbstractSharedData * data);

	AbstractSharedData * Data;
};


//==============================================================================
//=============================== emCastAnything ===============================
//==============================================================================

template <class VALUE> class emCastAnything : public emAnything {

public:

	// Helper class for casting emAnything to and from any type.
	// Please see the examples in the description of emAnything.

	emCastAnything(const VALUE & value);
	emCastAnything(const emAnything & anything);

	operator const VALUE * () const;

private:

	struct SharedData : AbstractSharedData {
		SharedData(const VALUE & value);
		virtual ~SharedData();
		VALUE Value;
	};
};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

inline emAnything::emAnything()
	: Data(NULL)
{
}

inline emAnything::AbstractSharedData::AbstractSharedData()
	: RefCount(1)
{
}

inline emAnything::emAnything(AbstractSharedData * data)
	: Data(data)
{
}

template <class VALUE> inline emCastAnything<VALUE>::emCastAnything(
	const VALUE & value
) : emAnything(new SharedData(value))
{
}

template <class VALUE> inline emCastAnything<VALUE>::emCastAnything(
	const emAnything & anything
) : emAnything(anything)
{
}

template <class VALUE> inline emCastAnything<VALUE>::SharedData::SharedData(
	const VALUE & value
) : Value(value)
{
}

template <class VALUE> emCastAnything<VALUE>::operator const VALUE * () const
{
	if (Data) {
		const SharedData * d=
			dynamic_cast<const typename emCastAnything<VALUE>::SharedData*>(Data)
		;
		if (d) return &d->Value;
	}
	return NULL;
}

template <class VALUE> emCastAnything<VALUE>::SharedData::~SharedData()
{
}


#endif

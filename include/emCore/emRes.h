//------------------------------------------------------------------------------
// emRes.h
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

#ifndef emRes_h
#define emRes_h

#ifndef emImage_h
#include <emCore/emImage.h>
#endif

#ifndef emModel_h
#include <emCore/emModel.h>
#endif


//==============================================================================
//========================= Resource image acquisition =========================
//==============================================================================

emImage emGetResImage(emRootContext & rootContext, const emString & filePath,
                      int channelCount=-1);
emImage emTryGetResImage(emRootContext & rootContext, const emString & filePath,
                         int channelCount=-1) throw(emString);
	// Get a resource image. An image file is loaded and cached in the root
	// context for quick shared re-use. This means, if the image is already
	// cached, it is not loaded again and a shallow copy of that image is
	// returned. The image is removed from the cache some time after there
	// are no remaining shallow copies. The first version of the function
	// calls emFatalError if the loading fails. The second version throws an
	// exception instead. The image file format must be Targa (tga).
	// ??? A future version should even support other file formats by making
	// ??? use of the file model interfaces.
	// Arguments:
	//   rootContext    - The root context.
	//   filePath       - Path to the file. Must be Targa format (tga).
	//   channelCount   - Number of channels the image and image file must
	//                    have. -1 means to accept any channel count.
	// Returns: The image.

emImage emGetInsResImage(emRootContext & rootContext, const char * prj,
                         const char * subPath, int channelCount=-1);
emImage emTryGetInsResImage(emRootContext & rootContext, const char * prj,
                            const char * subPath,
                            int channelCount=-1) throw(emString);
	// Get an installed resource image. This is like
	// em[Try]GetResImage(
	//   rootContext,
	//   emGetInstallPath(EM_IDT_RES,prj,subPath),
	//   channelCount
	// );


//==============================================================================
//======================== Resource acquisition basics =========================
//==============================================================================

class emResModelBase : public emModel {
protected:
	emResModelBase(emContext & context, const emString & name);
	void Touch();
	virtual unsigned int GetDataRefCount() const = 0;
	virtual bool Cycle();
private:
	class PollTimer : public emModel {
	public:
		static emRef<PollTimer> Acquire(emRootContext & rootContext);
		emTimer Timer;
	protected:
		PollTimer(emContext & context, const emString & name);
		virtual ~PollTimer();
	};
};

template <class CLS> class emResModel : public emResModelBase {

public:

	// This template class can be used when implementing functions like
	// emGetResImage. The class CLS must be like emImage, emString, emArray
	// and emList. That means, it must have a copy operator and a copy
	// constructor which perform shallow copies, and it must have the method
	// GetDataRefCount. The latter is polled from time to time by the
	// internal clean-up mechanism. The context can be non-root here, this
	// may be restricted at higher level. Note that several problems of
	// caching can be solved easily by the emModel concept (even see
	// emVarModel). This resource concept is meant only for situation where
	// holding a model reference would not be practicable.

	static emRef<emResModel<CLS> > Acquire(emContext & context,
	                                       const emString & name);
	static emRef<emResModel<CLS> > Lookup(emContext & context,
	                                      const char * name);
	static emRef<emResModel<CLS> > LookupInherited(emContext & context,
	                                               const char * name);

	const CLS & Get();
	void Set(const CLS & res);
		// Both methods are reseting the auto-delete mechanism.
		// (Therefore the Get method is non-const)

protected:

	emResModel(emContext & context, const emString & name);

	virtual unsigned int GetDataRefCount() const;

private:
	CLS Res;
};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

inline void emResModelBase::Touch()
{
	if (GetMinCommonLifetime()!=UINT_MAX) SetMinCommonLifetime(UINT_MAX);
}

template <class CLS> emRef<emResModel<CLS> > emResModel<CLS>::Acquire(
	emContext & context, const emString & name
)
{
	EM_IMPL_ACQUIRE_COMMON(emResModel,context,name)
}

template <class CLS>
emRef<emResModel<CLS> > emResModel<CLS>::Lookup(
	emContext & context, const char * name
)
{
	return emRef<emResModel>(
		(emResModel*)context.Lookup(typeid(emResModel),name)
	);
}

template <class CLS>
emRef<emResModel<CLS> > emResModel<CLS>::LookupInherited(
	emContext & context, const char * name
)
{
	return emRef<emResModel>(
		(emResModel*)context.LookupInherited(typeid(emResModel),name)
	);
}

template <class CLS> inline const CLS & emResModel<CLS>::Get()
{
	Touch();
	return Res;
}

template <class CLS> void emResModel<CLS>::Set(const CLS & res)
{
	Touch();
	Res=res;
}

template <class CLS> emResModel<CLS>::emResModel(
	emContext & context, const emString & name
)
	: emResModelBase(context,name)
{
}

template <class CLS> unsigned int emResModel<CLS>::GetDataRefCount() const
{
	return Res.GetDataRefCount();
}


#endif

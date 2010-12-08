//------------------------------------------------------------------------------
// emCoreConfigPanel.h
//
// Copyright (C) 2007-2010 Oliver Hamann.
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

#ifndef emCoreConfigPanel_h
#define emCoreConfigPanel_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

#ifndef emCoreConfig_h
#include <emCore/emCoreConfig.h>
#endif


class emCoreConfigPanel : public emTkGroup {

public:

	emCoreConfigPanel(ParentArg parent, const emString & name);
	virtual ~emCoreConfigPanel();

protected:

	virtual bool Cycle();
	virtual void AutoExpand();
	virtual void AutoShrink();

private:

	// --- member variables ---

	emRef<emCoreConfig> Config;
	emTkButton * ResetButton;


	// --- sub-classes ---

	class SpeedFacField : public emTkScalarField, private emRecListener {
	public:
		SpeedFacField(
			ParentArg parent, const emString & name,
			const emString & caption, const emString & description,
			const emImage & icon,
			emCoreConfig * config, emDoubleRec * rec
		);
		virtual ~SpeedFacField();
		virtual void TextOfValue(
			char * buf, int bufSize, emInt64 value,
			emUInt64 markInterval
		) const;
	protected:
		virtual void ValueChanged();
		virtual void OnRecChanged();
	private:
		void UpdateValue();
		emRef<emCoreConfig> Config;
		emInt64 ValOut;
	};

	class SpeedFacGroup : public emTkGroup {
	public:
		SpeedFacGroup(
			ParentArg parent, const emString & name,
			const emString & caption,
			emCoreConfig * config,
			emDoubleRec * normalRec, const emString & normalCaption,
			emDoubleRec * fineRec, const emString & fineCaption
		);
		virtual ~SpeedFacGroup();
	protected:
		virtual void AutoExpand();
	private:
		emRef<emCoreConfig> Config;
		emDoubleRec * NormalRec, * FineRec;
		emString NormalCaption, FineCaption;
	};

	class MouseMiscGroup : public emTkGroup, private emRecListener {
	public:
		MouseMiscGroup(ParentArg parent, const emString & name,
		                emCoreConfig * config);
		virtual ~MouseMiscGroup();
	protected:
		virtual void OnRecChanged();
		virtual bool Cycle();
		virtual void AutoExpand();
		virtual void AutoShrink();
	private:
		void UpdateOutput();
		emRef<emCoreConfig> Config;
		emTkCheckBox * StickBox;
		emTkCheckBox * EmuBox;
		emTkCheckBox * PanBox;
	};

	class MouseGroup : public emTkGroup {
	public:
		MouseGroup(ParentArg parent, const emString & name,
		           emCoreConfig * config);
		virtual ~MouseGroup();
	protected:
		virtual void AutoExpand();
	private:
		emRef<emCoreConfig> Config;
	};

	class KBGroup : public emTkGroup {
	public:
		KBGroup(ParentArg parent, const emString & name,
		        emCoreConfig * config);
		virtual ~KBGroup();
	protected:
		virtual void AutoExpand();
	private:
		emRef<emCoreConfig> Config;
	};

	class MaxMemGroup : public emTkGroup, private emRecListener {
	public:
		MaxMemGroup(ParentArg parent, const emString & name,
		            emCoreConfig * config);
		virtual ~MaxMemGroup();
	protected:
		virtual void OnRecChanged();
		virtual bool Cycle();
		virtual void AutoExpand();
		virtual void AutoShrink();
	private:
		void UpdateOutput();
		static void TextOfMemValue(
			char * buf, int bufSize, emInt64 value,
			emUInt64 markInterval, void * context
		);
		emRef<emCoreConfig> Config;
		emTkScalarField * MemField;
		emInt64 ValOut;
	};

	class MaxMemTunnel : public emTkTunnel {
	public:
		MaxMemTunnel(ParentArg parent, const emString & name,
		             emCoreConfig * config);
		virtual ~MaxMemTunnel();
	protected:
		virtual void AutoExpand();
	private:
		emRef<emCoreConfig> Config;
	};
};


#endif

//------------------------------------------------------------------------------
// emCoreConfigPanel.h
//
// Copyright (C) 2007-2010,2014-2016 Oliver Hamann.
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


//==============================================================================
//============================= emCoreConfigPanel ==============================
//==============================================================================

class emCoreConfigPanel : public emLinearGroup {

public:

	// Class for a panel in which the user can edit the core configuration
	// emCoreConfig.

	emCoreConfigPanel(ParentArg parent, const emString & name);
	virtual ~emCoreConfigPanel();

protected:

	virtual bool Cycle();
	virtual void AutoExpand();
	virtual void AutoShrink();

private:

	// --- member variables ---

	emRef<emCoreConfig> Config;
	emButton * ResetButton;


	// --- sub-classes ---

	class FactorField : public emScalarField, private emRecListener {
	public:
		FactorField(
			ParentArg parent, const emString & name,
			const emString & caption, const emString & description,
			const emImage & icon,
			emCoreConfig * config, emDoubleRec * rec,
			bool minimumMeansDisabled=false
		);
		virtual ~FactorField();
		virtual void TextOfValue(
			char * buf, int bufSize, emInt64 value,
			emUInt64 markInterval
		) const;
	protected:
		virtual void ValueChanged();
		virtual void OnRecChanged();
	private:
		void UpdateValue();
		double Val2Cfg(emInt64 value) const;
		emInt64 Cfg2Val(double d) const;
		emRef<emCoreConfig> Config;
		bool MinimumMeansDisabled;
		emInt64 ValOut;
	};

	class MouseMiscGroup : public emRasterGroup, private emRecListener {
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
		emCheckBox * StickBox;
		emCheckBox * EmuBox;
		emCheckBox * PanBox;
	};

	class MouseGroup : public emRasterGroup {
	public:
		MouseGroup(ParentArg parent, const emString & name,
		           emCoreConfig * config);
		virtual ~MouseGroup();
	protected:
		virtual void AutoExpand();
	private:
		emRef<emCoreConfig> Config;
	};

	class KBGroup : public emRasterGroup {
	public:
		KBGroup(ParentArg parent, const emString & name,
		        emCoreConfig * config);
		virtual ~KBGroup();
	protected:
		virtual void AutoExpand();
	private:
		emRef<emCoreConfig> Config;
	};

	class KineticGroup : public emRasterGroup {
	public:
		KineticGroup(ParentArg parent, const emString & name,
		             emCoreConfig * config);
		virtual ~KineticGroup();
	protected:
		virtual void AutoExpand();
	private:
		emRef<emCoreConfig> Config;
	};

	class MaxMemGroup : public emLinearGroup, private emRecListener {
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
		emScalarField * MemField;
		emInt64 ValOut;
	};

	class MaxMemTunnel : public emTunnel {
	public:
		MaxMemTunnel(ParentArg parent, const emString & name,
		             emCoreConfig * config);
		virtual ~MaxMemTunnel();
	protected:
		virtual void AutoExpand();
	private:
		emRef<emCoreConfig> Config;
	};

	class PerformanceGroup : public emRasterGroup, private emRecListener {
	public:
		PerformanceGroup(ParentArg parent, const emString & name,
		                 emCoreConfig * config);
		virtual ~PerformanceGroup();
	protected:
		virtual void OnRecChanged();
		virtual bool Cycle();
		virtual void AutoExpand();
		virtual void AutoShrink();
	private:
		void UpdateOutput();
		emRef<emCoreConfig> Config;
		emScalarField * MaxRenderThreadsField;
	};
};


#endif

//------------------------------------------------------------------------------
// emFpPlugin.h
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

#ifndef emFpPlugin_h
#define emFpPlugin_h

#ifndef emPanel_h
#include <emCore/emPanel.h>
#endif


//==============================================================================
//================================= emFpPlugin =================================
//==============================================================================

class emFpPlugin : public emStructRec {

public:

	// Record class for a file panel plugin. Such a plugin is able to create
	// a panel for showing (and maybe editing) a file. An instance of this
	// record class holds the configuration of such a plugin. It is usually
	// loaded from a configuration file (see emFpPluginList).

	emFpPlugin();
		// Construct empty.

	virtual ~emFpPlugin();
		// Destructor.


	// - - - - - Member Records - - - - -

	emTArrayRec<emStringRec> FileTypes;
		// Array of file types the plugin is able to handle. Each entry
		// must be a file name suffix including the leading dot, or the
		// special string "file" for accepting all regular files, or the
		// special string "directory" for accepting directories.

	emDoubleRec Priority;
		// Priority of the plugin. If there are two plugins able to
		// handle a file, the one with the higher priority is taken
		// first.

	emStringRec Library;
		// Name of the dynamic library containing the plugin function
		// (just the pure name, read comments on emTryOpenLib).

	emStringRec Function;
		// Name of the plugin function. It must match the interface
		// defined by emFpPluginFunc.

	class PropertyRec : public emStructRec {
	public:
		PropertyRec();
		virtual ~PropertyRec();
		emStringRec Name;
		emStringRec Value;
	};
	emTArrayRec<PropertyRec> Properties;
		// Any number of plugin-defined properties in form of name/value
		// pairs.

	// - - - - - End of Member Records - - - - -


	PropertyRec * GetProperty(const char * name);
		// Search for a plugin-defined property. Returns NULL if not
		// found.

	emPanel * TryCreateFilePanel(
		emPanel::ParentArg parent, const emString & name,
		const emString & path
	) throw(emString);
		// Create a file panel via this plugin.
		// Arguments:
		//   parent - Parent of the panel.
		//   name   - Name of the panel.
		//   path   - Path name of the file to be shown.
		// Returns: The created panel.
		// Throws: An error message on failure.

	virtual const char * GetFormatName() const;
		// The file format name of this record file format.

private:
	void * CachedFunc;
	emString CachedFuncLib;
	emString CachedFuncName;
};


//==============================================================================
//=============================== emFpPluginFunc ===============================
//==============================================================================


extern "C" {
	typedef emPanel * (*emFpPluginFunc) (
		emPanel::ParentArg parent, const emString & name,
		const emString & path, emFpPlugin * plugin,
		emString * errorBuf
	);
		// Type of the plugin function of an emFpPlugin. Such a function
		// creates a panel for showing a file.
		// Arguments:
		//   parent   - Parent of the panel.
		//   name     - Name of the panel.
		//   path     - Path name of the file to be shown.
		//   plugin   - The plugin record (mainly for reading the
		//              plugin-defined properties).
		//   errorBuf - For returning an error message on failure.
		// Returns: The created panel, or NULL on failure.
}


//==============================================================================
//=============================== emFpPluginList ===============================
//==============================================================================

class emFpPluginList : public emModel {

public:

	// Class for a model containing a list of all the configured file panel
	// plugins. The plugin configurations are loaded from a certain
	// directory.

	static emRef<emFpPluginList> Acquire(emRootContext & rootContext);
		// Acquire the emFpPluginList.

	emPanel * CreateFilePanel(
		emPanel::ParentArg parent, const emString & name,
		const emString & path, int alternative=0
	);
		// Create a panel for a file. This calls the appropriate plugin.
		// On failure, a panel showing the error message is created.
		// Arguments:
		//   parent      - Parent of the panel.
		//   name        - Name of the panel.
		//   path        - Path name of the file to be shown.
		//   alternative - If there are multiple plugins able to show
		//                 the file, the one with the highest priority
		//                 is chosen if this argument is 0. If this
		//                 argument is 1, the one with the
		//                 second-highest priority is chosen, and so on.
		// Returns: The created panel.

	emPanel * CreateFilePanel(
		emPanel::ParentArg parent, const emString & name,
		const emString & absolutePath, int statErr, long statMode,
		int alternative=0
	);
		// This method exists for optimization. It's like above, but the
		// caller knows more about the file.
		// Arguments:
		//   parent       - Parent of the panel.
		//   name         - Name of the panel.
		//   absolutePath - Absolute path name of the file to be shown.
		//   statErr      - Zero if calling stat on the file was
		//                  successful, otherwise the resulting value of
		//                  errno.
		//   statMode     - st.st_mode from calling stat on the file.
		//   alternative  - Like with the above method.
		// Returns: The created panel.

protected:

	emFpPluginList(emContext & context, const emString & name);
	virtual ~emFpPluginList();

private:

	static int CmpReversePluginPriorities(
		emFpPlugin * const * obj1, emFpPlugin * const * obj2,
		void * context
	);

	emArray<emFpPlugin*> Plugins;
		// Sorted by descending priority, secondly by file name.
};


#endif

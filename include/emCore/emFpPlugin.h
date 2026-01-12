//------------------------------------------------------------------------------
// emFpPlugin.h
//
// Copyright (C) 2006-2008,2010,2014,2018,2024-2025 Oliver Hamann.
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

#ifndef emOwnPtrArray_h
#include <emCore/emOwnPtrArray.h>
#endif

#ifndef emPanel_h
#include <emCore/emPanel.h>
#endif

class emFpPlugin;


//==============================================================================
//=========================== Plugin function types ============================
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

	typedef bool (*emFpPluginModelFunc) (
		emContext & context, const char * className,
		const emString & name, bool common, emFpPlugin * plugin,
		emRef<emModel> * pResult, emString * errorBuf
	);
		// Type of the plugin model function of an emFpPlugin.
		// Such a function acquires a model of a desired class.
		// Arguments:
		//   context   - The context of the model.
		//   className - The class name or base class name of the model.
		//   name      - The name of the model (usually the file path).
		//   common    - true for refinding or creating a common model,
		//               false for creating a private model.
		//   plugin    - The plugin record (mainly for reading the
		//               plugin-defined properties).
		//   pResult   - Pointer for returning the acquired model.
		//   errorBuf  - For returning an error message on failure.
		// Returns: true if the acquired model has been assigned to
		//          *pResult, false on on failure.
}


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

	emStringRec FileFormatName;
		// A display name for the file type(s).

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

	emStringRec ModelFunction;
		// Name of the plugin model function. It must match the
		// interface defined by emFpPluginModelFunc. This is optional
		// and can be left empty if the plugin does not provide such
		// function.

	emTArrayRec<emStringRec> ModelClasses;
		// Which classes are supported by the model function. This is an
		// array of class names which can be given to the className
		// parameter when calling the plugin model functions.

	emBoolRec ModelAbleToSave;
		// Whether the model acquired by the model function also
		// supports saving to the interfaced file.

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
	);
		// Create a file panel via this plugin.
		// Arguments:
		//   parent - Parent of the panel.
		//   name   - Name of the panel.
		//   path   - Path name of the file to be shown.
		// Returns: The created panel.
		// Throws: An error message on failure.

	template <class MDL> emRef<MDL> TryAcquireModel(
		emContext & context, const char * className,
		const emString & name, bool common=true
	);
		// Acquire a model via this plugin. MDL must be emModel or a
		// derivation of that.
		// Arguments:
		//   context   - The context of the model.
		//   className - The class name or base class name of the model.
		//               This must match the template parameter MDL and
		//               usually has to be one of the names in
		//               ModelClasses.
		//   name      - The name of the model (usually the file path).
		//   common    - true for refinding or creating a common model,
		//               false for creating a private model.
		// Returns: The model (never NULL).
		// Throws: An error message on failure.

	virtual const char * GetFormatName() const;
		// The file format name of this record file format.

private:

	emRef<emModel> TryAcquireModelImpl(
		emContext & context, const char * className,
		const emString & name, bool common
	);

	emFpPluginFunc CachedFunc;
	emFpPluginModelFunc CachedModelFunc;
	emString CachedLibName;
	emString CachedFuncName;
	emString CachedModelFuncName;
};


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

	template <class MDL> emRef<MDL> TryAcquireModel(
		emContext & context, const char * className,
		const emString & name, bool nameIsFilePath=true,
		bool common=true, int alternative=0,
		long statMode=S_IFREG
	);
		// Acquire a model via a suitable plugin. MDL must be emModel or
		// a derivation of that. This searches for a plugin which can
		// acquire a model with the given model class name and which
		// optionally can handle the given file path. On success, the
		// plugin model function is called to acquire the model.
		// Arguments:
		//   context        - The context of the model.
		//   className      - The class name or base class name of the
		//                    model. This must match the template
		//                    parameter MDL. A plugin that has this name
		//                    in ModelClasses is searched for.
		//   name           - The name of the model.
		//   nameIsFilePath - If true, then the name is a file path
		//                    (which must be absolute!), and then only
		//                    plugins whose FileTypes matches that file
		//                    are considered. Otherwise only className
		//                    is used to choose a plugin.
		//   common         - true for refinding or creating a common
		//                    model, false for creating a private model.
		//   alternative    - If there are multiple matching plugins,
		//                    then the one with the highest priority is
		//                    chosen if this argument is 0. If this
		//                    argument is 1, the one with the
		//                    second-highest priority is chosen, and so
		//                    on.
		//   statMode       - This has to be S_IFREG to acquire the
		//                    model for a regular file, or S_IFDIR for a
		//                    directory. The parameter is ignored if
		//                    nameIsFilePath is false.
		// Returns: The model (never NULL).
		// Throws: An error message on failure, also if no suitable
		//         plugin found.

	emFpPlugin * SearchPlugin(
		const char * modelClassName=NULL, const char * filePath=NULL,
		bool requireAbleToSave=false, int alternative=0,
		long statMode=S_IFREG
	);
		// Search a plugin that fits the given criteria.
		// Arguments:
		//   modelClassName    - If not NULL, then only consider plugins
		//                       whose ModelClasses contains this name.
		//   filePath          - If not NULL, then only consider plugins
		//                       whose FileTypes matches that file.
		//   requireAbleToSave - Whether to consider only plugins where
		//                       ModelAbleToSave is true.
		//   alternative       - If there are multiple matching plugins,
		//                       then the one with the highest priority
		//                       is chosen if this argument is 0. If
		//                       this argument is 1, the one with the
		//                       second-highest priority is chosen, and
		//                       so on.
		//   statMode          - This has to be S_IFREG if filePath
		//                       denotes a regular file, or S_IFDIR for
		//                       a directory. The parameter is ignored
		//                       if filePath is NULL.
		// Returns: A pointer to the plugin or NULL if not found.

	emArray<emFpPlugin*> SearchPlugins(
		const char * modelClassName=NULL, const char * filePath=NULL,
		bool requireAbleToSave=false, long statMode=S_IFREG
	);
		// Same as SearchPlugin, but return all plugins which match
		// the criteria, sorted from highest priority to lowest. The
		// returned array never contains any NULL.

protected:

	emFpPluginList(emContext & context, const emString & name);
	virtual ~emFpPluginList();

private:

	emRef<emModel> TryAcquireModelImpl(
		emContext & context, const char * className,
		const emString & name, bool nameIsFilePath,
		bool common, int alternative, long statMode
	);

	static bool IsMatchingPlugin(
		const emFpPlugin & plugin, const char * modelClassName,
		const char * fileName, int fileNameLen,
		bool requireAbleToSave, long statMode
	);

	static int CmpReversePluginPriorities(
		const emFpPlugin * obj1, const emFpPlugin * obj2,
		void * context
	);

	emOwnPtrArray<emFpPlugin> Plugins;
		// Sorted by descending priority, secondly by file name.
};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

template <class MDL> emRef<MDL> emFpPlugin::TryAcquireModel(
	emContext & context, const char * className,
	const emString & name, bool common
)
{
	emRef<emModel> base=TryAcquireModelImpl(context,className,name,common);
	emRef<MDL> model=dynamic_cast<MDL*>(base.Get());
	if (!model) throw emException(
		"emFpPlugin::TryAcquireModel: dynamic cast failed for %s",
		className
	);
	return model;
}

template <class MDL> emRef<MDL> emFpPluginList::TryAcquireModel(
	emContext & context, const char * className,
	const emString & name, bool nameIsFilePath,
	bool common, int alternative, long statMode
)
{
	emRef<emModel> base=TryAcquireModelImpl(
		context,className,name,nameIsFilePath,common,alternative,statMode
	);
	emRef<MDL> model=dynamic_cast<MDL*>(base.Get());
	if (!model) throw emException(
		"emFpPluginList::TryAcquireModel: dynamic cast failed for %s",
		className
	);
	return model;
}


#endif

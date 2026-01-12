//------------------------------------------------------------------------------
// emFpPlugin.cpp
//
// Copyright (C) 2006-2009,2011,2014,2018-2020,2024-2025 Oliver Hamann.
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

#include <emCore/emFpPlugin.h>
#include <emCore/emInstallInfo.h>
#include <emCore/emErrorPanel.h>


//==============================================================================
//================================= emFpPlugin =================================
//==============================================================================

emFpPlugin::emFpPlugin()
	: emStructRec(),
	FileTypes(this,"FileTypes"),
	FileFormatName(this,"FileFormatName"),
	Priority(this,"Priority",1.0),
	Library(this,"Library","unknown"),
	Function(this,"Function","unknown"),
	ModelFunction(this,"ModelFunction"),
	ModelClasses(this,"ModelClasses"),
	ModelAbleToSave(this,"ModelAbleToSave",false),
	Properties(this,"Properties"),
	CachedFunc(NULL),
	CachedModelFunc(NULL)
{
}


emFpPlugin::~emFpPlugin()
{
}


emFpPlugin::PropertyRec::PropertyRec()
	: emStructRec(),
	Name(this,"Name"),
	Value(this,"Value")
{
}


emFpPlugin::PropertyRec::~PropertyRec()
{
}


emFpPlugin::PropertyRec * emFpPlugin::GetProperty(const char * name)
{
	int i;

	for (i=Properties.GetCount()-1; i>=0; i--) {
		if (Properties[i].Name.Get()==name) return &Properties[i];
	}
	return NULL;
}


emPanel * emFpPlugin::TryCreateFilePanel(
	emPanel::ParentArg parent, const emString & name, const emString & path
)
{
	emString errorBuf;
	emPanel * panel;

	if (CachedLibName!=Library) {
		CachedFunc=NULL;
		CachedModelFunc=NULL;
		CachedLibName=Library;
	}

	if (!CachedFunc || CachedFuncName!=Function) {
		if (Function.Get().IsEmpty()) {
			throw emException("emFpPlugin: Function name is empty");
		}
		CachedFunc=(emFpPluginFunc)emTryResolveSymbol(
			Library.Get(),false,Function.Get()
		);
		CachedFuncName=Function;
	}

	panel=CachedFunc(parent,name,path,this,&errorBuf);
	if (!panel) {
		if (errorBuf.IsEmpty()) {
			throw emException(
				"Plugin function %s in %s failed.",
				Function.Get().Get(),
				Library.Get().Get()
			);
		}
		throw emException("%s",errorBuf.Get());
	}
	return panel;
}


const char * emFpPlugin::GetFormatName() const
{
	return "emFpPlugin";
}


emRef<emModel> emFpPlugin::TryAcquireModelImpl(
	emContext & context, const char * className,
	const emString & name, bool common
)
{
	emString errorBuf;
	emRef<emModel> model;

	if (CachedLibName!=Library) {
		CachedFunc=NULL;
		CachedModelFunc=NULL;
		CachedLibName=Library;
	}

	if (!CachedModelFunc || CachedModelFuncName!=ModelFunction) {
		if (ModelFunction.Get().IsEmpty()) {
			throw emException("emFpPlugin: Model function name is empty");
		}
		CachedModelFunc=(emFpPluginModelFunc)emTryResolveSymbol(
			Library.Get(),false,ModelFunction.Get()
		);
		CachedModelFuncName=ModelFunction;
	}

	if (!CachedModelFunc(context,className,name,common,this,&model,&errorBuf)) {
		if (errorBuf.IsEmpty()) {
			throw emException(
				"Plugin model function %s in %s failed.",
				ModelFunction.Get().Get(),
				Library.Get().Get()
			);
		}
		throw emException("%s",errorBuf.Get());
	}

	if (model==NULL) {
		throw emException(
			"Plugin model function %s in %s returned true but no model.",
			ModelFunction.Get().Get(),
			Library.Get().Get()
		);
	}
	return model;
}


//==============================================================================
//=============================== emFpPluginList ===============================
//==============================================================================

emRef<emFpPluginList> emFpPluginList::Acquire(emRootContext & rootContext)
{
	EM_IMPL_ACQUIRE_COMMON(emFpPluginList,rootContext,"")
}


emPanel * emFpPluginList::CreateFilePanel(
	emPanel::ParentArg parent, const emString & name,
	const emString & path, int alternative
)
{
	emString absPath;
	struct em_stat st;
	int err;

	absPath=emGetAbsolutePath(path);
	err=em_stat(absPath.Get(),&st);
	if (err) err=errno;
	return CreateFilePanel(parent,name,absPath,err,st.st_mode,alternative);
}


emPanel * emFpPluginList::CreateFilePanel(
	emPanel::ParentArg parent, const emString & name,
	const emString & absolutePath, int statErr, long statMode,
	int alternative
)
{
	emFpPlugin * plugin;

	if (statErr) {
		return new emErrorPanel(parent,name,emGetErrorText(statErr));
	}

	plugin=SearchPlugin(NULL,absolutePath,false,alternative,statMode);
	if (!plugin) {
		return new emErrorPanel(
			parent,name,
			alternative<=0 ?
				"This file type cannot be shown."
			:
				"No alternative file panel plugin available."
		);
	}

	try {
		return plugin->TryCreateFilePanel(parent,name,absolutePath);
	}
	catch (const emException & exception) {
		return new emErrorPanel(parent,name,exception.GetText());
	}
}


emFpPlugin * emFpPluginList::SearchPlugin(
	const char * modelClassName, const char * filePath,
	bool requireAbleToSave, int alternative, long statMode
)
{
	const char * fn;
	int i,fnLen;

	if (filePath) {
		fn=emGetNameInPath(filePath);
		fnLen=strlen(fn);
	}
	else {
		fn=NULL;
		fnLen=0;
	}

	for (i=0; i<Plugins.GetCount(); i++) {
		if (
			IsMatchingPlugin(
				*Plugins[i],modelClassName,fn,fnLen,requireAbleToSave,statMode
			)
		) {
			if (alternative<=0) return Plugins[i];
			alternative--;
		}
	}

	return NULL;
}


emArray<emFpPlugin*> emFpPluginList::SearchPlugins(
	const char * modelClassName, const char * filePath,
	bool requireAbleToSave, long statMode
)
{
	emArray<emFpPlugin*> result;
	const char * fn;
	int i,fnLen;

	if (filePath) {
		fn=emGetNameInPath(filePath);
		fnLen=strlen(fn);
	}
	else {
		fn=NULL;
		fnLen=0;
	}

	for (i=0; i<Plugins.GetCount(); i++) {
		if (
			IsMatchingPlugin(
				*Plugins[i],modelClassName,fn,fnLen,requireAbleToSave,statMode
			)
		) {
			result.Add(Plugins[i]);
		}
	}

	return result;
}


emFpPluginList::emFpPluginList(emContext & context, const emString & name)
	: emModel(context,name)
{
	emString dirPath,pluginPath;
	emArray<emString> dirList;
	emFpPlugin * plugin;
	int i;

	SetMinCommonLifetime(UINT_MAX);

	dirPath=emGetConfigDirOverloadable(GetRootContext(),"emCore","FpPlugins");

	try {
		dirList=emTryLoadDir(dirPath);
	}
	catch (const emException & exception) {
		emFatalError("emFpPluginList: %s",exception.GetText().Get());
	}
	dirList.Sort(emStdComparer<emString>::Compare);

	for (i=0; i<dirList.GetCount(); i++) {
		pluginPath=emGetChildPath(dirPath,dirList[i]);
		if (strcmp(emGetExtensionInPath(pluginPath),".emFpPlugin")==0) {
			plugin=new emFpPlugin;
			try {
				plugin->TryLoad(pluginPath);
			}
			catch (const emException & exception) {
				delete plugin;
				emFatalError("emFpPluginList: %s",exception.GetText().Get());
			}
			Plugins.Add(plugin);
		}
	}

	Plugins.Sort(CmpReversePluginPriorities,this);
	Plugins.Compact();
}


emFpPluginList::~emFpPluginList()
{
}


emRef<emModel> emFpPluginList::TryAcquireModelImpl(
	emContext & context, const char * className,
	const emString & name, bool nameIsFilePath,
	bool common, int alternative, long statMode
)
{
	emFpPlugin * plugin;

	plugin=SearchPlugin(
		className,
		nameIsFilePath ? name.Get() : NULL,
		false,
		alternative,
		statMode
	);

	if (!plugin) throw emException("No suitable plugin found");

	return plugin->TryAcquireModel<emModel>(
		context,className,name,common
	);
}


bool emFpPluginList::IsMatchingPlugin(
	const emFpPlugin & plugin, const char * modelClassName,
	const char * fileName, int fileNameLen,
	bool requireAbleToSave, long statMode
)
{
	const char * type;
	int i,n,typeLen;

	if (modelClassName) {
		n=plugin.ModelClasses.GetCount();
		for (i=0; i<n; i++) {
			if (plugin.ModelClasses[i].Get() == modelClassName) break;
		}
		if (i>=n) return false;
	}

	if (fileName) {
		n=plugin.FileTypes.GetCount();
		for (i=0; i<n; i++) {
			type=plugin.FileTypes[i].Get();
			if (type[0]=='.') {
				if ((statMode&S_IFMT)==S_IFREG) {
					typeLen=strlen(type);
					if (
						typeLen<fileNameLen &&
						strcasecmp(fileName+fileNameLen-typeLen,type)==0
					) break;
				}
			}
			else if (strcmp(type,"file")==0) {
				if ((statMode&S_IFMT)==S_IFREG) break;
			}
			else if (strcmp(type,"directory")==0) {
				if ((statMode&S_IFMT)==S_IFDIR) break;
			}
		}
		if (i>=n) return false;
	}

	if (requireAbleToSave && !plugin.ModelAbleToSave) return false;

	return true;
}


int emFpPluginList::CmpReversePluginPriorities(
	const emFpPlugin * obj1, const emFpPlugin * obj2, void * context
)
{
	double d;

	d = obj1->Priority - obj2->Priority;
	if (d<0.0) return 1;
	if (d>0.0) return -1;
	return 0;
}

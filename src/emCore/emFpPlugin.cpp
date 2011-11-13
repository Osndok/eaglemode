//------------------------------------------------------------------------------
// emFpPlugin.cpp
//
// Copyright (C) 2006-2009,2011 Oliver Hamann.
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

#include <emCore/emInstallInfo.h>
#include <emCore/emErrorPanel.h>
#include <emCore/emFpPlugin.h>


//==============================================================================
//================================= emFpPlugin =================================
//==============================================================================

emFpPlugin::emFpPlugin()
	: emStructRec(),
	FileTypes(this,"FileTypes"),
	Priority(this,"Priority",1.0),
	Library(this,"Library","unknown"),
	Function(this,"Function","unknown"),
	Properties(this,"Properties")
{
	CachedFunc=NULL;
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
) throw(emString)
{
	emString errorBuf;
	emPanel * panel;

	if (!CachedFunc || CachedFuncLib!=Library || CachedFuncName!=Function) {
		CachedFunc=emTryResolveSymbol(Library.Get(),false,Function.Get());
		CachedFuncLib=Library;
		CachedFuncName=Function;
	}
	errorBuf.Empty();
	panel=((emFpPluginFunc)CachedFunc)(
		parent,name,path,this,&errorBuf
	);
	if (!panel) {
		if (errorBuf.IsEmpty()) {
			errorBuf=emString::Format(
				"Plugin function %s in %s failed.",
				Function.Get().Get(),
				Library.Get().Get()
			);
		}
		throw errorBuf;
	}
	return panel;
}


const char * emFpPlugin::GetFormatName() const
{
	return "emFpPlugin";
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
	emFpPlugin * plugin, * found;
	const char * fn, * type;
	int i,j,fnLen,typeLen;

	if (statErr) {
		return new emErrorPanel(parent,name,emGetErrorText(statErr));
	}

	found=NULL;
	fn=emGetNameInPath(absolutePath);
	fnLen=strlen(fn);
	for (i=0; i<Plugins.GetCount(); i++) {
		plugin=Plugins[i];
		for (j=0; j<plugin->FileTypes.GetCount(); j++) {
			type=plugin->FileTypes[j].Get();
			if (type[0]=='.') {
				if ((statMode&S_IFMT)==S_IFREG) {
					typeLen=strlen(type);
					if (
						typeLen<fnLen &&
						strcasecmp(fn+fnLen-typeLen,type)==0
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
		if (j<plugin->FileTypes.GetCount()) {
			found=plugin;
			alternative--;
			if (alternative<0) break;
		}
	}

	if (!found) {
		return new emErrorPanel(parent,name,"This file type cannot be shown.");
	}
	else if (alternative>=0) {
		return new emErrorPanel(parent,name,"No alternative file panel plugin available.");
	}
	else {
		try {
			return found->TryCreateFilePanel(parent,name,absolutePath);
		}
		catch (emString errorMessage) {
			return new emErrorPanel(parent,name,errorMessage);
		}
	}
}


emFpPluginList::emFpPluginList(emContext & context, const emString & name)
	: emModel(context,name)
{
	emString dirPath,pluginPath;
	emArray<emString> dirList;
	emFpPlugin * plugin;
	int i;

	SetMinCommonLifetime(UINT_MAX);

	Plugins.SetTuningLevel(4);

	dirPath=emGetConfigDirOverloadable(GetRootContext(),"emCore","FpPlugins");

	try {
		dirList=emTryLoadDir(dirPath);
	}
	catch (emString errorMessage) {
		emFatalError("emFpPluginList: %s",errorMessage.Get());
	}
	dirList.Sort(emStdComparer<emString>::Compare);

	for (i=0; i<dirList.GetCount(); i++) {
		pluginPath=emGetChildPath(dirPath,dirList[i]);
		if (strcmp(emGetExtensionInPath(pluginPath),".emFpPlugin")==0) {
			plugin=new emFpPlugin;
			try {
				plugin->TryLoad(pluginPath);
			}
			catch (emString errorMessage) {
				delete plugin;
				emFatalError("emFpPluginList: %s",errorMessage.Get());
			}
			Plugins.Add(plugin);
		}
	}

	Plugins.Sort(CmpReversePluginPriorities,this);
	Plugins.Compact();
}


emFpPluginList::~emFpPluginList()
{
	int i;

	for (i=Plugins.GetCount()-1; i>=0; i--) delete Plugins[i];
}


int emFpPluginList::CmpReversePluginPriorities(
	emFpPlugin * const * obj1, emFpPlugin * const * obj2, void * context
)
{
	double d;

	d = (*obj1)->Priority - (*obj2)->Priority;
	if (d<0.0) return 1;
	if (d>0.0) return -1;
	return 0;
}

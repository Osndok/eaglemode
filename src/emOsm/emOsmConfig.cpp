//------------------------------------------------------------------------------
// emOsmConfig.cpp
//
// Copyright (C) 2024 Oliver Hamann.
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

#include <emOsm/emOsmConfig.h>
#include <emCore/emInstallInfo.h>
#if defined(_WIN32)
#	include <windows.h>
#	include <shlobj.h>
#endif


emRef<emOsmConfig> emOsmConfig::Acquire(emRootContext & rootContext)
{
	EM_IMPL_ACQUIRE_COMMON(emOsmConfig,rootContext,"")
}


const char * emOsmConfig::GetFormatName() const
{
	return "emOsmConfig";
}


const char * emOsmConfig::TryGetCacheDirectory()
{
	static const struct CacheDir {

		const char * Error;

#if defined(_WIN32)
		char Path[MAX_PATH+1];
#else
		char Path[PATH_MAX+1];
#endif

		CacheDir()
		{
#if defined(_WIN32)
			char buf[MAX_PATH+1];
			HRESULT r;
			LPMALLOC pMalloc;
			LPITEMIDLIST pidl;
			BOOL b;
			int n;

			Error=NULL;
			Path[0]=0;

			r=SHGetMalloc(&pMalloc);
			if (r!=NOERROR) {
				Error="SHGetMalloc failed.";
				return;
			}

			r=SHGetSpecialFolderLocation(NULL,CSIDL_LOCAL_APPDATA,&pidl);
			if (r!=NOERROR) {
				Error="SHGetSpecialFolderLocation failed.";
				return;
			}
			b=SHGetPathFromIDList(pidl,buf);
			if (!b) {
				Error="SHGetPathFromIDList failed.";
				return;
			}
			pMalloc->Free(pidl);

			n=snprintf(Path,sizeof(Path),"%s\\eaglemode\\cache\\emOsm",buf);
			if (n>=(int)sizeof(Path)) {
				Error="Cache path too long.";
			}
#else
			const char * p;
			int n;
			static const char * subPath = "eaglemode/emOsm";

			Error=NULL;
			Path[0]=0;

			p=getenv("XDG_CACHE_HOME");
			if (p && *p) {
				n=snprintf(Path,sizeof(Path),"%s/%s",p,subPath);
			}
			else {
				p=getenv("HOME");
				if (!p || !*p) {
					Error="Environment variable HOME not set.";
					return;
				}
				n=snprintf(Path,sizeof(Path),"%s/.cache/%s",p,subPath);
			}
			if (n>=(int)sizeof(Path)) {
				Error="Cache path too long.";
			}
#endif
		}

	} cacheDir;

	if (cacheDir.Error) throw emException("%s",cacheDir.Error);
	return cacheDir.Path;
}


emOsmConfig::emOsmConfig(emContext & context, const emString & name)
	: emConfigModel(context,name),
	emStructRec(),
	MaxCacheMegabytes(this,"MaxCacheMegabytes",1000,1,INT_MAX),
	MaxCacheAgeDays(this,"MaxCacheAgeDays",7,1,INT_MAX)
{
	PostConstruct(
		*this,
		emGetInstallPath(EM_IDT_USER_CONFIG,"emOsm","config.rec")
	);
	SetMinCommonLifetime(20);
	SetAutoSaveDelaySeconds(10);
	LoadOrInstall();
}


emOsmConfig::~emOsmConfig()
{
	Save();
}

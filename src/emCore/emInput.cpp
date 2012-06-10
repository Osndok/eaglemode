//------------------------------------------------------------------------------
// emInput.cpp
//
// Copyright (C) 2005-2012 Oliver Hamann.
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

#include <emCore/emInput.h>
#include <emCore/emThread.h>


//==============================================================================
//================================= emInputKey =================================
//==============================================================================

struct emInputKeyName {
	emInputKey Key;
	const char * Name;
};

static const emInputKeyName emInputKeyNames[] = {
	{ EM_KEY_NONE         , "None"           },
	{ EM_KEY_LEFT_BUTTON  , "Left-Button"    },
	{ EM_KEY_MIDDLE_BUTTON, "Middle-Button"  },
	{ EM_KEY_RIGHT_BUTTON , "Right-Button"   },
	{ EM_KEY_WHEEL_UP     , "Wheel-Up"       },
	{ EM_KEY_WHEEL_DOWN   , "Wheel-Down"     },
	{ EM_KEY_TOUCH        , "Touch"          },
	{ EM_KEY_SHIFT        , "Shift"          },
	{ EM_KEY_CTRL         , "Ctrl"           },
	{ EM_KEY_ALT          , "Alt"            },
	{ EM_KEY_META         , "Meta"           },
	{ EM_KEY_ALT_GR       , "Alt-Gr"         },
	{ EM_KEY_CURSOR_UP    , "Cursor-Up"      },
	{ EM_KEY_CURSOR_DOWN  , "Cursor-Down"    },
	{ EM_KEY_CURSOR_LEFT  , "Cursor-Left"    },
	{ EM_KEY_CURSOR_RIGHT , "Cursor-Right"   },
	{ EM_KEY_PAGE_UP      , "Page-Up"        },
	{ EM_KEY_PAGE_DOWN    , "Page-Down"      },
	{ EM_KEY_HOME         , "Home"           },
	{ EM_KEY_END          , "End"            },
	{ EM_KEY_PRINT        , "Print"          },
	{ EM_KEY_PAUSE        , "Pause"          },
	{ EM_KEY_MENU         , "Menu"           },
	{ EM_KEY_INSERT       , "Insert"         },
	{ EM_KEY_DELETE       , "Delete"         },
	{ EM_KEY_BACKSPACE    , "Backspace"      },
	{ EM_KEY_TAB          , "Tab"            },
	{ EM_KEY_ENTER        , "Enter"          },
	{ EM_KEY_ESCAPE       , "Escape"         },
	{ EM_KEY_SPACE        , "Space"          },
	{ EM_KEY_0            , "0"              },
	{ EM_KEY_1            , "1"              },
	{ EM_KEY_2            , "2"              },
	{ EM_KEY_3            , "3"              },
	{ EM_KEY_4            , "4"              },
	{ EM_KEY_5            , "5"              },
	{ EM_KEY_6            , "6"              },
	{ EM_KEY_7            , "7"              },
	{ EM_KEY_8            , "8"              },
	{ EM_KEY_9            , "9"              },
	{ EM_KEY_A            , "A"              },
	{ EM_KEY_B            , "B"              },
	{ EM_KEY_C            , "C"              },
	{ EM_KEY_D            , "D"              },
	{ EM_KEY_E            , "E"              },
	{ EM_KEY_F            , "F"              },
	{ EM_KEY_G            , "G"              },
	{ EM_KEY_H            , "H"              },
	{ EM_KEY_I            , "I"              },
	{ EM_KEY_J            , "J"              },
	{ EM_KEY_K            , "K"              },
	{ EM_KEY_L            , "L"              },
	{ EM_KEY_M            , "M"              },
	{ EM_KEY_N            , "N"              },
	{ EM_KEY_O            , "O"              },
	{ EM_KEY_P            , "P"              },
	{ EM_KEY_Q            , "Q"              },
	{ EM_KEY_R            , "R"              },
	{ EM_KEY_S            , "S"              },
	{ EM_KEY_T            , "T"              },
	{ EM_KEY_U            , "U"              },
	{ EM_KEY_V            , "V"              },
	{ EM_KEY_W            , "W"              },
	{ EM_KEY_X            , "X"              },
	{ EM_KEY_Y            , "Y"              },
	{ EM_KEY_Z            , "Z"              },
	{ EM_KEY_F1           , "F1"             },
	{ EM_KEY_F2           , "F2"             },
	{ EM_KEY_F3           , "F3"             },
	{ EM_KEY_F4           , "F4"             },
	{ EM_KEY_F5           , "F5"             },
	{ EM_KEY_F6           , "F6"             },
	{ EM_KEY_F7           , "F7"             },
	{ EM_KEY_F8           , "F8"             },
	{ EM_KEY_F9           , "F9"             },
	{ EM_KEY_F10          , "F10"            },
	{ EM_KEY_F11          , "F11"            },
	{ EM_KEY_F12          , "F12"            }
};


const char * emInputKeyToString(emInputKey key)
{
	static emThreadInitMutex initMutex;
	static const char * table[256];
	int i;

	if (initMutex.Begin()) {
		for (i=0; i<256; i++) table[i]=NULL;
		for (i=0; i<(int)(sizeof(emInputKeyNames)/sizeof(emInputKeyName)); i++) {
			if ((emInputKeyNames[i].Key&~255)==0) {
				table[emInputKeyNames[i].Key]=emInputKeyNames[i].Name;
			}
		}
		initMutex.End();
	}
	if ((key&~255)!=0) return NULL;
	return table[key];
}


static int emCompareInputKeyNameToName(
	const emInputKeyName * obj, const char * key, void * context
)
{
	return strcasecmp(obj->Name,key);
}


static int emCompareInputKeyNames(
	const emInputKeyName * obj1, const emInputKeyName * obj2, void * context
)
{
	return strcasecmp(obj1->Name,obj2->Name);
}


emInputKey emStringToInputKey(const char * str)
{
	static emThreadInitMutex initMutex;
	static emInputKeyName table[sizeof(emInputKeyNames)/sizeof(emInputKeyName)];
	int i;

	if (initMutex.Begin()) {
		memcpy(table,emInputKeyNames,sizeof(emInputKeyNames));
		emSortArray(
			table,
			(int)(sizeof(emInputKeyNames)/sizeof(emInputKeyName)),
			emCompareInputKeyNames,
			(void*)NULL
		);
		initMutex.End();
	}

	i=emBinarySearch(
		table,
		(int)(sizeof(emInputKeyNames)/sizeof(emInputKeyName)),
		str,
		emCompareInputKeyNameToName,
		(void*)NULL
	);
	return i>=0 ? table[i].Key : EM_KEY_NONE;
}


//==============================================================================
//================================ emInputEvent ================================
//==============================================================================

emInputEvent::emInputEvent()
{
	Key=EM_KEY_NONE;
	Repeat=0;
	Variant=0;
}


void emInputEvent::Setup(
	emInputKey key, const emString & chars, int repeat, int variant
)
{
	Key=key;
	Chars=chars;
	Repeat=repeat;
	Variant=variant;
}


void emInputEvent::Eat()
{
	Key=EM_KEY_NONE;
	Chars.Empty();
	Repeat=0;
	Variant=0;
}


bool emInputEvent::IsKeyboardEvent() const
{
	return !IsEmpty() && !IsMouseEvent() && !IsTouchEvent();
}


//==============================================================================
//=============================== emInputState ================================
//==============================================================================

emInputState::emInputState()
{
	MouseX=0.0;
	MouseY=0.0;
	memset(KeyStates,0,sizeof(KeyStates));
	Touches.SetTuningLevel(4);
}


emInputState::emInputState(const emInputState & inputState)
{
	MouseX=inputState.MouseX;
	MouseY=inputState.MouseY;
	memcpy(KeyStates,inputState.KeyStates,sizeof(KeyStates));
	Touches=inputState.Touches;
}


emInputState & emInputState::operator = (const emInputState & inputState)
{
	MouseX=inputState.MouseX;
	MouseY=inputState.MouseY;
	memcpy(KeyStates,inputState.KeyStates,sizeof(KeyStates));
	Touches=inputState.Touches;
	return *this;
}


bool emInputState::operator == (const emInputState & inputState) const
{
	int i;

	if (MouseX!=inputState.MouseX) return false;
	if (MouseY!=inputState.MouseY) return false;
	if (Touches.GetCount()!=inputState.Touches.GetCount()) return false;
	for (i=Touches.GetCount()-1; i>=0; i--) {
		if (Touches[i].Id!=inputState.Touches[i].Id) return false;
		if (Touches[i].X!=inputState.Touches[i].X) return false;
		if (Touches[i].Y!=inputState.Touches[i].Y) return false;
	}
	if (memcmp(KeyStates,inputState.KeyStates,sizeof(KeyStates))!=0) return false;
	return true;
}


bool emInputState::operator != (const emInputState & inputState) const
{
	return !(*this == inputState);
}


int emInputState::SearchTouch(emUInt64 id) const
{
	int i;

	for (i=Touches.GetCount()-1; i>=0; i--) {
		if (Touches[i].Id==id) break;
	}
	return i;
}


void emInputState::AddTouch(emUInt64 id, double x, double y)
{
	Touches.AddNew();
	SetTouch(Touches.GetCount()-1,id,x,y);
}


void emInputState::SetTouch(int index, emUInt64 id, double x, double y)
{
	Touch * t;

	t=&Touches.GetWritable(index);
	t->Id=id;
	t->X=x;
	t->Y=y;
}


void emInputState::RemoveTouch(int index)
{
	Touches.Remove(index);
}


void emInputState::ClearTouches()
{
	Touches.Empty();
}


bool emInputState::Get(emInputKey key) const
{
	unsigned k;

	k=(unsigned)key;
	return k<sizeof(KeyStates)*8 && (KeyStates[k>>3]&(1<<(k&7)))!=0;
}


void emInputState::Set(emInputKey key, bool pressed)
{
	unsigned k;

	k=(unsigned)key;
	if (k<sizeof(KeyStates)*8) {
		if (pressed) {
			KeyStates[k>>3]|=(unsigned char)(1<<(k&7));
		}
		else {
			KeyStates[k>>3]&=(unsigned char)(~(1<<(k&7)));
		}
	}
}


bool emInputState::IsNoMod() const
{
	return
		(KeyStates[EM_KEY_SHIFT>>3]&(1<<(EM_KEY_SHIFT&7)))==0 &&
		(KeyStates[EM_KEY_CTRL >>3]&(1<<(EM_KEY_CTRL &7)))==0 &&
		(KeyStates[EM_KEY_ALT  >>3]&(1<<(EM_KEY_ALT  &7)))==0 &&
		(KeyStates[EM_KEY_META >>3]&(1<<(EM_KEY_META &7)))==0
	;
}


bool emInputState::IsShiftMod() const
{
	return
		(KeyStates[EM_KEY_SHIFT>>3]&(1<<(EM_KEY_SHIFT&7)))!=0 &&
		(KeyStates[EM_KEY_CTRL >>3]&(1<<(EM_KEY_CTRL &7)))==0 &&
		(KeyStates[EM_KEY_ALT  >>3]&(1<<(EM_KEY_ALT  &7)))==0 &&
		(KeyStates[EM_KEY_META >>3]&(1<<(EM_KEY_META &7)))==0
	;
}


bool emInputState::IsShiftCtrlMod() const
{
	return
		(KeyStates[EM_KEY_SHIFT>>3]&(1<<(EM_KEY_SHIFT&7)))!=0 &&
		(KeyStates[EM_KEY_CTRL >>3]&(1<<(EM_KEY_CTRL &7)))!=0 &&
		(KeyStates[EM_KEY_ALT  >>3]&(1<<(EM_KEY_ALT  &7)))==0 &&
		(KeyStates[EM_KEY_META >>3]&(1<<(EM_KEY_META &7)))==0
	;
}


bool emInputState::IsShiftCtrlAltMod() const
{
	return
		(KeyStates[EM_KEY_SHIFT>>3]&(1<<(EM_KEY_SHIFT&7)))!=0 &&
		(KeyStates[EM_KEY_CTRL >>3]&(1<<(EM_KEY_CTRL &7)))!=0 &&
		(KeyStates[EM_KEY_ALT  >>3]&(1<<(EM_KEY_ALT  &7)))!=0 &&
		(KeyStates[EM_KEY_META >>3]&(1<<(EM_KEY_META &7)))==0
	;
}


bool emInputState::IsShiftCtrlAltMetaMod() const
{
	return
		(KeyStates[EM_KEY_SHIFT>>3]&(1<<(EM_KEY_SHIFT&7)))!=0 &&
		(KeyStates[EM_KEY_CTRL >>3]&(1<<(EM_KEY_CTRL &7)))!=0 &&
		(KeyStates[EM_KEY_ALT  >>3]&(1<<(EM_KEY_ALT  &7)))!=0 &&
		(KeyStates[EM_KEY_META >>3]&(1<<(EM_KEY_META &7)))!=0
	;
}


bool emInputState::IsShiftCtrlMetaMod() const
{
	return
		(KeyStates[EM_KEY_SHIFT>>3]&(1<<(EM_KEY_SHIFT&7)))!=0 &&
		(KeyStates[EM_KEY_CTRL >>3]&(1<<(EM_KEY_CTRL &7)))!=0 &&
		(KeyStates[EM_KEY_ALT  >>3]&(1<<(EM_KEY_ALT  &7)))==0 &&
		(KeyStates[EM_KEY_META >>3]&(1<<(EM_KEY_META &7)))!=0
	;
}


bool emInputState::IsShiftAltMod() const
{
	return
		(KeyStates[EM_KEY_SHIFT>>3]&(1<<(EM_KEY_SHIFT&7)))!=0 &&
		(KeyStates[EM_KEY_CTRL >>3]&(1<<(EM_KEY_CTRL &7)))==0 &&
		(KeyStates[EM_KEY_ALT  >>3]&(1<<(EM_KEY_ALT  &7)))!=0 &&
		(KeyStates[EM_KEY_META >>3]&(1<<(EM_KEY_META &7)))==0
	;
}


bool emInputState::IsShiftAltMetaMod() const
{
	return
		(KeyStates[EM_KEY_SHIFT>>3]&(1<<(EM_KEY_SHIFT&7)))!=0 &&
		(KeyStates[EM_KEY_CTRL >>3]&(1<<(EM_KEY_CTRL &7)))==0 &&
		(KeyStates[EM_KEY_ALT  >>3]&(1<<(EM_KEY_ALT  &7)))!=0 &&
		(KeyStates[EM_KEY_META >>3]&(1<<(EM_KEY_META &7)))!=0
	;
}


bool emInputState::IsShiftMetaMod() const
{
	return
		(KeyStates[EM_KEY_SHIFT>>3]&(1<<(EM_KEY_SHIFT&7)))!=0 &&
		(KeyStates[EM_KEY_CTRL >>3]&(1<<(EM_KEY_CTRL &7)))==0 &&
		(KeyStates[EM_KEY_ALT  >>3]&(1<<(EM_KEY_ALT  &7)))==0 &&
		(KeyStates[EM_KEY_META >>3]&(1<<(EM_KEY_META &7)))!=0
	;
}


bool emInputState::IsCtrlMod() const
{
	return
		(KeyStates[EM_KEY_SHIFT>>3]&(1<<(EM_KEY_SHIFT&7)))==0 &&
		(KeyStates[EM_KEY_CTRL >>3]&(1<<(EM_KEY_CTRL &7)))!=0 &&
		(KeyStates[EM_KEY_ALT  >>3]&(1<<(EM_KEY_ALT  &7)))==0 &&
		(KeyStates[EM_KEY_META >>3]&(1<<(EM_KEY_META &7)))==0
	;
}


bool emInputState::IsCtrlAltMod() const
{
	return
		(KeyStates[EM_KEY_SHIFT>>3]&(1<<(EM_KEY_SHIFT&7)))==0 &&
		(KeyStates[EM_KEY_CTRL >>3]&(1<<(EM_KEY_CTRL &7)))!=0 &&
		(KeyStates[EM_KEY_ALT  >>3]&(1<<(EM_KEY_ALT  &7)))!=0 &&
		(KeyStates[EM_KEY_META >>3]&(1<<(EM_KEY_META &7)))==0
	;
}


bool emInputState::IsCtrlAltMetaMod() const
{
	return
		(KeyStates[EM_KEY_SHIFT>>3]&(1<<(EM_KEY_SHIFT&7)))==0 &&
		(KeyStates[EM_KEY_CTRL >>3]&(1<<(EM_KEY_CTRL &7)))!=0 &&
		(KeyStates[EM_KEY_ALT  >>3]&(1<<(EM_KEY_ALT  &7)))!=0 &&
		(KeyStates[EM_KEY_META >>3]&(1<<(EM_KEY_META &7)))!=0
	;
}


bool emInputState::IsCtrlMetaMod() const
{
	return
		(KeyStates[EM_KEY_SHIFT>>3]&(1<<(EM_KEY_SHIFT&7)))==0 &&
		(KeyStates[EM_KEY_CTRL >>3]&(1<<(EM_KEY_CTRL &7)))!=0 &&
		(KeyStates[EM_KEY_ALT  >>3]&(1<<(EM_KEY_ALT  &7)))==0 &&
		(KeyStates[EM_KEY_META >>3]&(1<<(EM_KEY_META &7)))!=0
	;
}


bool emInputState::IsAltMod() const
{
	return
		(KeyStates[EM_KEY_SHIFT>>3]&(1<<(EM_KEY_SHIFT&7)))==0 &&
		(KeyStates[EM_KEY_CTRL >>3]&(1<<(EM_KEY_CTRL &7)))==0 &&
		(KeyStates[EM_KEY_ALT  >>3]&(1<<(EM_KEY_ALT  &7)))!=0 &&
		(KeyStates[EM_KEY_META >>3]&(1<<(EM_KEY_META &7)))==0
	;
}


bool emInputState::IsAltMetaMod() const
{
	return
		(KeyStates[EM_KEY_SHIFT>>3]&(1<<(EM_KEY_SHIFT&7)))==0 &&
		(KeyStates[EM_KEY_CTRL >>3]&(1<<(EM_KEY_CTRL &7)))==0 &&
		(KeyStates[EM_KEY_ALT  >>3]&(1<<(EM_KEY_ALT  &7)))!=0 &&
		(KeyStates[EM_KEY_META >>3]&(1<<(EM_KEY_META &7)))!=0
	;
}


bool emInputState::IsMetaMod() const
{
	return
		(KeyStates[EM_KEY_SHIFT>>3]&(1<<(EM_KEY_SHIFT&7)))==0 &&
		(KeyStates[EM_KEY_CTRL >>3]&(1<<(EM_KEY_CTRL &7)))==0 &&
		(KeyStates[EM_KEY_ALT  >>3]&(1<<(EM_KEY_ALT  &7)))==0 &&
		(KeyStates[EM_KEY_META >>3]&(1<<(EM_KEY_META &7)))!=0
	;
}


bool emInputState::ClearKeyStates()
{
	bool result;
	int i;

	result=false;
	for (i=0; i<(int)sizeof(KeyStates); i++) {
		if (KeyStates[i]) {
			KeyStates[i]=0;
			result=true;
		}
	}
	return result;
}


//==============================================================================
//=============================== emInputHotkey ================================
//==============================================================================

emInputHotkey::emInputHotkey(emInputKey key)
{
	ClearModifiers();
	SetKey(key);
}


emInputHotkey::emInputHotkey(emInputKey modifier, emInputKey key)
{
	ClearModifiers();
	AddModifier(modifier);
	SetKey(key);
}


emInputHotkey::emInputHotkey(
	emInputKey modifier1, emInputKey modifier2, emInputKey key
)
{
	ClearModifiers();
	AddModifier(modifier1);
	AddModifier(modifier2);
	SetKey(key);
}


emInputHotkey::emInputHotkey(
	emInputKey modifier1, emInputKey modifier2, emInputKey modifier3,
	emInputKey key
)
{
	ClearModifiers();
	AddModifier(modifier1);
	AddModifier(modifier2);
	AddModifier(modifier3);
	SetKey(key);
}


emInputHotkey::emInputHotkey(
	emInputKey modifier1, emInputKey modifier2, emInputKey modifier3,
	emInputKey modifier4, emInputKey key
)
{
	ClearModifiers();
	AddModifier(modifier1);
	AddModifier(modifier2);
	AddModifier(modifier3);
	AddModifier(modifier4);
	SetKey(key);
}


emInputHotkey::emInputHotkey(
	const emInputEvent & event, const emInputState & state
)
{
	Data.MFlags=0;
	if (state.GetShift()) Data.MFlags|=MF_SHIFT;
	if (state.GetCtrl() ) Data.MFlags|=MF_CTRL;
	if (state.GetAlt()  ) Data.MFlags|=MF_ALT;
	if (state.GetMeta() ) Data.MFlags|=MF_META;
	SetKey(event.GetKey());
}


void emInputHotkey::TryParse(const char * str) throw(emString)
{
	char tmp[256];
	const char * p;
	emInputKey key;
	int i,l;

	ClearModifiers();
	for (i=0;;) {
		p=strchr(str+i,'+');
		if (!p) break;
		l=p-str-i;
		if (l<=0) break;
		if (l>(int)sizeof(tmp)-1) goto L_ERROR;
		memcpy(tmp,str+i,l);
		tmp[l]=0;
		key=emStringToInputKey(tmp);
		if (!emIsModifierInputKey(key)) goto L_ERROR;
		AddModifier(key);
		i+=l+1;
	}
	key=emStringToInputKey(str+i);
	if (!emIsKeyboardInputKey(key) || emIsModifierInputKey(key)) goto L_ERROR;
	SetKey(key);
	return;

L_ERROR:
	Packed=0;
	throw emString::Format("Not a valid hotkey: %s",str);
}


emString emInputHotkey::GetString() const
{
	char buf[256];

	GetString(buf,sizeof(buf));
	return emString(buf);
}


void emInputHotkey::GetString(char * buf, int bufSize) const
{
	const char * s[9];
	int i,n,l;

	if (!buf || bufSize<1) return;
	if (Data.Key!=(emByte)EM_KEY_NONE) {
		n=0;
		if (GetShift()) {
			s[n++]=emInputKeyToString(EM_KEY_SHIFT);
			s[n++]="+";
		}
		if (GetCtrl()) {
			s[n++]=emInputKeyToString(EM_KEY_CTRL);
			s[n++]="+";
		}
		if (GetAlt()) {
			s[n++]=emInputKeyToString(EM_KEY_ALT);
			s[n++]="+";
		}
		if (GetMeta()) {
			s[n++]=emInputKeyToString(EM_KEY_META);
			s[n++]="+";
		}
		s[n++]=emInputKeyToString((emInputKey)Data.Key);
		for (i=0; i<n; i++) {
			if (s[i]) {
				l=strlen(s[i]);
				if (l>bufSize-1) l=bufSize-1;
				if (l>0) {
					memcpy(buf,s[i],l);
					buf+=l;
					bufSize-=l;
				}
			}
		}
	}
	*buf=0;
}


bool emInputHotkey::Match(
	const emInputEvent & event, const emInputState & state
) const
{
	return
		event.GetKey()   == Data.Key &&
		state.GetShift() == ((Data.MFlags&MF_SHIFT)!=0) &&
		state.GetCtrl()  == ((Data.MFlags&MF_CTRL )!=0) &&
		state.GetAlt()   == ((Data.MFlags&MF_ALT  )!=0) &&
		state.GetMeta()  == ((Data.MFlags&MF_META )!=0)
	;
}


void emInputHotkey::AddModifier(emInputKey modifier)
{
	switch (modifier) {
	case EM_KEY_SHIFT:
		Data.MFlags|=MF_SHIFT;
		break;
	case EM_KEY_CTRL:
		Data.MFlags|=MF_CTRL;
		break;
	case EM_KEY_ALT:
		Data.MFlags|=MF_ALT;
		break;
	case EM_KEY_META:
		Data.MFlags|=MF_META;
		break;
	default:
		break;
	}
}


void emInputHotkey::SetKey(emInputKey key)
{
	if (!emIsKeyboardInputKey(key) || emIsModifierInputKey(key)) {
		key=EM_KEY_NONE;
	}
	Data.Key=(emByte)key;
}

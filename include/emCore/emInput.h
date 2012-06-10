//------------------------------------------------------------------------------
// emInput.h
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

#ifndef emInput_h
#define emInput_h

#ifndef emStd2_h
#include <emCore/emStd2.h>
#endif

#ifndef emArray_h
#include <emCore/emArray.h>
#endif


//==============================================================================
//================================= emInputKey =================================
//==============================================================================

enum emInputKey {

	// None:
	EM_KEY_NONE          = 0x00,

	// Mouse buttons and wheel:
	EM_KEY_LEFT_BUTTON   = 0xF0,
	EM_KEY_MIDDLE_BUTTON = 0xF1,
	EM_KEY_RIGHT_BUTTON  = 0xF2,
	EM_KEY_WHEEL_UP      = 0xF3,
	EM_KEY_WHEEL_DOWN    = 0xF4,

	// Screen touch:
	EM_KEY_TOUCH         = 0xEF,

	// Keyboard keys:
	EM_KEY_SHIFT         = 0x81,
	EM_KEY_CTRL          = 0x82,
	EM_KEY_ALT           = 0x83,
	EM_KEY_META          = 0x84,
	EM_KEY_ALT_GR        = 0x85,
	EM_KEY_CURSOR_UP     = 0x91,
	EM_KEY_CURSOR_DOWN   = 0x92,
	EM_KEY_CURSOR_LEFT   = 0x93,
	EM_KEY_CURSOR_RIGHT  = 0x94,
	EM_KEY_PAGE_UP       = 0x95,
	EM_KEY_PAGE_DOWN     = 0x96,
	EM_KEY_HOME          = 0x97,
	EM_KEY_END           = 0x98,
	EM_KEY_PRINT         = 0x99,
	EM_KEY_PAUSE         = 0x9A,
	EM_KEY_MENU          = 0x9B,
	EM_KEY_INSERT        = 0x9F,
	EM_KEY_DELETE        = 0x7F,
	EM_KEY_BACKSPACE     = 0x08,
	EM_KEY_TAB           = 0x09,
	EM_KEY_ENTER         = 0x0D,
	EM_KEY_ESCAPE        = 0x1B,
	EM_KEY_SPACE         = 0x20,
	EM_KEY_0             = 0x30,
	EM_KEY_1             = 0x31,
	EM_KEY_2             = 0x32,
	EM_KEY_3             = 0x33,
	EM_KEY_4             = 0x34,
	EM_KEY_5             = 0x35,
	EM_KEY_6             = 0x36,
	EM_KEY_7             = 0x37,
	EM_KEY_8             = 0x38,
	EM_KEY_9             = 0x39,
	EM_KEY_A             = 0x41,
	EM_KEY_B             = 0x42,
	EM_KEY_C             = 0x43,
	EM_KEY_D             = 0x44,
	EM_KEY_E             = 0x45,
	EM_KEY_F             = 0x46,
	EM_KEY_G             = 0x47,
	EM_KEY_H             = 0x48,
	EM_KEY_I             = 0x49,
	EM_KEY_J             = 0x4A,
	EM_KEY_K             = 0x4B,
	EM_KEY_L             = 0x4C,
	EM_KEY_M             = 0x4D,
	EM_KEY_N             = 0x4E,
	EM_KEY_O             = 0x4F,
	EM_KEY_P             = 0x50,
	EM_KEY_Q             = 0x51,
	EM_KEY_R             = 0x52,
	EM_KEY_S             = 0x53,
	EM_KEY_T             = 0x54,
	EM_KEY_U             = 0x55,
	EM_KEY_V             = 0x56,
	EM_KEY_W             = 0x57,
	EM_KEY_X             = 0x58,
	EM_KEY_Y             = 0x59,
	EM_KEY_Z             = 0x5A,
	EM_KEY_F1            = 0xA1,
	EM_KEY_F2            = 0xA2,
	EM_KEY_F3            = 0xA3,
	EM_KEY_F4            = 0xA4,
	EM_KEY_F5            = 0xA5,
	EM_KEY_F6            = 0xA6,
	EM_KEY_F7            = 0xA7,
	EM_KEY_F8            = 0xA8,
	EM_KEY_F9            = 0xA9,
	EM_KEY_F10           = 0xAA,
	EM_KEY_F11           = 0xAB,
	EM_KEY_F12           = 0xAC
};


bool emIsMouseInputKey(emInputKey key);
	// True for mouse buttons and mouse wheel.

bool emIsTouchInputKey(emInputKey key);
	// True screen touch.

bool emIsKeyboardInputKey(emInputKey key);
	// True for all keyboard keys, including modifiers.

bool emIsModifierInputKey(emInputKey key);
	// True for shift, ctrl, alt and meta.

const char * emInputKeyToString(emInputKey key);
emInputKey emStringToInputKey(const char * str);
	// Convert an input key to and from string representation.


//==============================================================================
//================================ emInputEvent ================================
//==============================================================================

class emInputEvent : public emUncopyable {

public:

	// Class for an input event. Such an event consists of an input key
	// (mouse buttons and wheel are even "keys" here), which has changed
	// from non-pressed to pressed state. Instead of a key, or in addition
	// to the key, a translation into text characters may be provided. It is
	// possible that an original event comes through two input events: one
	// with the input key, and one with the translated text characters (at
	// the time of writing this, it is so with emWndsWindowPort, but not
	// with emX11WindowPort).

	emInputEvent();
		// Construct an event which is initially empty.

	void Setup(emInputKey key, const emString & chars, int repeat,
	           int variant);
		// Set-up the event.
		// Arguments:
		//   key     - see GetKey().
		//   chars   - see GetChars().
		//   repeat  - see GetRepeat().
		//   variant - see GetVariant().

	emInputKey GetKey() const;
		// Get the key. This is EM_KEY_NONE if the event is empty or if
		// the event provides characters only (see GetChars()).

	const emString & GetChars() const;
		// Get the translated text characters. This should be at most
		// one text character, but possibly encoded as a multi-byte
		// sequence (e.g. UTF-8). This is an empty string if the event
		// is empty, or if the key could not be translated.

	int GetRepeat() const;
		// Get number of event repetitions. For example, 1 means
		// double-click for mouse buttons.

	int GetVariant() const;
		// Get variant of the event source. If the key is on the numeric
		// key pad, or if it is the right shift, right ctrl or right alt
		// key, then this should be 1. Otherwise this should be 0.

	void Eat();
		// Eat this event (make it an empty event).

	bool IsEmpty() const;
	bool IsMouseEvent() const;
	bool IsTouchEvent() const;
	bool IsKeyboardEvent() const;
		// Ask for the type of event. One of these returns true.

	bool IsKey(emInputKey key) const;
		// Like GetKey()==key.

	bool IsLeftButton() const;
	bool IsMiddleButton() const;
	bool IsRightButton() const;
		// Like GetKey()==EM_KEY_LEFT_BUTTON and so on.

private:
	emInputKey Key;
	emString Chars;
	int Repeat,Variant;
};


//==============================================================================
//================================ emInputState ================================
//==============================================================================

class emInputState {

public:

	// Class for the state of keyboard and mouse. The state consists of the
	// mouse pointer position, and which keys are pressed. (Mouse buttons
	// are even keys)

	emInputState();
		// Construct without having any key pressed.

	emInputState(const emInputState & inputState);
		// Construct a copy of an input state.

	emInputState & operator = (const emInputState & inputState);
		// Copy another input state to this input state.

	bool operator == (const emInputState & inputState) const;
	bool operator != (const emInputState & inputState) const;
		// Comparison operators.

	double GetMouseX() const;
	double GetMouseY() const;
	void SetMouse(double mouseX, double mouseY);
		// Get or set the mouse position in pixel coordinates of the
		// view.

	int GetTouchCount() const;
		// Get number of touches.

	int SearchTouch(emUInt64 id) const;
		// Search a touch by id. Returns the index or -1 if not found.

	emUInt64 GetTouchId(int index) const;
		// Get the id of a touch. A touch may get another index over the
		// time, but it should keep its id.

	double GetTouchX(int index) const;
	double GetTouchY(int index) const;
		// Get position of a touch in pixel coordinates of the view.

	void AddTouch(emUInt64 id, double x, double y);
		// Add a touch.

	void SetTouch(int index, emUInt64 id, double x, double y);
		// Modify an existing touch.

	void RemoveTouch(int index);
		// Remove a touch.

	void ClearTouches();
		// Remove all touches.

	bool Get(emInputKey key) const;
	void Set(emInputKey key, bool pressed);
		// Get or set the state of a particular key. It is true if the
		// key or button is pressed, false otherwise. EM_KEY_WHEEL_UP
		// and EM_KEY_WHEEL_DOWN should never be set here.

	bool GetLeftButton() const;
	bool GetMiddleButton() const;
	bool GetRightButton() const;
	bool GetShift() const;
	bool GetCtrl() const;
	bool GetAlt() const;
	bool GetMeta() const;
		// These are just some abbreviations. It's like
		// Get(EM_KEY_LEFT_BUTTON) and so on.

	bool IsNoMod() const;
	bool IsShiftMod() const;
	bool IsShiftCtrlMod() const;
	bool IsShiftCtrlAltMod() const;
	bool IsShiftCtrlAltMetaMod() const;
	bool IsShiftCtrlMetaMod() const;
	bool IsShiftAltMod() const;
	bool IsShiftAltMetaMod() const;
	bool IsShiftMetaMod() const;
	bool IsCtrlMod() const;
	bool IsCtrlAltMod() const;
	bool IsCtrlAltMetaMod() const;
	bool IsCtrlMetaMod() const;
	bool IsAltMod() const;
	bool IsAltMetaMod() const;
	bool IsMetaMod() const;
		// Test the four modifier keys at once. For example,
		// IsShiftMetaMod() is like (GetShift() && !GetCtrl() &&
		// !GetAlt() && GetMeta()).

	bool ClearKeyStates();
		// Set all keys to non-pressed state. Returns true if something
		// was pressed.

	const unsigned char * GetKeyStates() const;
	unsigned char * GetKeyStates();
		// Get a pointer to the states of all keyboard keys and mouse
		// buttons, even for modification. The size of the returned
		// array is 32 bytes. A key is pressed if
		// GetKeyStates()[key>>3]&(1<<(key&7)) is non-zero.

private:

	struct Touch {
		emUInt64 Id;
		double X, Y;
	};

	double MouseX, MouseY;
	unsigned char KeyStates[32];
	emArray<Touch> Touches;
};


//==============================================================================
//=============================== emInputHotkey ================================
//==============================================================================

class emInputHotkey {

public:

	// Class for a hotkey. It is a combination of modifier keys and a
	// non-modifier keyboard key.

	emInputHotkey();
		// Construct an invalid hotkey.

	emInputHotkey(const emInputHotkey & hotkey);
		// Copy constructor.

	emInputHotkey(emInputKey key);
	emInputHotkey(emInputKey modifier, emInputKey key);
	emInputHotkey(emInputKey modifier1, emInputKey modifier2,
	              emInputKey key);
	emInputHotkey(emInputKey modifier1, emInputKey modifier2,
	              emInputKey modifier3, emInputKey key);
	emInputHotkey(emInputKey modifier1, emInputKey modifier2,
	              emInputKey modifier3, emInputKey modifier4,
	              emInputKey key);
		// Construct a valid hotkey. The last argument must be a
		// non-modifier keyboard key, the other arguments must be
		// modifier keys.

	emInputHotkey(const emInputEvent & event, const emInputState & state);
		// Construct form an input event and an input state. The hotkey
		// gets invalid if the event is not a non-modifier keyboard key.

	emInputHotkey & operator = (const emInputHotkey & hotkey);
		// Copy operator.

	bool operator == (const emInputHotkey & hotkey) const;
	bool operator != (const emInputHotkey & hotkey) const;
		// Comparison operators.

	void TryParse(const char * str) throw(emString);
		// Try to set this hotkey from a human-readable string
		// representation (e.g. "Ctrl+C"). On failure, the hotkey is set
		// invalid and an error message is thrown.

	emString GetString() const;
	void GetString(char * buf, int bufSize) const;
		// Create a human-readable string representation (e.g.
		// "Ctrl+C"). If the hotkey is invalid, an empty string is
		// returned.

	bool IsValid() const;
		// Whether this is a valid hotkey.

	bool GetShift() const;
	bool GetCtrl() const;
	bool GetAlt() const;
	bool GetMeta() const;
		// Whether the respective modifier key is part of this hotkey.

	emInputKey GetKey() const;
		// Get the event key. Returns EM_KEY_NONE if the hotkey is not
		// valid.

	bool Match(const emInputEvent & event, const emInputState & state) const;
		// Ask whether the input event and input state matches this
		// hotkey.

	void ClearModifiers();
		// Remove all modifiers.

	void AddModifier(emInputKey modifier);
		// Add a modifier.

	void SetKey(emInputKey key);
		// Set the event key. Must be a non-modifier keyboard key or the
		// hotkey gets invalid.

private:

	static const char * Key2Name(emInputKey key);
	static emInputKey Name2Key(const char * name, int len);

	enum {
		MF_SHIFT=(1<<0),
		MF_CTRL =(1<<1),
		MF_ALT  =(1<<2),
		MF_META =(1<<3)
	};

	union {
		emUInt16 Packed;
		struct {
			emByte MFlags;
			emByte Key;
		} Data;
	};
};


//==============================================================================
//=========================== Inline Implementations ===========================
//==============================================================================

inline bool emIsMouseInputKey(emInputKey key)
{
	return key>=EM_KEY_LEFT_BUTTON;
}

inline bool emIsTouchInputKey(emInputKey key)
{
	return key==EM_KEY_TOUCH;
}

inline bool emIsKeyboardInputKey(emInputKey key)
{
	return key && key<EM_KEY_TOUCH;
}

inline bool emIsModifierInputKey(emInputKey key)
{
	return (((int)key)&0xF8)==0x80;
}

inline emInputKey emInputEvent::GetKey() const
{
	return Key;
}

inline const emString & emInputEvent::GetChars() const
{
	return Chars;
}

inline int emInputEvent::GetRepeat() const
{
	return Repeat;
}

inline int emInputEvent::GetVariant() const
{
	return Variant;
}

inline bool emInputEvent::IsEmpty() const
{
	return Key==EM_KEY_NONE && Chars.IsEmpty();
}

inline bool emInputEvent::IsMouseEvent() const
{
	return emIsMouseInputKey(Key);
}

inline bool emInputEvent::IsTouchEvent() const
{
	return emIsTouchInputKey(Key);
}

inline bool emInputEvent::IsKey(emInputKey key) const
{
	return Key==key;
}

inline bool emInputEvent::IsLeftButton() const
{
	return Key==EM_KEY_LEFT_BUTTON;
}

inline bool emInputEvent::IsMiddleButton() const
{
	return Key==EM_KEY_MIDDLE_BUTTON;
}

inline bool emInputEvent::IsRightButton() const
{
	return Key==EM_KEY_RIGHT_BUTTON;
}

inline double emInputState::GetMouseX() const
{
	return MouseX;
}

inline double emInputState::GetMouseY() const
{
	return MouseY;
}

inline void emInputState::SetMouse(double mouseX, double mouseY)
{
	MouseX=mouseX;
	MouseY=mouseY;
}

inline int emInputState::GetTouchCount() const
{
	return Touches.GetCount();
}

inline emUInt64 emInputState::GetTouchId(int index) const
{
	return Touches[index].Id;
}

inline double emInputState::GetTouchX(int index) const
{
	return Touches[index].X;
}

inline double emInputState::GetTouchY(int index) const
{
	return Touches[index].Y;
}

inline bool emInputState::GetLeftButton() const
{
	return (KeyStates[EM_KEY_LEFT_BUTTON>>3]&(1<<(EM_KEY_LEFT_BUTTON&7)))!=0;
}

inline bool emInputState::GetMiddleButton() const
{
	return (KeyStates[EM_KEY_MIDDLE_BUTTON>>3]&(1<<(EM_KEY_MIDDLE_BUTTON&7)))!=0;
}

inline bool emInputState::GetRightButton() const
{
	return (KeyStates[EM_KEY_RIGHT_BUTTON>>3]&(1<<(EM_KEY_RIGHT_BUTTON&7)))!=0;
}

inline bool emInputState::GetShift() const
{
	return (KeyStates[EM_KEY_SHIFT>>3]&(1<<(EM_KEY_SHIFT&7)))!=0;
}

inline bool emInputState::GetCtrl() const
{
	return (KeyStates[EM_KEY_CTRL>>3]&(1<<(EM_KEY_CTRL&7)))!=0;
}

inline bool emInputState::GetAlt() const
{
	return (KeyStates[EM_KEY_ALT>>3]&(1<<(EM_KEY_ALT&7)))!=0;
}

inline bool emInputState::GetMeta() const
{
	return (KeyStates[EM_KEY_META>>3]&(1<<(EM_KEY_META&7)))!=0;
}

inline const unsigned char * emInputState::GetKeyStates() const
{
	return KeyStates;
}

inline unsigned char * emInputState::GetKeyStates()
{
	return KeyStates;
}

inline emInputHotkey::emInputHotkey()
{
	Packed=0;
}

inline emInputHotkey::emInputHotkey(const emInputHotkey & hotkey)
{
	Packed=hotkey.Packed;
}

inline emInputHotkey & emInputHotkey::operator = (const emInputHotkey & hotkey)
{
	Packed=hotkey.Packed;
	return *this;
}

inline bool emInputHotkey::operator == (const emInputHotkey & hotkey) const
{
	return Packed==hotkey.Packed;
}

inline bool emInputHotkey::operator != (const emInputHotkey & hotkey) const
{
	return Packed!=hotkey.Packed;
}

inline bool emInputHotkey::IsValid() const
{
	return Data.Key!=(emByte)EM_KEY_NONE;
}

inline bool emInputHotkey::GetShift() const
{
	return (Data.MFlags&MF_SHIFT)!=0;
}

inline bool emInputHotkey::GetCtrl() const
{
	return (Data.MFlags&MF_CTRL)!=0;
}

inline bool emInputHotkey::GetAlt() const
{
	return (Data.MFlags&MF_ALT)!=0;
}

inline bool emInputHotkey::GetMeta() const
{
	return (Data.MFlags&MF_META)!=0;
}

inline emInputKey emInputHotkey::GetKey() const
{
	return (emInputKey)Data.Key;
}

inline void emInputHotkey::ClearModifiers()
{
	Data.MFlags=0;
}


#endif

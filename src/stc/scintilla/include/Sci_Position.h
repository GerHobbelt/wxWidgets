// Scintilla source code edit control
/** @file Sci_Position.h
 ** Define the Sci_Position type used in Scintilla's external interfaces.
 ** These need to be available to clients written in C so are not in a C++ namespace.
 **/
// Copyright 2015 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCI_POSITION_H
#define SCI_POSITION_H

// Basic signed type used throughout interface
using Sci_Position = int;

// Unsigned variant used for ILexer::Lex and ILexer::Fold
using Sci_PositionU = unsigned int;

// For Sci_CharacterRange  which is defined as long to be compatible with Win32 CHARRANGE
using Sci_PositionCR = long;

#endif

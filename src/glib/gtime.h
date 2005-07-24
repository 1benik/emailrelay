//
// Copyright (C) 2001-2005 Graeme Walker <graeme_walker@users.sourceforge.net>
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later
// version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
// 
// ===
//
// gtime.h
//

#ifndef G_TIME_H
#define G_TIME_H

#include "gdef.h"
#include "gexception.h"
#include "gdatetime.h"
#include <ctime>

namespace G
{
	class Time ;
}

// Class: G::Time
// Description: A simple time-of-day (hh/mm/ss) class.
// See also: G::Date, G::DateTime
//
class G::Time 
{
public:
	class LocalTime // An overload discriminator class for Time constructors.
		{} ;

	Time() ;
		// Constructor, using UTC, for now.

	explicit Time( const G::DateTime::BrokenDownTime & tm ) ;
		// Constructor for the given broken-down time.

	explicit Time( G::DateTime::EpochTime t ) ;
		// Constructor, using UTC, for the given epoch time.

	Time( G::DateTime::EpochTime t , const LocalTime & ) ;
		// Constructor, using the local timezone, for the given epoch time.

	explicit Time( const LocalTime & ) ;
		// Localtime constructor for now.

	unsigned int hours() const ;
		// Returns the hours (0 <= h < 24).

	unsigned int minutes() const ;
		// Returns the minutes (0 <= m < 60).

	unsigned int seconds() const ;
		// Returns the seconds (0 <= s <= 61 [sic]).

	std::string hhmmss( const char * sep = NULL ) const ;
		// Returns a hhmmss string.

	std::string hhmm( const char * sep = NULL ) const ;
		// Returns a hhmm string.

	std::string ss() const ;
		// Returns the seconds as a two-digit decimal seconds.

private:
	unsigned int m_hh ;
	unsigned int m_mm ;
	unsigned int m_ss ;
} ;

#endif

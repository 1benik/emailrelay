//
// Copyright (C) 2001-2013 Graeme Walker <graeme_walker@users.sourceforge.net>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// ===
//
// gsecrets_none.cpp
//

#include "gdef.h"
#include "gauth.h"
#include "gsecrets.h"

GAuth::Secrets::Secrets( const std::string & , const std::string & , const std::string & )
{
}

GAuth::Secrets::Secrets()
{
}

GAuth::Secrets::~Secrets()
{
}

std::string GAuth::Secrets::source() const
{
	return std::string() ;
}

bool GAuth::Secrets::valid() const
{
	return false ;
}

std::string GAuth::Secrets::id( const std::string & ) const
{
	return std::string() ;
}

std::string GAuth::Secrets::secret( const std::string & ) const
{
	return std::string() ;
}

std::string GAuth::Secrets::secret( const std::string & , const std::string & ) const
{
	return std::string() ;
}

bool GAuth::Secrets::contains( const std::string & ) const
{
	return false ;
}

/// \file gsecrets_none.cpp

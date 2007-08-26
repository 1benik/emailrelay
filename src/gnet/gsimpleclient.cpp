//
// Copyright (C) 2001-2007 Graeme Walker <graeme_walker@users.sourceforge.net>
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
// gsimpleclient.cpp
//

#include "gdef.h"
#include "gnet.h"
#include "gaddress.h"
#include "gsocket.h"
#include "gdatetime.h"
#include "gexception.h"
#include "gresolver.h"
#include "groot.h"
#include "gmonitor.h"
#include "gsimpleclient.h"
#include "gassert.h"
#include "gtest.h"
#include "gdebug.h"
#include "glog.h"

namespace
{
	const int c_retries = 10 ; // number of retries when using a privileged local port number
	const int c_port_start = 512 ;
	const int c_port_end = 1024 ;
	const size_t c_buffer_size = 1500U ; // see also gserver.h
	const char * c_cannot_connect_to = "cannot connect to " ;
}

// ==

GNet::SimpleClient::SimpleClient( const ResolverInfo & remote ,
	const Address & local_address , bool privileged , bool sync_dns ) :
		m_resolver(*this) ,
		m_s(NULL) ,
		m_remote(remote) ,
		m_local_address(local_address) ,
		m_privileged(privileged) ,
		m_state(Idle) ,
		m_sync_dns(sync_dns)
{
	G_DEBUG( "SimpleClient::ctor" ) ;
	if( Monitor::instance() ) Monitor::instance()->add( *this ) ;
}

GNet::SimpleClient::~SimpleClient()
{
	if( Monitor::instance() ) Monitor::instance()->remove( *this ) ;
	close() ;
}

std::string GNet::SimpleClient::logId() const
{
	std::string s = m_remote.displayString(true) ;
	if( m_s != NULL )
		s.append( std::string() + "@" + m_s->asString() ) ; // cf. ServerPeer::logId()
	return s ;
}

GNet::ResolverInfo GNet::SimpleClient::resolverInfo() const
{
	return m_remote ;
}

void GNet::SimpleClient::updateResolverInfo( const ResolverInfo & update )
{
	if( m_remote.host() == update.host() && m_remote.service() == update.service() && update.hasAddress() )
	{
		G_DEBUG( "GNet::SimpleClient::updateResolverInfo: reusing dns lookup for " << update.displayString() ) ;
		m_remote = update ;
	}
}

GNet::Socket & GNet::SimpleClient::socket()
{
	if( m_s == NULL )
		throw NotConnected() ;
	return *m_s ;
}

void GNet::SimpleClient::connect()
{
	G_DEBUG( "GNet::SimpleClient::connect: [" << m_remote.str() << "]" ) ;
	if( m_state != Idle )
	{
		G_WARNING( "SimpleClient::connect: invalid state" ) ;
		return ;
	}

	if( m_remote.hasAddress() )
	{
		bool immediate = startConnecting() ;
		if( immediate )
		{
			immediateConnection() ; // calls onConnect()
		}
		else
		{
			setState( Connecting ) ;
		}
	}
	else if( m_sync_dns )
	{
		std::string error = Resolver::resolve( m_remote ) ;
		if( !error.empty() )
		{
			throw DnsError( error ) ;
		}
		bool immediate = startConnecting() ;
		if( immediate )
		{
			immediateConnection() ; // calls onConnect()
		}
		else
		{
			setState( Connecting ) ;
		}
	}
	else
	{
		if( !m_resolver.resolveReq( m_remote.str() ) )
		{
			throw DnsError( m_remote.str() ) ;
		}
		setState( Resolving ) ;
	}
}

void GNet::SimpleClient::immediateConnection()
{
	G_DEBUG( "GNet::SimpleClient::connect: immediate connection" ) ;
	s().addReadHandler( *this ) ;
	s().addExceptionHandler( *this ) ;
	setState( Connected ) ;
	onConnectImp() ; // from within connect()
	onConnect() ; // from within connect()
}

bool GNet::SimpleClient::canRetry( const std::string & error )
{
	return error.find( c_cannot_connect_to ) == 0U ;
}

int GNet::SimpleClient::getRandomPort() 
{
	static bool first = true ;
	if( first )
	{
		std::srand( static_cast<unsigned int>(G::DateTime::now()) ) ;
		first = false ;
	}

	int r = std::rand() ;
	if( r < 0 ) r = -r ;
	r = r % (c_port_end - c_port_start) ;
	return r + c_port_start ;
}

GNet::StreamSocket & GNet::SimpleClient::s()
{ 
	G_ASSERT( m_s != NULL ) ;
	return *m_s ;
}

const GNet::StreamSocket & GNet::SimpleClient::s() const
{ 
	G_ASSERT( m_s != NULL ) ;
	return *m_s ;
}

void GNet::SimpleClient::close()
{
	StreamSocket * s = m_s ;
	m_s = NULL ;
	delete s ;
}

bool GNet::SimpleClient::connected() const
{
	return m_state == Connected ;
}

void GNet::SimpleClient::resolveCon( bool success , const Address & address , std::string name_or_reason )
{
	if( success )
	{
		G_DEBUG( "GNet::SimpleClient::resolveCon: " << address.displayString() ) ;
		std::string peer_name = name_or_reason ;
		m_remote.update( address , peer_name ) ;
		bool immediate = startConnecting() ;
		setState( immediate ? Connected : Connecting ) ;
	}
	else
	{
		throw DnsError( name_or_reason ) ;
	}
}

bool GNet::SimpleClient::startConnecting()
{
	G_DEBUG( "GNet::SimpleClient::startConnecting: " << m_remote.displayString() ) ;

	// create and open a socket
	//
	delete m_s ; m_s = NULL ; // just in case -- should be null
	m_s = new StreamSocket( m_remote.address() ) ;
	if( !s().valid() )
		throw ConnectError( "cannot open socket" ) ;

	// specifiy this as a 'write' event handler for the socket
	// (before the connect() in case it is reentrant)
	//
	s().addWriteHandler( *this ) ;

	// bind a local address to the socket and connect
	//
	ConnectStatus status = Failure ;
	std::string error ;
	if( m_privileged )
	{
		for( int i = 0 ; i < c_retries ; i++ )
		{
			int port = getRandomPort() ;
			m_local_address.setPort( port ) ;
			G_DEBUG( "GNet::SimpleClient::startConnecting: trying to bind " << m_local_address.displayString() ) ;
			status = localBind(m_local_address) ? Success : Retry ;
			if( status == Retry )
				continue ;

			status = connectCore( m_remote.address() , &error ) ;
			if( status != Retry )
				break ;
		}
	}
	else
	{
		status = connectCore( m_remote.address() , &error ) ;
	}

	// deal with immediate connection (typically if connecting locally)
	//
	bool immediate = status == ImmediateSuccess ;
	if( status != Success )
		s().dropWriteHandler() ;

	return immediate ;
}

bool GNet::SimpleClient::localBind( Address local_address )
{
	G::Root claim_root ;
	bool bound = s().bind(local_address) ;
	G_DEBUG( "GNet::SimpleClient::bind: bound local address " << local_address.displayString() ) ;
	return bound ;
}

GNet::SimpleClient::ConnectStatus GNet::SimpleClient::connectCore( Address remote_address , std::string *error_p )
{
	G_ASSERT( error_p != NULL ) ;
	std::string &error = *error_p ;

	// initiate the connection
	//
	bool immediate = false ;
	if( !s().connect( remote_address , &immediate ) )
	{
		G_DEBUG( "GNet::SimpleClient::connectCore: immediate failure" ) ;
		error = c_cannot_connect_to + remote_address.displayString() ; // see canRetry()

		// we should return Failure here, but Microsoft's stack
		// will happily bind the same local address more than once,
		// so it is the connect that fails, not the bind, if
		// the port was already in use
		//
		return Retry ;
	}
	else 
	{
		return immediate ? ImmediateSuccess : Success ;
	}
}

void GNet::SimpleClient::writeEvent()
{
	G_DEBUG( "GNet::SimpleClient::writeEvent" ) ;

	if( m_state == Connected )
	{
		s().dropWriteHandler() ;
		onWriteable() ;
	}
	else if( m_state == Connecting && s().hasPeer() )
	{
		s().addReadHandler( *this ) ;
		s().addExceptionHandler( *this ) ;
		s().dropWriteHandler() ;

		setState( Connected ) ;
		onConnectImp() ;
		onConnect() ;
	}
	else if( m_state == Connecting )
	{
		throw G::Exception( c_cannot_connect_to + m_remote.address().displayString() ) ; // see canRetry()
	}
}

void GNet::SimpleClient::readEvent()
{
	char buffer[c_buffer_size] ;
	buffer[0] = '\0' ;
	const size_t buffer_size = G::Test::enabled("small-client-input-buffer") ? 3 : sizeof(buffer) ;
	ssize_t rc = s().read( buffer , buffer_size ) ;

	if( rc == 0 || ( rc == -1 && !s().eWouldBlock() ) )
	{
		throw ReadError() ;
	}
	else if( rc != -1 )
	{
		G_ASSERT( static_cast<size_t>(rc) <= buffer_size ) ;
		onData( buffer , static_cast<size_type>(rc) ) ;
	}
	else
	{
		; // no-op (windows)
	}
}

void GNet::SimpleClient::setState( State new_state )
{
	m_state = new_state ;
}

std::pair<bool,GNet::Address> GNet::SimpleClient::localAddress() const
{
	return 
		m_s != NULL ?
			s().getLocalAddress() :
			std::make_pair(false,GNet::Address::invalidAddress()) ;
}

std::pair<bool,GNet::Address> GNet::SimpleClient::peerAddress() const
{
	return 
		m_s != NULL ?
			s().getPeerAddress() :
			std::make_pair(false,GNet::Address::invalidAddress()) ;
}

void GNet::SimpleClient::onConnectImp()
{
}

// ===

GNet::ClientResolver::ClientResolver( SimpleClient & client ) :
	Resolver(client) ,
	m_client(client)
{
}

void GNet::ClientResolver::resolveCon( bool success , const Address &address , 
	std::string reason )
{
	m_client.resolveCon( success , address , reason ) ;
}

/// \file gsimpleclient.cpp
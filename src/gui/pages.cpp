//
// Copyright (C) 2001-2006 Graeme Walker <graeme_walker@users.sourceforge.net>
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
// pages.cpp
//

#include "qt.h"
#include "thread.h"
#include "pages.h"
#include "legal.h"
#include "dir.h"
#include "gfile.h"
#include "gprocess.h"
#include "gidentity.h"
#include "gmd5.h"
#include "gdebug.h"
#include <stdexcept>

namespace
{
	std::string rot13( const std::string & in )
	{
		std::string s( in ) ;
		for( std::string::iterator p = s.begin() ; p != s.end() ; ++p )
		{
			if( *p >= 'a' && *p <= 'z' ) 
				*p = 'a' + ( ( ( *p - 'a' ) + 13U ) % 26U ) ;
			if( *p >= 'A' && *p <= 'Z' )
				*p = 'A' + ( ( ( *p - 'A' ) + 13U ) % 26U ) ;
		}
		return s ;
	}
	std::string encrypt( const std::string & pwd , const std::string & mechanism )
	{
		return mechanism == "CRAM-MD5" ? G::Md5::mask(pwd) : rot13(pwd) ;
	}
}

// ==

TitlePage::TitlePage( GDialog & dialog , const std::string & name , 
	const std::string & next_1 , const std::string & next_2 , bool finish , bool close ) : 
		GPage(dialog,name,next_1,next_2,finish,close)
{
	m_label = new QLabel( Legal::text() ) ;

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(newTitle(tr("E-MailRelay"))) ;
	layout->addWidget(m_label);
	setLayout(layout);
}

std::string TitlePage::nextPage()
{
	return next1() ;
}

void TitlePage::dump( std::ostream & stream , const std::string & prefix , const std::string & eol ) const
{
	GPage::dump( stream , prefix , eol ) ;
}

// ==

LicensePage::LicensePage( GDialog & dialog , const std::string & name , 
	const std::string & next_1 , const std::string & next_2 , bool finish , bool close ) : 
		GPage(dialog,name,next_1,next_2,finish,close)
{
	m_text_edit = new QTextEdit;
	m_text_edit->setReadOnly(true) ;
	m_text_edit->setWordWrapMode(QTextOption::NoWrap) ;
	m_text_edit->setLineWrapMode(QTextEdit::NoWrap) ;
	m_text_edit->setFontFamily("courier") ;
	m_text_edit->setPlainText(Legal::license()) ;

	m_agree_checkbox = new QCheckBox(tr("I agree to the terms and conditions of the license"));
	setFocusProxy( m_agree_checkbox ) ;

	if( testMode() )
		m_agree_checkbox->setChecked(true) ;

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(newTitle(tr("License"))) ;
	layout->addWidget(m_text_edit);
	layout->addWidget(m_agree_checkbox);
	setLayout(layout);

	connect( m_agree_checkbox , SIGNAL(toggled(bool)) , this , SIGNAL(pageUpdateSignal()) ) ;
}

std::string LicensePage::nextPage()
{
	return next1() ;
}

void LicensePage::dump( std::ostream & stream , const std::string & prefix , const std::string & eol ) const
{
	GPage::dump( stream , prefix , eol ) ;
}

bool LicensePage::isComplete()
{
	return m_agree_checkbox->isChecked() ;
}

// ==

DirectoryPage::DirectoryPage( GDialog & dialog , const std::string & name ,
	const std::string & next_1 , const std::string & next_2 , bool finish , bool close ) : 
		GPage(dialog,name,next_1,next_2,finish,close)
{
	m_install_dir_label = new QLabel(tr("&Directory:")) ;
	m_install_dir_edit_box = new QLineEdit ;
	m_install_dir_label->setBuddy(m_install_dir_edit_box) ;
	m_install_dir_browse_button = new QPushButton(tr("B&rowse")) ;

	QHBoxLayout * install_layout = new QHBoxLayout ;
	install_layout->addWidget( m_install_dir_label ) ;
	install_layout->addWidget( m_install_dir_edit_box ) ;
	install_layout->addWidget( m_install_dir_browse_button ) ;

	QGroupBox * install_group = new QGroupBox(tr("Installation directory")) ;
	install_group->setLayout( install_layout ) ;

	//

	m_spool_dir_label = new QLabel(tr("D&irectory:")) ;
	m_spool_dir_edit_box = new QLineEdit ;
	m_spool_dir_label->setBuddy(m_spool_dir_edit_box) ;
	m_spool_dir_browse_button = new QPushButton(tr("B&rowse")) ;

	QHBoxLayout * spool_layout = new QHBoxLayout ;
	spool_layout->addWidget( m_spool_dir_label ) ;
	spool_layout->addWidget( m_spool_dir_edit_box ) ;
	spool_layout->addWidget( m_spool_dir_browse_button ) ;

	QGroupBox * spool_group = new QGroupBox(tr("Spool directory")) ;
	spool_group->setLayout( spool_layout ) ;

	//

	m_config_dir_label = new QLabel(tr("Dir&ectory:")) ;
	m_config_dir_edit_box = new QLineEdit ;
	m_config_dir_label->setBuddy(m_config_dir_edit_box) ;
	m_config_dir_browse_button = new QPushButton(tr("B&rowse")) ;

	QHBoxLayout * config_layout = new QHBoxLayout ;
	config_layout->addWidget( m_config_dir_label ) ;
	config_layout->addWidget( m_config_dir_edit_box ) ;
	config_layout->addWidget( m_config_dir_browse_button ) ;

	QGroupBox * config_group = new QGroupBox(tr("Configuration directory")) ;
	config_group->setLayout( config_layout ) ;

	setFocusProxy( m_install_dir_edit_box ) ;

	m_install_dir_edit_box->setText( QString(Dir::install().str().c_str()) ) ;
	m_spool_dir_edit_box->setText( QString(Dir::spool().str().c_str()) ) ;
	m_config_dir_edit_box->setText( QString(Dir::config().str().c_str()) ) ;

	QVBoxLayout * layout = new QVBoxLayout;
	layout->addWidget(newTitle(tr("Directories"))) ;
	layout->addWidget( install_group ) ;
	layout->addWidget( spool_group ) ;
	layout->addWidget( config_group ) ;
	layout->addStretch() ;
	setLayout( layout ) ;

	connect( m_install_dir_browse_button , SIGNAL(clicked()) , this , SLOT(browseInstall()) ) ;
	connect( m_spool_dir_browse_button , SIGNAL(clicked()) , this , SLOT(browseSpool()) ) ;
	connect( m_config_dir_browse_button , SIGNAL(clicked()) , this , SLOT(browseConfig()) ) ;

	connect( m_install_dir_edit_box , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()));
	connect( m_spool_dir_edit_box , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()));
	connect( m_config_dir_edit_box , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()));
}

void DirectoryPage::browseInstall()
{
	QString s = browse(m_install_dir_edit_box->text()) ;
	if( ! s.isEmpty() )
		m_install_dir_edit_box->setText( s ) ;
}

void DirectoryPage::browseSpool()
{
	QString s = browse(m_spool_dir_edit_box->text()) ;
	if( ! s.isEmpty() )
		m_spool_dir_edit_box->setText( s ) ;
}

void DirectoryPage::browseConfig()
{
	QString s = browse(m_config_dir_edit_box->text()) ;
	if( ! s.isEmpty() )
		m_config_dir_edit_box->setText( s ) ;
}

QString DirectoryPage::browse( QString dir )
{
	return QFileDialog::getExistingDirectory( this , QString() , dir ) ;
}

std::string DirectoryPage::nextPage()
{
	return next1() ;
}

void DirectoryPage::dump( std::ostream & stream , const std::string & prefix , const std::string & eol ) const
{
	GPage::dump( stream , prefix , eol ) ;
	stream << prefix << "dir-install: " << value(m_install_dir_edit_box) << eol ;
	stream << prefix << "dir-spool: " << value(m_spool_dir_edit_box) << eol ;
	stream << prefix << "dir-config: " << value(m_config_dir_edit_box) << eol ;
	stream << prefix << "dir-pid: " << Dir::pid() << eol ;
	stream << prefix << "dir-desktop: " << Dir::desktop() << eol ;
	stream << prefix << "dir-login: " << Dir::login() << eol ;
	stream << prefix << "dir-menu: " << Dir::menu() << eol ;
	stream << prefix << "dir-reskit: " << Dir::reskit() << eol ;
	stream << prefix << "dir-boot: " << Dir::boot() << eol ;
}

bool DirectoryPage::isComplete()
{
	return 
		!m_install_dir_edit_box->text().isEmpty() &&
		!m_spool_dir_edit_box->text().isEmpty() &&
		!m_config_dir_edit_box->text().isEmpty() ;
}

// ==

DoWhatPage::DoWhatPage( GDialog & dialog, const std::string & name , 
	const std::string & next_1 , const std::string & next_2 , bool finish , bool close ) : 
		GPage(dialog,name,next_1,next_2,finish,close)
{
	m_pop_checkbox = new QCheckBox(tr("&POP3 server"));
	m_smtp_checkbox = new QCheckBox(tr("&SMTP server"));
	m_smtp_checkbox->setChecked(true) ;
	if( testMode() )
		m_pop_checkbox->setChecked(true) ;

	QVBoxLayout * server_type_box_layout = new QVBoxLayout ;
	server_type_box_layout->addWidget( m_pop_checkbox ) ;
	server_type_box_layout->addWidget( m_smtp_checkbox ) ;

	QGroupBox * server_type_group = new QGroupBox(tr("Server")) ;
	server_type_group->setLayout( server_type_box_layout ) ;

	m_immediate_checkbox = new QRadioButton(tr("&After a message is received"));
	m_periodically_checkbox = new QRadioButton(tr("&Check periodically"));
	m_on_demand_checkbox = new QRadioButton(tr("&Only when triggered"));
	m_immediate_checkbox->setChecked(true) ;

	QLabel * period_label = new QLabel( tr("e&very") ) ;
	m_period_combo = new QComboBox ;
	m_period_combo->addItem( tr("second") ) ;
	m_period_combo->addItem( tr("minute") ) ;
	m_period_combo->addItem( tr("hour") ) ;
	m_period_combo->setCurrentIndex( 1 ) ;
	m_period_combo->setEditable( false ) ;
	period_label->setBuddy( m_period_combo ) ;

	QVBoxLayout * forwarding_box_layout = new QVBoxLayout ;
	forwarding_box_layout->addWidget( m_immediate_checkbox ) ;
	{
		QHBoxLayout * inner = new QHBoxLayout ;
		inner->addWidget( m_periodically_checkbox ) ;
		inner->addWidget( period_label ) ;
		inner->addWidget( m_period_combo ) ;
		forwarding_box_layout->addLayout( inner ) ;
	}
	forwarding_box_layout->addWidget( m_on_demand_checkbox ) ;

	m_forwarding_group = new QGroupBox(tr("SMTP forwarding")) ;
	m_forwarding_group->setLayout( forwarding_box_layout ) ;

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(newTitle(tr("Installation type"))) ;
	layout->addWidget( server_type_group ) ;
	layout->addWidget( m_forwarding_group ) ;
	layout->addStretch() ;
	setLayout(layout);

	connect( m_pop_checkbox , SIGNAL(toggled(bool)) , this , SIGNAL(pageUpdateSignal()) ) ;
	connect( m_smtp_checkbox , SIGNAL(toggled(bool)) , this , SIGNAL(pageUpdateSignal()) ) ;
	connect( m_periodically_checkbox , SIGNAL(toggled(bool)) , this , SLOT(onToggle()) ) ;
	connect( m_smtp_checkbox , SIGNAL(toggled(bool)) , this , SLOT(onToggle()) ) ;

	onToggle() ;
}

void DoWhatPage::onToggle()
{
	m_period_combo->setEnabled( m_smtp_checkbox->isChecked() && m_periodically_checkbox->isChecked() ) ;
	m_forwarding_group->setEnabled( m_smtp_checkbox->isChecked() ) ;
}

std::string DoWhatPage::nextPage()
{
	// sneaky feature...
	if( dialog().currentPageName() != name() )
	{
		return m_smtp_checkbox->isChecked() ? next2() : std::string() ;
	}

	return 
		m_pop_checkbox->isChecked() ?
			next1() :
			next2() ;
}

void DoWhatPage::dump( std::ostream & stream , const std::string & prefix , const std::string & eol ) const
{
	GPage::dump( stream , prefix , eol ) ;
	stream << prefix << "do-pop: " << value(m_pop_checkbox) << eol ;
	stream << prefix << "do-smtp: " << value(m_smtp_checkbox) << eol ;
	stream << prefix << "forward-immediate: " << value(m_immediate_checkbox) << eol ;
	stream << prefix << "forward-poll: " << value(m_periodically_checkbox) << eol ;
	stream << prefix << "forward-poll-period: " << value(m_period_combo) << eol ;
}

bool DoWhatPage::isComplete()
{
	return 
		m_pop_checkbox->isChecked() ||
		m_smtp_checkbox->isChecked() ;
}

// ==

PopPage::PopPage( GDialog & dialog , const std::string & name ,
	const std::string & next_1 , const std::string & next_2 , bool finish , bool close ) : 
		GPage(dialog,name,next_1,next_2,finish,close)
{
	QLabel * port_label = new QLabel( tr("P&ort:") ) ;
	m_port_edit_box = new QLineEdit( tr("110") ) ;
	port_label->setBuddy( m_port_edit_box ) ;

	QHBoxLayout * server_layout = new QHBoxLayout ;
	server_layout->addWidget( port_label ) ;
	server_layout->addWidget( m_port_edit_box ) ;

	QGroupBox * server_group = new QGroupBox(tr("Local server")) ;
	server_group->setLayout( server_layout ) ;

	m_one = new QRadioButton( tr("&One client") ) ;
	m_shared = new QRadioButton( tr("&Many clients sharing a spool directory") ) ;
	m_pop_by_name = new QRadioButton( tr("M&any clients with separate spool directories") ) ;
	m_one->setChecked(true) ;

	m_no_delete_checkbox = new QCheckBox( tr("Disable message deletion") ) ;
	m_no_delete_checkbox->setChecked(true) ;

	m_auto_copy_checkbox = new QCheckBox( tr("Copy SMTP messages to all") ) ;
	m_auto_copy_checkbox->setChecked(false) ;

	QGridLayout * radio_layout = new QGridLayout ;
	radio_layout->addWidget( m_one , 0 , 0 ) ;
	radio_layout->addWidget( m_shared , 1 , 0 ) ;
	radio_layout->addWidget( m_no_delete_checkbox , 1 , 1 ) ;
	radio_layout->addWidget( m_pop_by_name , 2 , 0 ) ;
	radio_layout->addWidget( m_auto_copy_checkbox , 2 , 1 ) ;

	QGroupBox * accounts_group = new QGroupBox(tr("Client accounts")) ;
	accounts_group->setLayout( radio_layout ) ;

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(newTitle(tr("POP server"))) ;
	layout->addWidget( server_group ) ;
	layout->addWidget( accounts_group ) ;
	layout->addStretch() ;
	setLayout( layout ) ;

	connect( m_port_edit_box , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()) ) ;
	connect( m_one , SIGNAL(toggled(bool)) , this , SLOT(onToggle()) ) ;
	connect( m_shared , SIGNAL(toggled(bool)) , this , SLOT(onToggle()) ) ;
	connect( m_pop_by_name , SIGNAL(toggled(bool)) , this , SLOT(onToggle()) ) ;

	onToggle() ;
}

std::string PopPage::nextPage()
{
	return 
		m_one->isChecked() ?
			next1() :
			next2() ;
}

void PopPage::dump( std::ostream & stream , const std::string & prefix , const std::string & eol ) const
{
	GPage::dump( stream , prefix , eol ) ;
	stream << prefix << "pop-port: " << value(m_port_edit_box) << eol ;
	stream << prefix << "pop-simple: " << value(m_one) << eol ;
	stream << prefix << "pop-shared: " << value(m_shared) << eol ;
	stream << prefix << "pop-shared-no-delete: " << value(m_no_delete_checkbox) << eol ;
	stream << prefix << "pop-by-name: " << value(m_pop_by_name) << eol ;
	stream << prefix << "pop-by-name-auto-copy: " << value(m_auto_copy_checkbox) << eol ;
}

bool PopPage::isComplete()
{
	return ! m_port_edit_box->text().isEmpty() ;
}

void PopPage::onToggle()
{
	m_no_delete_checkbox->setEnabled( m_shared->isChecked() ) ;
	m_auto_copy_checkbox->setEnabled( m_pop_by_name->isChecked() ) ;
}

// ==

PopAccountsPage::PopAccountsPage( GDialog & dialog , const std::string & name ,
	const std::string & next_1 , const std::string & next_2 , bool finish , bool close ) : 
		GPage(dialog,name,next_1,next_2,finish,close)
{
	m_mechanism_combo = new QComboBox ;
	m_mechanism_combo->addItem( tr("APOP") ) ;
	m_mechanism_combo->addItem( tr("CRAM-MD5") ) ;
	m_mechanism_combo->addItem( tr("LOGIN") ) ;
	m_mechanism_combo->setCurrentIndex( 1 ) ;
	m_mechanism_combo->setEditable( false ) ;
	QLabel * mechanism_label = new QLabel( tr("Authentication &mechanism") ) ;
	mechanism_label->setBuddy( m_mechanism_combo ) ;

	QGridLayout * account_layout = new QGridLayout ;
	QLabel * name_label = new QLabel(tr("Name:")) ;
	QLabel * pwd_label = new QLabel(tr("Password:")) ;
	m_name_1 = new QLineEdit ;
	m_pwd_1 = new QLineEdit ;
	m_pwd_1->setEchoMode( QLineEdit::Password ) ;
	m_name_2 = new QLineEdit ;
	m_pwd_2 = new QLineEdit ;
	m_pwd_2->setEchoMode( QLineEdit::Password ) ;
	m_name_3 = new QLineEdit ;
	m_pwd_3 = new QLineEdit ;
	m_pwd_3->setEchoMode( QLineEdit::Password ) ;
	account_layout->addWidget( name_label , 0 , 0 ) ;
	account_layout->addWidget( pwd_label , 0 , 1 ) ;
	account_layout->addWidget( m_name_1 , 1 , 0 ) ;
	account_layout->addWidget( m_pwd_1 , 1 , 1 ) ;
	account_layout->addWidget( m_name_2 , 2 , 0 ) ;
	account_layout->addWidget( m_pwd_2 , 2 , 1 ) ;
	account_layout->addWidget( m_name_3 , 3 , 0 ) ;
	account_layout->addWidget( m_pwd_3 , 3 , 1 ) ;

	if( testMode() )
	{
		m_name_1->setText("me") ;
		m_pwd_1->setText("secret") ;
	}

	QGroupBox * account_group = new QGroupBox(tr("Accounts")) ;
	account_group->setLayout( account_layout ) ;

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(newTitle(tr("POP accounts"))) ;
	{
		QHBoxLayout * inner = new QHBoxLayout ;
		inner->addWidget( mechanism_label ) ;
		inner->addWidget( m_mechanism_combo ) ;
		layout->addLayout( inner ) ;
	}
	layout->addWidget( account_group ) ;
	layout->addStretch() ;
	setLayout( layout ) ;

	connect( m_mechanism_combo , SIGNAL(currentIndexChanged(QString)), this, SLOT(mechanismUpdateSlot(QString)) ) ;
	connect( m_name_1 , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()) ) ;
	connect( m_pwd_1 , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()) ) ;
	connect( m_name_2 , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()) ) ;
	connect( m_pwd_2 , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()) ) ;
	connect( m_name_3 , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()) ) ;
	connect( m_pwd_3 , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()) ) ;
}

std::string PopAccountsPage::nextPage()
{
	// only the dowhat page knows whether we should do smtp -- a special	
	// feature of the dowhat page's nextPage() is that if it detects
	// that it is not the current page then it will give us an empty string 
	// if no smtp is required

	return
		dialog().previousPage(2U).nextPage().empty() ?
			next2() :
			next1() ;
}

void PopAccountsPage::dump( std::ostream & stream , const std::string & prefix , const std::string & eol ) const
{
	GPage::dump( stream , prefix , eol ) ;
	stream << prefix << "pop-auth-mechanism: " << value(m_mechanism_combo) << eol ;
	stream << prefix << "pop-account-1-name: " << value(m_name_1) << eol ;
	stream << prefix << "pop-account-1-password: " << encrypt(value(m_pwd_1),value(m_mechanism_combo)) << eol ;
	stream << prefix << "pop-account-2-name: " << value(m_name_2) << eol ;
	stream << prefix << "pop-account-2-password: " << encrypt(value(m_pwd_2),value(m_mechanism_combo)) << eol ;
	stream << prefix << "pop-account-3-name: " << value(m_name_3) << eol ;
	stream << prefix << "pop-account-3-password: " << encrypt(value(m_pwd_3),value(m_mechanism_combo)) << eol ;
}

bool PopAccountsPage::isComplete()
{
	return
		( !m_name_1->text().isEmpty() && !m_pwd_1->text().isEmpty() ) ||
		( !m_name_2->text().isEmpty() && !m_pwd_2->text().isEmpty() ) ||
		( !m_name_3->text().isEmpty() && !m_pwd_3->text().isEmpty() ) ;
}

// ==

PopAccountPage::PopAccountPage( GDialog & dialog , const std::string & name ,
	const std::string & next_1 , const std::string & next_2 , bool finish , bool close ) : 
		GPage(dialog,name,next_1,next_2,finish,close)
{
	m_mechanism_combo = new QComboBox ;
	m_mechanism_combo->addItem( tr("APOP") ) ;
	m_mechanism_combo->addItem( tr("CRAM-MD5") ) ;
	m_mechanism_combo->addItem( tr("LOGIN") ) ;
	m_mechanism_combo->setCurrentIndex( 1 ) ;
	m_mechanism_combo->setEditable( false ) ;
	QLabel * mechanism_label = new QLabel( tr("Authentication &mechanism") ) ;
	mechanism_label->setBuddy( m_mechanism_combo ) ;

	QGridLayout * account_layout = new QGridLayout ;
	QLabel * name_label = new QLabel(tr("Name:")) ;
	QLabel * pwd_label = new QLabel(tr("Password:")) ;
	m_name_1 = new QLineEdit ;
	m_pwd_1 = new QLineEdit ;
	m_pwd_1->setEchoMode( QLineEdit::Password ) ;
	account_layout->addWidget( name_label , 0 , 0 ) ;
	account_layout->addWidget( m_name_1 , 0 , 1 ) ;
	account_layout->addWidget( pwd_label , 1 , 0 ) ;
	account_layout->addWidget( m_pwd_1 , 1 , 1 ) ;

	if( testMode() )
	{
		m_name_1->setText("me") ;
		m_pwd_1->setText("secret") ;
	}

	QGroupBox * account_group = new QGroupBox(tr("Account")) ;
	account_group->setLayout( account_layout ) ;

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(newTitle(tr("POP account"))) ;
	{
		QHBoxLayout * inner = new QHBoxLayout ;
		inner->addWidget( mechanism_label ) ;
		inner->addWidget( m_mechanism_combo ) ;
		layout->addLayout( inner ) ;
	}
	layout->addWidget( account_group ) ;
	layout->addStretch() ;
	setLayout( layout ) ;

	connect( m_mechanism_combo , SIGNAL(currentIndexChanged(QString)), this, SLOT(mechanismUpdateSlot(QString)) ) ;
	connect( m_name_1 , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()) ) ;
	connect( m_pwd_1 , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()) ) ;
}

std::string PopAccountPage::nextPage()
{
	// (see above)
	return
		dialog().previousPage(2U).nextPage().empty() ?
			next2() :
			next1() ;
}

void PopAccountPage::dump( std::ostream & stream , const std::string & prefix , const std::string & eol ) const
{
	GPage::dump( stream , prefix , eol ) ;
	stream << prefix << "pop-auth-mechanism: " << value(m_mechanism_combo) << eol ;
	stream << prefix << "pop-account-1-name: " << value(m_name_1) << eol ;
	stream << prefix << "pop-account-1-password: " << encrypt(value(m_pwd_1),value(m_mechanism_combo)) << eol ;
}

bool PopAccountPage::isComplete()
{
	return
		!m_name_1->text().isEmpty() && !m_pwd_1->text().isEmpty() ;
}

// ==

SmtpServerPage::SmtpServerPage( GDialog & dialog , const std::string & name , 
	const std::string & next_1 , const std::string & next_2 , bool finish , bool close ) : 
		GPage(dialog,name,next_1,next_2,finish,close)
{
	QLabel * port_label = new QLabel(tr("P&ort:")) ;
	m_port_edit_box = new QLineEdit(tr("25")) ;
	port_label->setBuddy( m_port_edit_box ) ;

	QHBoxLayout * server_layout = new QHBoxLayout ;
	server_layout->addWidget( port_label ) ;
	server_layout->addWidget( m_port_edit_box ) ;

	QGroupBox * server_group = new QGroupBox(tr("Local server")) ;
	server_group->setLayout( server_layout ) ;

	//

	m_auth_checkbox = new QCheckBox( tr("&Require authentication") ) ;

	m_mechanism_combo = new QComboBox ;
	m_mechanism_combo->addItem( tr("CRAM-MD5") ) ;
	m_mechanism_combo->addItem( tr("LOGIN") ) ;
	m_mechanism_combo->setCurrentIndex( 0 ) ;
	m_mechanism_combo->setEditable( false ) ;
	QLabel * mechanism_label = new QLabel( tr("Authentication &mechanism") ) ;
	mechanism_label->setBuddy( m_mechanism_combo ) ;

	QHBoxLayout * mechanism_layout = new QHBoxLayout ;
	mechanism_layout->addWidget( mechanism_label ) ;
	mechanism_layout->addWidget( m_mechanism_combo ) ;

	//

	QLabel * account_name_label = new QLabel(tr("&Name:")) ;
	m_account_name = new QLineEdit ;
	account_name_label->setBuddy( m_account_name ) ;

	QLabel * account_pwd_label = new QLabel(tr("&Password:")) ;
	m_account_pwd = new QLineEdit ;
	m_account_pwd->setEchoMode( QLineEdit::Password ) ;
	account_pwd_label->setBuddy( m_account_pwd ) ;

	if( testMode() )
	{
		m_auth_checkbox->setChecked(true) ;
		m_account_name->setText("me") ;
		m_account_pwd->setText("secret") ;
	}

	QGridLayout * account_layout = new QGridLayout ;
	account_layout->addWidget( account_name_label , 0 , 0 ) ;
	account_layout->addWidget( m_account_name , 0 , 1 ) ;
	account_layout->addWidget( account_pwd_label , 1 , 0 ) ;
	account_layout->addWidget( m_account_pwd , 1 , 1 ) ;

	m_account_group = new QGroupBox(tr("Account")) ;
	m_account_group->setLayout( account_layout ) ;

	//

	QLabel * trust_label = new QLabel(tr("&IP address:")) ;
	m_trust_address = new QLineEdit ;
	trust_label->setBuddy( m_trust_address ) ;
	m_trust_group = new QGroupBox(tr("Exemptions")) ;
	QHBoxLayout * trust_layout = new QHBoxLayout ;
	trust_layout->addWidget( trust_label ) ;
	trust_layout->addWidget( m_trust_address ) ;
	m_trust_group->setLayout( trust_layout ) ;

	if( testMode() )
	{
		m_trust_address->setText( "192.168.0.*" ) ;
	}
	
	//

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(newTitle(tr("SMTP server"))) ;
	layout->addWidget( server_group ) ;
	layout->addWidget( m_auth_checkbox ) ;
	layout->addLayout( mechanism_layout ) ;
	layout->addWidget( m_account_group ) ;
	layout->addWidget( m_trust_group ) ;
	layout->addStretch() ;
	setLayout( layout ) ;

	connect( m_mechanism_combo , SIGNAL(currentIndexChanged(QString)), this, SLOT(mechanismUpdateSlot(QString)) ) ;
	connect( m_port_edit_box , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()) ) ;
	connect( m_account_name , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()) ) ;
	connect( m_account_pwd , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()) ) ;
	connect( m_trust_address , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()) ) ;
	connect( m_auth_checkbox , SIGNAL(toggled(bool)), this, SIGNAL(pageUpdateSignal()) ) ;
	connect( m_auth_checkbox , SIGNAL(toggled(bool)), this, SLOT(onToggle()) ) ;

	onToggle() ;
}

std::string SmtpServerPage::nextPage()
{
	return next1() ;
}

void SmtpServerPage::dump( std::ostream & stream , const std::string & prefix , const std::string & eol ) const
{
	GPage::dump( stream , prefix , eol ) ;
	stream << prefix << "smtp-server-port: " << value(m_port_edit_box) << eol ;
	stream << prefix << "smtp-server-auth: " << value(m_auth_checkbox) << eol ;
	stream << prefix << "smtp-server-auth-mechanism: " << value(m_mechanism_combo) << eol ;
	stream << prefix << "smtp-server-account-name: " << value(m_account_name) << eol ;
	stream << prefix << "smtp-server-account-password: " << encrypt(value(m_account_pwd),value(m_mechanism_combo)) << eol ;
	stream << prefix << "smtp-server-trust: " << value(m_trust_address) << eol ;
}

void SmtpServerPage::onToggle()
{
	m_account_group->setEnabled( m_auth_checkbox->isChecked() ) ;
	m_mechanism_combo->setEnabled( m_auth_checkbox->isChecked() ) ;
	m_trust_group->setEnabled( m_auth_checkbox->isChecked() ) ;
}

bool SmtpServerPage::isComplete()
{
	return
		! m_port_edit_box->text().isEmpty() &&
		( ! m_auth_checkbox->isChecked() || (
			! m_account_name->text().isEmpty() &&
			! m_account_pwd->text().isEmpty() ) ) ;
}

// ==

SmtpClientPage::SmtpClientPage( GDialog & dialog , const std::string & name , 
	const std::string & next_1 , const std::string & next_2 , bool finish , bool close ) : 
		GPage(dialog,name,next_1,next_2,finish,close)
{
	QLabel * server_label = new QLabel(tr("&Hostname:")) ;
	m_server_edit_box = new QLineEdit ;
	server_label->setBuddy( m_server_edit_box ) ;

	if( testMode() )
		m_server_edit_box->setText("myisp.net") ;

	QLabel * port_label = new QLabel( tr("P&ort:") ) ;
	m_port_edit_box = new QLineEdit( tr("25") ) ;
	port_label->setBuddy( m_port_edit_box ) ;

	QHBoxLayout * server_layout = new QHBoxLayout ;
	server_layout->addWidget( server_label ) ;
	server_layout->addWidget( m_server_edit_box ) ;
	server_layout->addWidget( port_label ) ;
	server_layout->addWidget( m_port_edit_box ) ;
	server_layout->setStretchFactor( m_server_edit_box , 4 ) ;

	QGroupBox * server_group = new QGroupBox(tr("Remote server")) ;
	server_group->setLayout( server_layout ) ;

	m_auth_checkbox = new QCheckBox( tr("&Supply authentication") ) ;

	m_mechanism_combo = new QComboBox ;
	m_mechanism_combo->addItem( tr("CRAM-MD5") ) ;
	m_mechanism_combo->addItem( tr("LOGIN") ) ;
	m_mechanism_combo->setCurrentIndex( 0 ) ;
	m_mechanism_combo->setEditable( false ) ;
	QLabel * mechanism_label = new QLabel( tr("Authentication &mechanism") ) ;
	mechanism_label->setBuddy( m_mechanism_combo ) ;

	QLabel * account_name_label = new QLabel(tr("&Name:")) ;
	m_account_name = new QLineEdit ;
	account_name_label->setBuddy( m_account_name ) ;

	QLabel * account_pwd_label = new QLabel(tr("&Password:")) ;
	m_account_pwd = new QLineEdit ;
	m_account_pwd->setEchoMode( QLineEdit::Password ) ;
	account_pwd_label->setBuddy( m_account_pwd ) ;

	if( testMode() )
	{
		m_auth_checkbox->setChecked(true) ;
		m_account_name->setText("me") ;
		m_account_pwd->setText("secret") ;
	}

	QGridLayout * account_layout = new QGridLayout ;
	account_layout->addWidget( account_name_label , 0 , 0 ) ;
	account_layout->addWidget( m_account_name , 0 , 1 ) ;
	account_layout->addWidget( account_pwd_label , 1 , 0 ) ;
	account_layout->addWidget( m_account_pwd , 1 , 1 ) ;

	m_account_group = new QGroupBox(tr("Account")) ;
	m_account_group->setLayout( account_layout ) ;

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(newTitle(tr("SMTP client"))) ;
	layout->addWidget( server_group ) ;
	layout->addWidget( m_auth_checkbox ) ;
	{
		QHBoxLayout * inner = new QHBoxLayout ;
		inner->addWidget( mechanism_label ) ;
		inner->addWidget( m_mechanism_combo ) ;
		layout->addLayout( inner ) ;
	}
	layout->addWidget( m_account_group ) ;
	layout->addStretch() ;
	setLayout( layout ) ;

	connect( m_mechanism_combo , SIGNAL(currentIndexChanged(QString)), this, SLOT(mechanismUpdateSlot(QString)) ) ;
	connect( m_port_edit_box , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()) ) ;
	connect( m_server_edit_box , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()) ) ;
	connect( m_account_name , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()) ) ;
	connect( m_account_pwd , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()) ) ;
	connect( m_auth_checkbox , SIGNAL(toggled(bool)), this, SIGNAL(pageUpdateSignal()) ) ;
	connect( m_auth_checkbox , SIGNAL(toggled(bool)), this, SLOT(onToggle()) ) ;

	onToggle() ;
}

void SmtpClientPage::onToggle()
{
	m_account_group->setEnabled( m_auth_checkbox->isChecked() ) ;
	m_mechanism_combo->setEnabled( m_auth_checkbox->isChecked() ) ;
}

std::string SmtpClientPage::nextPage()
{
	return next1() ;
}

void SmtpClientPage::dump( std::ostream & stream , const std::string & prefix , const std::string & eol ) const
{
	GPage::dump( stream , prefix , eol ) ;
	stream << prefix << "smtp-client-host: " << value(m_server_edit_box) << eol ;
	stream << prefix << "smtp-client-port: " << value(m_port_edit_box) << eol ;
	stream << prefix << "smtp-client-auth: " << value(m_auth_checkbox) << eol ;
	stream << prefix << "smtp-client-auth-mechanism: " << value(m_mechanism_combo) << eol ;
	stream << prefix << "smtp-client-account-name: " << value(m_account_name) << eol ;
	stream << prefix << "smtp-client-account-password: " << encrypt(value(m_account_pwd),value(m_mechanism_combo)) << eol ;
}

bool SmtpClientPage::isComplete()
{
	return
		! m_port_edit_box->text().isEmpty() &&
		! m_server_edit_box->text().isEmpty() &&
		( ! m_auth_checkbox->isChecked() || (
			! m_account_name->text().isEmpty() &&
			! m_account_pwd->text().isEmpty() ) ) ;
}

// ==

LoggingPage::LoggingPage( GDialog & dialog , const std::string & name ,
	const std::string & next_1 , const std::string & next_2 , bool finish , bool close ) : 
		GPage(dialog,name,next_1,next_2,finish,close)
{
	m_debug_checkbox = new QCheckBox( tr("&Debug messages") ) ;
	m_verbose_checkbox = new QCheckBox( tr("&Verbose logging") ) ;
	m_syslog_checkbox = new QCheckBox( tr("&Write to the system log") ) ;

	QVBoxLayout * logging_layout = new QVBoxLayout ;
	logging_layout->addWidget( m_verbose_checkbox ) ;
	logging_layout->addWidget( m_syslog_checkbox ) ;
	logging_layout->addWidget( m_debug_checkbox ) ;
	m_syslog_checkbox->setChecked(true) ;
	m_debug_checkbox->setEnabled(false) ;

	QGroupBox * logging_group = new QGroupBox(tr("Logging")) ;
	logging_group->setLayout( logging_layout ) ;

	//

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(newTitle(tr("Logging"))) ;
	layout->addWidget(logging_group) ;
	layout->addStretch() ;
	setLayout( layout ) ;
}

std::string LoggingPage::nextPage()
{
	return next1() ;
}

void LoggingPage::dump( std::ostream & stream , const std::string & prefix , const std::string & eol ) const
{
	GPage::dump( stream , prefix , eol ) ;
	stream << prefix << "logging-verbose: " << value(m_verbose_checkbox) << eol ;
	stream << prefix << "logging-debug: " << value(m_debug_checkbox) << eol ;
	stream << prefix << "logging-syslog: " << value(m_syslog_checkbox) << eol ;
}

// ==

ListeningPage::ListeningPage( GDialog & dialog , const std::string & name ,
	const std::string & next_1 , const std::string & next_2 , bool finish , bool close ) : 
		GPage(dialog,name,next_1,next_2,finish,close)
{
	m_listening_interface = new QLineEdit ;
	m_all_radio = new QRadioButton(tr("&All interfaces")) ;
	m_one_radio = new QRadioButton(tr("&One")) ;
	QLabel * listening_interface_label = new QLabel(tr("&Interface:")) ;
	listening_interface_label->setBuddy( m_listening_interface ) ;
	m_all_radio->setChecked(true) ;
	m_listening_interface->setEnabled(false) ;

	QGridLayout * listening_layout = new QGridLayout ;
	listening_layout->addWidget( m_all_radio , 0 , 0 ) ;
	listening_layout->addWidget( m_one_radio , 1 , 0 ) ;
	listening_layout->addWidget( listening_interface_label , 1 , 1 ) ;
	listening_layout->addWidget( m_listening_interface , 1 , 2 ) ;

	QGroupBox * listening_group = new QGroupBox(tr("Listen on")) ;
	listening_group->setLayout( listening_layout ) ;

	//

	m_remote_checkbox = new QCheckBox(tr("&Allow remote clients")) ;

	QHBoxLayout * connections_layout = new QHBoxLayout ;
	connections_layout->addWidget( m_remote_checkbox ) ;

	QGroupBox * connections_group = new QGroupBox(tr("Clients")) ;
	connections_group->setLayout( connections_layout ) ;

	//

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(newTitle(tr("Connections"))) ;
	layout->addWidget(listening_group) ;
	layout->addWidget(connections_group) ;
	layout->addStretch() ;
	setLayout( layout ) ;

	if( testMode() )
	{
		m_one_radio->setChecked(true) ;
		m_all_radio->setChecked(false) ;
		m_listening_interface->setEnabled(true) ;
		m_listening_interface->setText("192.168.1.0") ;
	}

	connect( m_all_radio , SIGNAL(toggled(bool)) , this , SLOT(onToggle()) ) ;
	connect( m_all_radio , SIGNAL(toggled(bool)) , this , SIGNAL(pageUpdateSignal()) ) ;
	connect( m_one_radio , SIGNAL(toggled(bool)) , this , SLOT(onToggle()) ) ;
	connect( m_one_radio , SIGNAL(toggled(bool)) , this , SIGNAL(pageUpdateSignal()) ) ;
	connect( m_listening_interface , SIGNAL(textChanged(QString)), this, SIGNAL(pageUpdateSignal()) ) ;

	onToggle() ;
}

std::string ListeningPage::nextPage()
{
	return next1() ;
}

void ListeningPage::onToggle()
{
	m_listening_interface->setEnabled( ! m_all_radio->isChecked() ) ;
}

bool ListeningPage::isComplete()
{
	G_DEBUG( "ListeningPage::isComplete" ) ;
	return
		m_all_radio->isChecked() || 
		!m_listening_interface->text().isEmpty() ;
}

void ListeningPage::dump( std::ostream & stream , const std::string & prefix , const std::string & eol ) const
{
	GPage::dump( stream , prefix , eol ) ;
	stream << prefix << "listening-all: " << value(m_all_radio) << eol ;
	stream << prefix << "listening-interface: " << value(m_listening_interface) << eol ;
	stream << prefix << "listening-remote: " << value(m_remote_checkbox) << eol ;
}

// ==

StartupPage::StartupPage( GDialog & dialog , const std::string & name ,
	const std::string & next_1 , const std::string & next_2 , bool finish , bool close ) : 
		GPage(dialog,name,next_1,next_2,finish,close)
{
	m_on_boot_checkbox = new QCheckBox( tr("At &system startup") ) ;
	m_at_login_checkbox = new QCheckBox( tr("&When logging in") ) ;
	QVBoxLayout * auto_layout = new QVBoxLayout ;
	auto_layout->addWidget( m_on_boot_checkbox ) ;
	auto_layout->addWidget( m_at_login_checkbox ) ;

	if( ! testMode() )
		m_on_boot_checkbox->setEnabled(false) ;

	m_add_menu_item_checkbox = new QCheckBox( tr("Add to start menu") ) ;
	m_add_desktop_item_checkbox = new QCheckBox( tr("Add to desktop") ) ;
	QVBoxLayout * manual_layout = new QVBoxLayout ;
	manual_layout->addWidget( m_add_menu_item_checkbox ) ;
	manual_layout->addWidget( m_add_desktop_item_checkbox ) ;
	m_add_menu_item_checkbox->setChecked(true) ;

	QGroupBox * auto_group = new QGroupBox(tr("Automatic")) ;
	auto_group->setLayout( auto_layout ) ;

	QGroupBox * manual_group = new QGroupBox(tr("Manual")) ;
	manual_group->setLayout( manual_layout ) ;

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(newTitle(tr("Server startup"))) ;
	layout->addWidget(auto_group) ;
	layout->addWidget(manual_group) ;
	layout->addStretch() ;
	setLayout( layout ) ;
}

std::string StartupPage::nextPage()
{
	return next1() ;
}

void StartupPage::dump( std::ostream & stream , const std::string & prefix , const std::string & eol ) const
{
	GPage::dump( stream , prefix , eol ) ;
	stream << prefix << "start-on-boot: " << value(m_on_boot_checkbox) << eol ;
	stream << prefix << "start-at-login: " << value(m_at_login_checkbox) << eol ;
	stream << prefix << "start-link-menu: " << value(m_add_menu_item_checkbox) << eol ;
	stream << prefix << "start-link-desktop: " << value(m_add_desktop_item_checkbox) << eol ;
}

// ==

ConfigurationPage::ConfigurationPage( GDialog & dialog , const std::string & name , const std::string & next_1 , 
	const std::string & next_2 , bool finish , bool close ) :
		GPage(dialog,name,next_1,next_2,finish,close)
{
	m_label = new QLabel( text() ) ;

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(newTitle(tr("Ready to install"))) ;
	layout->addWidget(m_label);
	setLayout(layout);
}

void ConfigurationPage::onShow( bool )
{
}

QString ConfigurationPage::text() const
{
	return GPage::tr(
		"<center>"
		"<p>Your configuration will now be saved to \"install.cfg\"</p>"
		"<p>and processed by \"emailrelay-install-tool\"</p>"
		"</center>" ) ;
}

std::string ConfigurationPage::nextPage()
{
	return next1() ;
}

void ConfigurationPage::dump( std::ostream & , const std::string & , const std::string & ) const
{
	// no-op
}

// ==

ProgressPage::ProgressPage( GDialog & dialog , const std::string & name ,
	const std::string & next_1 , const std::string & next_2 , bool finish , bool close ) :
		GPage(dialog,name,next_1,next_2,finish,close) ,
		m_thread(NULL) ,
		m_rc(1)
{
	m_text_edit = new QTextEdit;
	m_text_edit->setReadOnly(true) ;
	m_text_edit->setWordWrapMode(QTextOption::NoWrap) ;
	m_text_edit->setLineWrapMode(QTextEdit::NoWrap) ;
	m_text_edit->setFontFamily("courier") ;

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(newTitle(tr("Installing"))) ;
	layout->addWidget(m_text_edit) ;
	setLayout( layout ) ;
}

void ProgressPage::onShow( bool back )
{
	if( ! back )
	{
		// write the config file
		G::Path config_file( Dir::thisdir() , "install.cfg" ) ; // TODO -- try other directories on error
		std::ofstream file( config_file.str().c_str() ) ;
		dialog().dump( file ) ;
		file.flush() ;
		bool file_good = file.good() ; // TODO -- error handling

		// spawn the thread that spawns the process and feeds its output back
		check( m_thread == NULL ) ;
		m_thread = new Thread( tool() , toolArgs() ) ;
		connect( m_thread, SIGNAL(changeSignal()), this, SLOT(onThreadChangeEvent()), Qt::QueuedConnection ) ;
		connect( m_thread, SIGNAL(doneSignal(int)), this, SLOT(onThreadDoneEvent(int)), Qt::QueuedConnection ) ;
		dialog().wait(true) ;
		m_thread->start() ;
	}
}

void ProgressPage::onThreadChangeEvent()
{
	G_DEBUG( "ProcessPage::onThreadChangeEvent" ) ;
	check( m_thread != NULL ) ;
	QString text = m_thread->text() ;
	m_text_edit->setFontFamily("courier") ;
	m_text_edit->setPlainText( text ) ;
}

void ProgressPage::onThreadDoneEvent( int rc )
{
	G_DEBUG( "ProcessPage::onThreadDoneEvent: " << rc ) ;
	m_rc = rc ;
	check( m_thread != NULL ) ;
	bool timeout = ! m_thread->wait( 2000U ) ;
	Thread * p = timeout ? NULL : m_thread ; // on timeout leave the thread dangling and move on
	m_thread = NULL ;
	delete p ;
	dialog().wait(false) ;
}

std::string ProgressPage::nextPage()
{
	return next1() ;
}

void ProgressPage::dump( std::ostream & , const std::string & , const std::string & ) const
{
	// no-op
}

void ProgressPage::check( bool ok )
{
	if( !ok )
		throw std::runtime_error("internal error") ;
}

bool ProgressPage::closeButton() const
{
	bool result = m_rc == 0 ? GPage::closeButton() : false ;
	G_DEBUG( "ProgressPage::closeButton: " << result ) ;
	return result ;
}

// ==

EndPage_::EndPage_( GDialog & dialog , const std::string & name ) :
	GPage(dialog,name,"","",true,true)
{
	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(newTitle(tr("Finish"))) ;
	layout->addStretch() ;
	setLayout( layout ) ;
}

std::string EndPage_::nextPage()
{
	return std::string() ;
}

void EndPage_::dump( std::ostream & , const std::string & , const std::string & ) const
{
	// no-op
}


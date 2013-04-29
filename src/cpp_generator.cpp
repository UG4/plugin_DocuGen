/**
 * \file apps/ugdocu/src/cpp_generator.cpp
 * \author Torbjörn Klatt
 * \date 2013-04-23
 */

#include "cpp_generator.h"

#include "registry/class_helper.h"
#ifdef UG_BRIDGE
# include "bridge/bridge.h"
#endif
#include "registry/class_helper.h"

#include "common/log.h"
#include "common/assert.h"
#include "common/error.h"

#include <iostream>	// std::fstream
#include <sstream>	// std::stringstream
#include <boost/algorithm/string.hpp>	// boost::replace_all, boost::split, boost::to_lower, boost::trim

namespace ug
{
namespace regDocu
{

using namespace std;

/**
 * \brief Organization unit for Doxygen commands
 */
namespace Doxygen {
	static const string DEFGROUP = "/// \\defgroup ";
	static const string ADDTOGROUP = "/// \\addtogroup ";
	static const string INGROUP = "/// \\ingroup ";
	static const string GROUP_OPEN = "/// \\{\n";
	static const string GROUP_CLOSE = "/// \\}\n";
	static const string CLASS = "/// \\class ";
	static const string BRIEF = "/// \\brief ";
	static const string DETAILS = "/// \\details ";
	static const string SEE = "/// \\see ";
	static const string PARAM_IN = "/// \\param[in] ";
	static const string RETURNS = "/// \\returns ";
}

CppGenerator::CppGenerator( const string dir, bool silent ) :
	  m_output_dir( dir )
	, mr_reg( bridge::GetUGRegistry() )
	, m_curr_class( NULL )
	, m_curr_group( NULL )
	, m_curr_group_name( "" )
	, m_written_classes()
	, m_silent( silent )
{}

CppGenerator::~CppGenerator()
{
	if ( m_curr_file.is_open() ) m_curr_file.close();
}

void CppGenerator::generate_cpp_files()
{
	m_curr_file.open( string( m_output_dir ).append( "regdocu.doxygen" ).c_str(), ios::out );
	m_curr_file << Doxygen::ADDTOGROUP << "regdocu UGRegistry Documentation" << endl
	            << Doxygen::BRIEF << "This module holds documentation for all registered functions and classes of libug4." << endl;
	m_curr_file.close();
	
	UG_LOG( "Generating CPP files for " << mr_reg.num_class_groups() << " class groups ..." << endl );
	for ( size_t i_class_group = 0; i_class_group < mr_reg.num_class_groups(); ++i_class_group ) {
		m_curr_group = mr_reg.get_class_group( i_class_group );
		generate_class_group();
	}
	
	UG_LOG( "Generating CPP files for ungrouped classes ..." << endl );
	size_t count_new_classes = 0;
	for ( size_t i_class = 0; i_class < mr_reg.num_classes(); ++i_class ) {
		m_curr_class = mr_reg.get_class( mr_reg.get_class( i_class ).name() );
		if ( m_written_classes.count( m_curr_class->name() ) == 0 ) {
			string class_id = name_to_id( m_curr_class->name() );
			string file_name = string( m_output_dir ).append( class_id ).append( ".cpp" );
			m_curr_file.open( file_name.c_str(), ios::out );
			
			m_curr_group_name = string( "ug::" ).append( m_curr_class->name() );
			if ( !m_silent ) {
				UG_LOG( "  Writing class " << m_curr_class->name() << " to '" 
				        << file_name << "'." << endl );
			}
			m_curr_file << "namespace ug4bridge {" << endl
			// add this class group into the global regdocu group
			            << Doxygen::ADDTOGROUP << "regdocu" << endl
			            << Doxygen::GROUP_OPEN << endl;
			generate_class_docu();
			generate_class_cpp();
			m_curr_file << Doxygen::GROUP_CLOSE << endl
			            << "}" << endl;
			m_written_classes.insert( make_pair( m_curr_class->name(), file_name ) );
			m_curr_group_name = "";
			m_curr_file.close();
			++count_new_classes;
		} else {
			if ( !m_silent ) {
				UG_LOG( "  Class '" << m_curr_class->name() << "' already written to '" 
				        << m_written_classes.at( m_curr_class->name() ) << "'." << endl );
			}
		}
	}
	UG_LOG( count_new_classes << " additional classes written." << endl );
	
	UG_LOG( "Generating CPP for " << mr_reg.num_functions() << " global functions ..." << endl );
	generate_global_functions();
}

void CppGenerator::generate_class_group()
{
	m_curr_group_name = string( "ug::" ).append( m_curr_group->name() );
	string group_id = name_to_id( m_curr_group->name() );
	
	if ( group_id.empty() ) {
		UG_WARNING( "Empty group_id for group_name '" << m_curr_group_name << "'" << endl );
		return;
	}
	
	string file_name = string( m_output_dir ).append( group_id ).append( ".cpp" );
	if ( !m_silent ) {
		UG_LOG( "  Writing group '" << m_curr_group_name << "' to " << file_name << endl );
	}
	m_curr_file.open( file_name.c_str(), ios::out );
	
	// let's take the default class
	m_curr_class = m_curr_group->get_default_class();
	
	// setup namespace hierarchy
	string namespace_closing = "";
	if ( m_curr_class->class_names()->size() > 1 ) {
		namespace_closing = write_group_namespaces( split_group_hieararchy( name_to_id( m_curr_class->group() ) ) );
	}
	
	
	// BEGIN class group definition
	m_curr_file << Doxygen::DEFGROUP << group_id << " " << m_curr_group->name() << endl;
	// in case this group contains several specializations, display the default
	if ( m_curr_group->get_default_class() != NULL ) {
		m_curr_file << Doxygen::BRIEF << "default specialization is " << m_curr_group->get_default_class()->name() << endl;
	}
	// name and display the registered ug4 class
	m_curr_file << Doxygen::SEE << m_curr_group_name << endl;
	// add this class group into the global regdocu group
	m_curr_file << Doxygen::INGROUP << "regdocu" << endl
	            << Doxygen::GROUP_OPEN;
	// END class group definition
	
	// BEGIN classes of this group
	for( size_t class_num = 0; class_num < m_curr_group->num_classes(); ++class_num ) {
		m_curr_class = m_curr_group->get_class( class_num );
		generate_class_docu();
		generate_class_cpp();
		m_written_classes.insert( make_pair( m_curr_class->name(), file_name ) );
	}
	// END classes of this group
	
	// close class group definition
	m_curr_file << Doxygen::GROUP_CLOSE << endl;
	
	// Close namespaces
	m_curr_file << namespace_closing;
	
	// resetting current group
	m_curr_group = NULL;
	m_curr_group_name = "";
	
	m_curr_file.close();
}

void CppGenerator::generate_global_functions()
{
	string file_name = string( m_output_dir ).append( "global_functions.cpp" );
	UG_LOG( "Writing global functions to " << file_name << endl );
	m_curr_file.open( file_name.c_str(), ios::out );
	
	// add this class group into the global regdocu group
	m_curr_file << Doxygen::ADDTOGROUP << "regdocu" << endl
	            << Doxygen::GROUP_OPEN << endl;
	
	string namespace_closing = "";
	for ( size_t i_global_function = 0; i_global_function < mr_reg.num_functions(); ++i_global_function ) {
		bridge::ExportedFunction curr_func = mr_reg.get_function( i_global_function );
		
		// setup namespace hierarchy
		namespace_closing = "";
		if ( m_curr_class->class_names()->size() > 1 ) {
			namespace_closing = write_group_namespaces( split_group_hieararchy( name_to_id( curr_func.group() ) ) );
		}
		
		write_generic_function( curr_func );
		
		m_curr_file << namespace_closing << endl;
	}
	
	// end global group regdocu
	m_curr_file << Doxygen::GROUP_CLOSE << endl;
	
	m_curr_file.close();
}

void CppGenerator::generate_class_docu()
{
	if ( m_curr_file.is_open() ) {
		m_curr_file << endl << Doxygen::CLASS << m_curr_class->name() << endl;
		// tooltip
		if ( m_curr_class->tooltip().size() > 0 ) {
			m_curr_file << Doxygen::BRIEF << m_curr_class->tooltip() << endl;
		}
	} else {
		UG_WARNING( "File not open." );
	}
}

void CppGenerator::generate_class_cpp()
{
	if ( m_curr_file.is_open() ) {
		m_curr_file << "class " << m_curr_class->name();
		if ( m_curr_class->class_name_node().num_base_classes() > 0 ) {
			m_curr_file << " : public " << m_curr_class->class_name_node().base_class(0).name();
		}
		m_curr_file << " {" << endl;
		generate_class_constructors();
		generate_class_public_methods();
		generate_class_public_members();
		m_curr_file << "};" << endl;
	} else {
		UG_WARNING( "File not open." );
	}
}

void CppGenerator::generate_class_constructors()
{
	if ( m_curr_class->is_instantiable() ) {
		m_curr_file << endl << "public:" << endl;
		for( size_t i_ctor = 0; i_ctor < m_curr_class->num_constructors(); ++i_ctor ) {
			const bridge::ExportedConstructor ctor = m_curr_class->get_constructor( i_ctor );
			
			// constructor docu
			write_brief_detail_docu( ctor );
			
			// input parameter docu
			write_parameter_docu( ctor );
			
			m_curr_file << m_curr_class->name();
			
			string param_list = generate_parameter_list( ctor );
			m_curr_file << param_list << ";" << endl;
		}
	} else {
		m_curr_file << endl << "private:" << endl
		            << Doxygen::BRIEF << "Constructor hidden / deactivated" << endl
		            << m_curr_class->name() << "()=delete;" << endl;
	}
}

void CppGenerator::generate_class_public_methods()
{
	m_curr_file << endl << "public:" << endl;
	size_t num_methods = m_curr_class->num_methods();
	size_t num_const_methods = m_curr_class->num_const_methods();
	
	// non-const methods
	for ( size_t i_method = 0; i_method < num_methods; ++i_method ) {
		const bridge::ExportedMethod method = m_curr_class->get_method( i_method );
		write_generic_function( method );
	}
	
	// const methods
	for ( size_t i_const_method = 0; i_const_method < num_const_methods; ++i_const_method ) {
		const bridge::ExportedMethod const_method = m_curr_class->get_const_method( i_const_method );
		write_generic_function( const_method, true );
	}
}

void CppGenerator::generate_class_public_members()
{
	m_curr_file << endl << "public:" << endl;
	//TODO implement handling of public member variables of registered classes (if applicable)
}

template< class TEntity >
void CppGenerator::write_brief_detail_docu( const TEntity &class_function ) {
	m_curr_file << Doxygen::BRIEF << class_function.tooltip() << endl
	            << Doxygen::DETAILS << class_function.help() << endl;
}

template< class TFunction >
void CppGenerator::write_parameter_docu( const TFunction &function )
{
	for( size_t i_param_in = 0; i_param_in < function.num_parameter(); ++i_param_in ) {
		m_curr_file << Doxygen::PARAM_IN
		            << sanitize_parameter_name( function.parameter_name( i_param_in ) ) << endl;
	}
}

template< class TFunction >
void CppGenerator::write_generic_function( const TFunction &function, bool constant )
{
	// method docu
	write_brief_detail_docu( function );
	
	// display and link registered function
	string registered_function_name = "";
	if ( !m_curr_group_name.empty() ) {
		registered_function_name = registered_function_name.append( m_curr_group_name ).append( "::" );
	}
	registered_function_name = registered_function_name.append( function.name() );
	
	// input parameter docu
	write_parameter_docu( function );
	
	// write return value docu
	string return_type = generate_return_value( function );
	string param_list = generate_parameter_list( function );
	
	// method signature
	m_curr_file << return_type << " " << function.name()
	            << param_list;
	if ( constant ) m_curr_file << " const";
	m_curr_file << " { " << registered_function_name
	            << param_list << "; }" << endl;
}

template< class TFunction >
string CppGenerator::generate_parameter_list( const TFunction &func )
{
	stringstream param_list;
	param_list << "(";
	if ( func.num_parameter() > 0 ) {
		size_t i_last_param = func.num_parameter() - 1;
		for( size_t i_param = 0; i_param < i_last_param; ++i_param ) {
			param_list << bridge::ParameterToString( func.params_in(), i_param )
			           << " " << sanitize_parameter_name( func.parameter_name( i_param ) ) << ", ";
		}
		param_list << bridge::ParameterToString( func.params_in(), i_last_param )
		           << " " << sanitize_parameter_name( func.parameter_name( i_last_param ) );
	}
	param_list << ")";
	return param_list.str();
}

string CppGenerator::generate_return_value( const bridge::ExportedFunctionBase &method )
{
	const bridge::ParameterInfo param_out = method.params_out();
	if ( param_out.size() == 1 ) {
		// exactly one return value
		m_curr_file << Doxygen::RETURNS << sanitize_docu( method.return_info(0) ) << endl;
		return bridge::ParameterToString( param_out, 0 );
	} else if ( param_out.size() > 1 ) {
		// more than one return value
		//TODO implement handling of multiple return values
		UG_WARNING( "Multiple return values not yet implemented in C++Generator."
		            << " Displaying as '()'." );
		return "()";
	} else {
		// no return value (i.e. void)
		return string("void");
	}
}

string CppGenerator::name_to_id( const string &str )
{
	string id = str;
	boost::trim( id );
	boost::replace_all( id, " ", "");
	boost::replace_all( id, "/", "_");
	boost::to_lower( id );
	return id;
}

string CppGenerator::sanitize_parameter_name( const string &param ) const
{
	string sanitized = ( param.empty() ) ? "unnamed" : param;
	boost::trim( sanitized );
	boost::replace_all( sanitized, " ", "_" );
	return sanitized;
}

string CppGenerator::sanitize_docu( const string &param_docu ) const
{
	return ( param_docu.empty() ) ? "undocumented" : param_docu;
}

vector< string > CppGenerator::split_group_hieararchy( const string group )
{
	vector<string> namespaces;
	boost::split( namespaces, group, boost::is_any_of("_/"), boost::algorithm::token_compress_on );
	vector<string>::iterator plugin = find( namespaces.begin(), namespaces.end(), "(plugin)ug4" );
	if ( plugin != namespaces.end() ) {
		(*plugin) = "plugins";
		namespaces.insert( namespaces.begin(), "ug4bridge" );
	}
	return namespaces;
}

string CppGenerator::write_group_namespaces( vector<string> group_hierarchy )
{
	size_t count_namespaces = 0;
	for ( vector<string>::iterator iter = group_hierarchy.begin();
		iter != group_hierarchy.end(); ++iter ) {
		if ( ! (*iter).empty() ) {
			if ( (*iter).compare( "ug4" ) == 0 ) (*iter) = "ug4bridge";
			m_curr_file << "namespace " << *iter << " {" << endl;
			++count_namespaces;
		}
	}
	
	stringstream closing;
	for ( size_t i_namespace = 0; i_namespace < count_namespaces; ++i_namespace ) {
		closing << "}" << endl;
	}
	
	return closing.str();
}

}	// end namespace ugdocu
}	// end namespace ug

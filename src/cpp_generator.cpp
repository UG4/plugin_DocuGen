/*
 * Copyright (c) 2013-2016:  G-CSC, Goethe University Frankfurt
 * Author: Torbjörn Klatt
 * 
 * This file is part of UG4.
 * 
 * UG4 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License version 3 (as published by the
 * Free Software Foundation) with the following additional attribution
 * requirements (according to LGPL/GPL v3 §7):
 * 
 * (1) The following notice must be displayed in the Appropriate Legal Notices
 * of covered and combined works: "Based on UG4 (www.ug4.org/license)".
 * 
 * (2) The following notice must be displayed at a prominent place in the
 * terminal output of covered works: "Based on UG4 (www.ug4.org/license)".
 * 
 * (3) The following bibliography is recommended for citation and must be
 * preserved in all covered files:
 * "Reiter, S., Vogel, A., Heppner, I., Rupp, M., and Wittum, G. A massively
 *   parallel geometric multigrid solver on hierarchically distributed grids.
 *   Computing and visualization in science 16, 4 (2013), 151-164"
 * "Vogel, A., Reiter, S., Rupp, M., Nägel, A., and Wittum, G. UG4 -- a novel
 *   flexible software system for simulating pde based models on high performance
 *   computers. Computing and visualization in science 16, 4 (2013), 165-179"
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
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
namespace DocuGen
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
	static const string NOTE = "/// \\note ";
	static const string WARNING = "/// \\warning ";
	static const string SEE = "/// \\see ";
	static const string PARAM_IN = "/// \\param[in] ";
	static const string RETURNS = "/// \\returns ";
}

CppGenerator::CppGenerator( const string dir, ClassHierarchyProvider &chp, bool silent ) :
	  m_output_dir( dir )
	, mr_reg( bridge::GetUGRegistry() )
	, mr_chp( chp )
	, m_curr_class( NULL )
	, m_curr_group( NULL )
	, m_curr_group_name( "" )
	, m_is_plugin( false )
	, m_is_global( false )
	, m_written_classes()
	, m_silent( silent )
{}

CppGenerator::~CppGenerator()
{
	if ( m_curr_file.is_open() ) m_curr_file.close();
}

void CppGenerator::generate_cpp_files()
{
	try{
	UG_LOG( "Generating CPP files for " << mr_reg.num_class_groups() << " class groups ..." << endl );
	for ( size_t i_class_group = 0; i_class_group < mr_reg.num_class_groups(); ++i_class_group ) {
		m_curr_group = mr_reg.get_class_group( i_class_group );
		generate_class_group();
	}
	
	UG_LOG( "Generating CPP files for ungrouped classes ..." << endl );
	size_t count_new_classes = 0;
	for ( size_t i_class = 0; i_class < mr_reg.num_classes(); ++i_class ) {
		m_curr_class = mr_reg.get_class( mr_reg.get_class( i_class ).name() );
		string trimmed_class_name = mr_chp.get_group( m_curr_class->name() );
		if ( m_written_classes.count( trimmed_class_name ) == 0 ) {
			string class_id = name_to_id( trimmed_class_name );
			string file_name = string( m_output_dir ).append( class_id ).append( ".cpp" );
			m_curr_file.open( file_name.c_str(), ios::out );
			
			m_curr_group_name = string( "ug::" ).append( trimmed_class_name );
			if ( !m_silent ) {
				UG_LOG( "  Writing class " << trimmed_class_name << " to '" 
				        << file_name << "'." << endl );
			}

			// setup namespace hierarchy
			string namespace_group_closing = write_group_namespaces( split_group_hieararchy( m_curr_class->group() ) );

			generate_class_docu();
			generate_class_cpp();
			
			m_curr_file << namespace_group_closing;
			
			m_written_classes.insert( make_pair( trimmed_class_name, file_name ) );
			m_curr_group_name = "";
			m_curr_file.close();
			++count_new_classes;
		} else {
			if ( !m_silent ) {
				UG_LOG( "  Class '" << trimmed_class_name << "' already written to '" 
				        << m_written_classes.at( trimmed_class_name ) << "'." << endl );
			}
		}
	}
	UG_LOG( count_new_classes << " additional classes written." << endl );
	
	UG_LOG( "Generating CPP for " << mr_reg.num_functions() << " global functions ..." << endl );
	generate_global_functions();
	
	UG_LOG( "Writing Doxygen group definitions ..." << endl );
	write_group_definitions();
	}UG_CATCH_THROW_FUNC();
}

void CppGenerator::generate_class_group()
{
	try{
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
	
	// we only consider the default class
	m_curr_class = m_curr_group->get_default_class();
	if(m_curr_class  == NULL) return;
	
	// setup namespace and groups hierarchy
	string namespace_group_closing = write_group_namespaces( split_group_hieararchy( m_curr_class->group() ) );
	
	// write class to file
	generate_class_docu();
	if ( m_curr_group->get_default_class() != NULL ) {
		m_curr_file << Doxygen::NOTE << "default specialization is " << m_curr_group->get_default_class()->name() << endl;
	}
	m_curr_file << Doxygen::SEE << m_curr_group_name << endl;
	generate_class_cpp();
	m_written_classes.insert( make_pair( mr_chp.get_group( m_curr_class->name() ), file_name ) );
	
	// Close namespaces and groups
	m_curr_file << namespace_group_closing;
	
	// resetting current group
	m_curr_group = NULL;
	m_curr_group_name = "";
	m_is_plugin = false;
	
	m_curr_file.close();
	}UG_CATCH_THROW_FUNC();
}

void CppGenerator::generate_global_functions()
{
	try{
	string file_name = string( m_output_dir ).append( "global_functions.cpp" );
	UG_LOG( "Writing global functions to " << file_name << endl );
	m_curr_file.open( file_name.c_str(), ios::out );
	
	m_is_global = true;
	
	string namespace_group_closing;
	for ( size_t i_global_function = 0; i_global_function < mr_reg.num_functions(); ++i_global_function ) {
		bridge::ExportedFunction curr_func = mr_reg.get_function( i_global_function );
		
		// setup namespace and groups hierarchy
		namespace_group_closing = write_group_namespaces( split_group_hieararchy( curr_func.group() ), true );
		
		write_generic_function( curr_func );
		
		// close namespaces and groups
		m_curr_file << namespace_group_closing << endl;
		
		m_is_plugin = false;
	}
	
	m_is_global = false;
	
	m_curr_file.close();
	}UG_CATCH_THROW_FUNC();
}

void CppGenerator::write_group_definitions()
{
	try{
	m_curr_file.open( string( m_output_dir ).append( "regdocu.doxygen" ).c_str(), ios::out );
	
	m_curr_file << Doxygen::BRIEF << "This namespace holds documentation for all registered functions and classes of libug4." << endl
	            << "namespace ug4Bridge {" << endl
	            << Doxygen::BRIEF << "This namespace holds documentation for all registered plugins of libug4." << endl
	            << Doxygen::WARNING << "The members of this namespace require certain compile-time parameters!" << endl
	            << "namespace Plugins {" << endl
	            << "}" << endl << "}" << endl
	            << endl;
	
	m_curr_file.close();
	}UG_CATCH_THROW_FUNC();
}

void CppGenerator::generate_class_docu()
{
	try{
	if ( m_curr_file.is_open() ) {
		m_curr_file << endl << Doxygen::CLASS << mr_chp.get_group( m_curr_class->name() ) << endl;
		// tooltip
		if ( ! m_curr_class->tooltip().empty() ) {
			m_curr_file << Doxygen::BRIEF << m_curr_class->tooltip() << endl;
		}
		if ( m_is_plugin ) {
			m_curr_file << Doxygen::WARNING << "This class is part of a plugin. "
			            << "Special compile-time parameters are required for this." << endl;
		}
	} else {
		UG_WARNING( "File not open." );
	}

	}UG_CATCH_THROW_FUNC();
}

void CppGenerator::generate_class_cpp()
{
	try{

	if ( m_curr_file.is_open() ) {
		m_curr_file << "class " << mr_chp.get_group( m_curr_class->name() );
		if ( m_curr_class->class_name_node().num_base_classes() > 0 ) {
			m_curr_file << " : public " << mr_chp.get_group( m_curr_class->class_name_node().base_class(0).name() );
		}
		m_curr_file << " {" << endl;
		generate_class_constructors();
		generate_class_public_methods();
		generate_class_public_members();
		m_curr_file << "};" << endl;
	} else {
		UG_WARNING( "File not open." );
	}

	}UG_CATCH_THROW_FUNC();
}

void CppGenerator::generate_class_constructors()
{
	try{
	if ( m_curr_class->is_instantiable() ) {
		m_curr_file << endl << "public:" << endl;
		for( size_t i_ctor = 0; i_ctor < m_curr_class->num_constructors(); ++i_ctor ) {
			const bridge::ExportedConstructor ctor = m_curr_class->get_constructor( i_ctor );
			
			// constructor docu
			write_brief_detail_docu( ctor );
			
			// input parameter docu
			write_parameter_docu( ctor );
			
			m_curr_file << mr_chp.get_group( m_curr_class->name() );
			
			string param_list = generate_parameter_list( ctor );
			m_curr_file << param_list << ";" << endl;
		}
	} else {
		m_curr_file << endl << "private:" << endl
		            << Doxygen::BRIEF << "Constructor hidden / deactivated" << endl
		            << mr_chp.get_group( m_curr_class->name() ) << "()=delete;" << endl;
	}
	}UG_CATCH_THROW_FUNC();
}

void CppGenerator::generate_class_public_methods()
{
	try{
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
	}UG_CATCH_THROW_FUNC();
}

void CppGenerator::generate_class_public_members()
{
	//TODO implement handling of public member variables of registered classes (if applicable)
// 	m_curr_file << endl << "public:" << endl;
}

template< class TEntity >
void CppGenerator::write_brief_detail_docu( const TEntity &class_function ) {
	if ( ! sanitize_docstring( class_function.tooltip(), true ).empty() ) {
		m_curr_file << Doxygen::BRIEF << sanitize_docstring( class_function.tooltip(), true ) << endl;
	}
	if ( ! sanitize_docstring( class_function.help() ).empty() ) {
		m_curr_file << Doxygen::DETAILS << sanitize_docstring( class_function.help() ) << endl;
	}
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
	try{
	// method docu
	write_brief_detail_docu( function );
	
	if ( m_is_global && m_is_plugin ) {
		m_curr_file << Doxygen::WARNING << "This function is part of a plugin. "
		            << "Special compile-time parameters are required for this." << endl;
	}
	
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
	}UG_CATCH_THROW_FUNC();
}

template< class TFunction >
string CppGenerator::generate_parameter_list( const TFunction &func )
{
	try{
	stringstream param_list;
	param_list << "(";
	if ( func.num_parameter() > 0 ) {
		size_t i_last_param = func.num_parameter() - 1;
		for( size_t i_param = 0; i_param < i_last_param; ++i_param ) {
			param_list << parameter_to_string( func.params_in(), i_param )
			           << " " << sanitize_parameter_name( func.parameter_name( i_param ) ) << ", ";
		}
		param_list << parameter_to_string( func.params_in(), i_last_param )
		           << " " << sanitize_parameter_name( func.parameter_name( i_last_param ) );
	}
	param_list << ")";
	return param_list.str();

	}UG_CATCH_THROW_FUNC(); return "";
}

string CppGenerator::generate_return_value( const bridge::ExportedFunctionBase &method )
{
	try{
	const bridge::ParameterInfo param_out = method.params_out();
	if ( param_out.size() == 1 ) {
		// exactly one return value
		if ( !sanitize_docu( method.return_info(0) ).empty() ) {
			m_curr_file << Doxygen::RETURNS << sanitize_docu( method.return_info(0) ) << endl;
		}
		return parameter_to_string( param_out, 0 );
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

	}UG_CATCH_THROW_FUNC(); return "";
}

string CppGenerator::name_to_id( const string &str )
{
	string id = str;
	boost::trim( id );
	boost::replace_all( id, " ", "");
	boost::replace_all( id, "/", "_");
	return id;
}

string CppGenerator::parameter_to_string( const bridge::ParameterInfo &par, const int i ) const
{
	string res = string( "" );
	bool is_vector = par.is_vector(i);
	
	if ( is_vector ) res.append( "std::vector< " );
	
	switch( par.type(i) ) {
		default:
			// Fall through to invalid
		case Variant::VT_INVALID:
			res.append( "unknown" );
			break;
		case Variant::VT_BOOL:
			res.append( "bool" );
			break;
		case Variant::VT_SIZE_T:
			res.append( "size_t" );
			break;
		case Variant::VT_INT:
			res.append( "int" );
			break;
		case Variant::VT_FLOAT:
			res.append( "float" );
			break;
		case Variant::VT_DOUBLE:
			res.append( "double" );
			break;
		case Variant::VT_CSTRING:
			res.append( "const char*" );
			break;
		case Variant::VT_STDSTRING:
			res.append( "std::string" );
			break;
		case Variant::VT_POINTER:
			res.append( mr_chp.get_group( par.class_name(i) ) ).append( "*" );
			break;
		case Variant::VT_CONST_POINTER:
			res.append( "const " ).append( mr_chp.get_group( par.class_name(i) ) ).append( "*" );
			break;
		case Variant::VT_SMART_POINTER:
			res.append( "SmartPtr<" ).append( mr_chp.get_group( par.class_name(i) ) ).append( ">" );
			break;
		case Variant::VT_CONST_SMART_POINTER:
			res.append( "ConstSmartPtr<" ).append( mr_chp.get_group( par.class_name(i) ) ).append( ">" );
			break;
	}
	if ( is_vector ) res.append( " >" );
	
	return res;
}

string CppGenerator::sanitize_parameter_name( const string &param ) const
{
	string sanitized = ( param.empty() ) ? "unnamed" : param;
	boost::trim( sanitized );
	boost::replace_all( sanitized, " ", "_" );
	return sanitized;
}

string CppGenerator::sanitize_docstring( const string &docstring, bool is_brief )
{
	string sanitized = "";
	if ( docstring.empty() ) {
// 		sanitized = ( is_brief ) ? "no brief documentation" : "no documentation";
		sanitized = "";
	} else {
		sanitized = docstring;
	}
	return sanitized;
}

string CppGenerator::sanitize_docu( const string &param_docu ) const
{
// 	return ( param_docu.empty() ) ? "undocumented" : param_docu;
	return ( param_docu.empty() ) ? "" : param_docu;
}

vector< string > CppGenerator::split_group_hieararchy( const string group )
{
	string cleanuped_group = name_to_id( group );
	if ( cleanuped_group.empty() ) {
		cleanuped_group = "ug4";
	}
	vector<string> namespaces;
	boost::split( namespaces, cleanuped_group, boost::is_any_of("_/"), boost::algorithm::token_compress_on );
	vector<string>::iterator ug4 = find( namespaces.begin(), namespaces.end(), "ug4" );
	if ( ug4 != namespaces.end() ) {
		*ug4 = "ug4Bridge";
	}
	for ( vector<string>::iterator plugin = namespaces.begin(); plugin != namespaces.end(); ++plugin ) {
		if ( (*plugin).find( "(Plugin)" ) != string::npos ) {
			(*plugin) = "Plugins";
			namespaces.insert( namespaces.begin(), "ug4Bridge" );
			m_is_plugin = true;
			break;
		}
	}
	return namespaces;
}

string CppGenerator::write_group_namespaces( vector<string> group_hierarchy, bool is_global_func )
{
	stringstream closing;
	
	for ( vector<string>::iterator iter = group_hierarchy.begin();
	      iter != group_hierarchy.end(); ++iter ) {
		if ( ! (*iter).empty() ) {
			if ( (*iter).compare( "ug4" ) == 0 ) {
				m_curr_file << "namespace ug4Bridge {" << endl;
			} else {
				m_curr_file << "namespace " << *iter << " {" << endl;
			}
			closing << "}" << endl;
		}
	}
	
	return closing.str();
}

}	// end namespace ugdocu
}	// end namespace ug

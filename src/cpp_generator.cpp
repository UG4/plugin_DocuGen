/*
 *
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

#include <iostream> // std::fstream
#include <boost/algorithm/string.hpp>

namespace ug
{
namespace regDocu
{

using namespace std;

CppGenerator::CppGenerator( const string dir, bool silent ) :
	  m_output_dir( dir )
	, mr_reg( bridge::GetUGRegistry() )
	, m_curr_class( NULL )
	, m_curr_group( NULL )
	, m_written_classes()
	, m_silent( silent )
{}

CppGenerator::~CppGenerator()
{
	if ( m_curr_file.is_open() ) m_curr_file.close();
}

void CppGenerator::generate_cpp_files()
{
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
			UG_LOG( "  Writing class " << m_curr_class->name() << " to '" << file_name << "'." << endl );
			m_curr_file << "namespace ug4bridge {" << endl;
			generate_class_docu();
			generate_class_cpp();
			m_curr_file << "}" << endl;
			m_written_classes.insert( make_pair( m_curr_class->name(), file_name ) );
			m_curr_file.close();
			++count_new_classes;
		} else {
// 			UG_LOG( "  Class '" << m_curr_class->name() << "' already written to '" m_written_classes.at( m_curr_class->name() ) << "'." << endl );
		}
	}
	UG_LOG( count_new_classes << " additional classes written." << endl );
	
	UG_LOG( "Generating CPP for " << mr_reg.num_functions() << " global functions ..." << endl );
	generate_global_functions();
}

void CppGenerator::generate_class_group()
{
	string group_name = m_curr_group->name();
	string group_id = name_to_id( group_name );
	
	if ( group_id.empty() ) {
		UG_WARNING( "Empty group_id for group_name '" << group_name << "'" << endl );
		return;
	}
	
	string file_name = string( m_output_dir ).append( group_id ).append( ".cpp" );
	if ( !m_silent ) {
		UG_LOG( "  Writing group '" << group_name << "' to " << file_name << endl );
	}
	m_curr_file.open( file_name.c_str(), ios::out );
	
	// let's take the default class
	m_curr_class = m_curr_group->get_default_class();
	
	// setup namespace hierarchy
	size_t num_namespaces = 0;
	if ( m_curr_class->class_names()->size() > 1 ) {
		vector<string> namespaces = split_group_hieararchy( name_to_id( m_curr_class->group() ) );
		num_namespaces = namespaces.size();
		for ( vector<string>::iterator iter = namespaces.begin();
		      iter != namespaces.end(); ++iter ) {
			if ( ! (*iter).empty() ) {
				if ( (*iter).compare( "ug4" ) == 0 ) (*iter) = "ug4bridge"; 
				m_curr_file << "namespace " << *iter << " {" << endl;
			}
		}
	}
	
	// BEGIN class group definition
	m_curr_file << "/// \\defgroup " << group_id << " " << group_name << endl;
	// if this group is inside an organization group, place it into it
	// in case this group contains several specializations, display the default
	if ( m_curr_group->get_default_class() != NULL ) {
		m_curr_file << "/// default class is " << m_curr_group->get_default_class()->name() << endl;
	}
	m_curr_file << "/// \\{" << endl;
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
	m_curr_file << "/// \\}" << endl;
	
	// Close namespaces
	for ( size_t i_namespace = 0; i_namespace < num_namespaces; ++i_namespace ) {
		m_curr_file << "}" << endl;
	}
	
	m_curr_file.close();
}

void CppGenerator::generate_global_functions()
{
	string file_name = string( m_output_dir ).append( "global_functions.cpp" );
	UG_LOG( "Writing global functions to " << file_name << endl );
	m_curr_file.open( file_name.c_str(), ios::out );
	
	m_curr_file << "namespace ug4bridge {" << endl;
	
	for ( size_t i_global_function = 0; i_global_function < mr_reg.num_functions(); ++i_global_function ) {
		bridge::ExportedFunctionBase curr_func = mr_reg.get_function( i_global_function );
		write_generic_function( curr_func );
		m_curr_file << ";" << endl << endl;
	}
	
	m_curr_file << "}" << endl;
	
	m_curr_file.close();
}

void CppGenerator::generate_class_docu()
{
	if ( m_curr_file.is_open() ) {
		m_curr_file << endl << "/// \\class " << m_curr_class->name() << endl;
		// tooltip
		if ( m_curr_class->tooltip().size() > 0 ) {
			m_curr_file << "/// \\brief " << m_curr_class->tooltip() << endl;
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
			
			generate_parameter_list( ctor );
			m_curr_file << ";" << endl;
		}
	} else {
		m_curr_file << endl << "  private:" << endl;
		m_curr_file << "/// \\brief Constructor hidden / deactivated" << endl;
		m_curr_file << m_curr_class->name() << "() {}" << endl;
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
		m_curr_file << ";" << endl << endl;
	}
	
	// const methods
	for ( size_t i_const_method = 0; i_const_method < num_const_methods; ++i_const_method ) {
		const bridge::ExportedMethod const_method = m_curr_class->get_const_method( i_const_method );
		write_generic_function( const_method );
		m_curr_file << " const;" << endl << endl;
	}
}

void CppGenerator::generate_class_public_members()
{
	m_curr_file << endl << "public:" << endl;
}

template< class TEntity >
void CppGenerator::write_brief_detail_docu( const TEntity &class_function ) {
	m_curr_file << "/// \\brief " << class_function.tooltip() << endl;
	m_curr_file << "/// \\details " << class_function.help() << endl;
}

template< class TFunction >
void CppGenerator::write_parameter_docu( const TFunction &function )
{
	for( size_t i_param_in = 0; i_param_in < function.num_parameter(); ++i_param_in ) {
		m_curr_file << "/// \\param[in] " << sanitize_parameter_name( function.parameter_name( i_param_in ) );
		m_curr_file << endl;
	}
}

template< class TFunction >
void CppGenerator::write_generic_function( const TFunction &function )
{
	// method docu
	write_brief_detail_docu( function );
	
	// input parameter docu
	write_parameter_docu( function );
	
	// write return value docu
	string return_type = generate_return_value( function );
	
	// method signature
	m_curr_file << return_type << " " << function.name();
	generate_parameter_list( function );
}

string CppGenerator::generate_return_value( const bridge::ExportedFunctionBase &method )
{
	const bridge::ParameterInfo param_out = method.params_out();
	if ( param_out.size() == 1 ) {
		// exactly one return value
		m_curr_file << "/// \\returns " << sanitize_docu( method.return_info(0) ) << endl;
		return bridge::ParameterToString( param_out, 0 );
	} else if ( param_out.size() > 1 ) {
		// more than one return value
		UG_WARNING( "Multiple return values not yet implemented in C++Generator."
		            << " Displaying as '()'." );
		return "()";
	} else {
		// no return value (i.e. void)
		return string("void");
	}
}

template< class TFunction >
void CppGenerator::generate_parameter_list( const TFunction &func )
{
	m_curr_file << "(";
	if ( func.num_parameter() > 0 ) {
		size_t i_last_param = func.num_parameter() - 1;
		for( size_t i_param = 0; i_param < i_last_param; ++i_param ) {
			m_curr_file << bridge::ParameterToString( func.params_in(), i_param )
			            << " " << sanitize_parameter_name( func.parameter_name( i_param ) ) << ", ";
		}
		m_curr_file << bridge::ParameterToString( func.params_in(), i_last_param )
		            << " " << sanitize_parameter_name( func.parameter_name( i_last_param ) );
	}
	m_curr_file << ")";
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
	return namespaces;
}

}	// end namespace ugdocu
}	// end namespace ug

/**
 * \file apps/ugdocu/src/cpp_generator.h
 * \author Torbjörn Klatt
 * \date 2013-04-23
 */

#ifndef __UG__REGDOCU__CPP_GENERATOR_H
#define __UG__REGDOCU__CPP_GENERATOR_H

#include <vector> // std::vector
#include <map>    // std::map
#include <string> // std::string

#ifdef UG_BRIDGE
# include "bridge/bridge.h"
#endif
#include "registry/registry.h"
#include "registry/class_helper.h"

namespace ug
{
namespace regDocu
{

using namespace std;

/// \addtogroup apps_ugdocu
/// \{

/**
 * \class CppGenerator
 * \brief Generator to produce valid but dummy C++ files from registered classes and functions
 * \details It requires the registry to be accessible by a call to 
 *   ug::bridge::GetUGRegistry().
 */
class CppGenerator
{
	public:
		/**
		 * \brief Default constructor
		 * \param dir     name and path of output directory for generated C++ files
		 * \param silent  flag for suppressing verbose logging
		 */
		CppGenerator( const string dir, bool silent=false );
		~CppGenerator();
		
		/**
		 * \brief Generates C++ files for all registered classes and functions
		 */
		void generate_cpp_files();
		
	private:
		/// \{
		/**
		 * \brief Generates C++ files for all classes in class groups
		 * \details This iterates over all registered class groups and generates
		 *   one C++ file per group containing all classes in this group.
		 * 
		 *   Usually a class group is a templated \ug4 class and the classes in this
		 *   group are specializations of this template depending on the build
		 *   parameters of the linked libug4.
		 */
		void generate_class_group();
		/**
		 * \brief Generates C++ code for all global functions
		 * \details This iterates over all global functions and writes their 
		 *   documentation and declaration to the file `global_functions.cpp`.
		 */
		void generate_global_functions();
		/// \}
		
		/// \{
		/**
		 * \brief Writes documentation for current class to current file
		 */
		void generate_class_docu();
		/**
		 * \brief Writes C++ code for current class to current file
		 * \details This includes the C++ code and documentation for all members of
		 *   the current class.
		 */
		void generate_class_cpp();
		/// \}
		
		/// \{
		/**
		 * \brief Writes documentation and declarations of constructors of current class
		 * \details Only public constructors are written.
		 *   In case there are no public constructors (i.e. the class is not 
		 *   instanciable), no constructors are written.
		 */
		void generate_class_constructors();
		/**
		 * \brief Generates documentation and declarations for all methods of current class
		 */
		void generate_class_public_methods();
		/**
		 * \brief Generates documentation and C++ signature for all public members of current class
		 * \todo Not really implemented yet. Are there even any public members registered?
		 */
		void generate_class_public_members();
		/// \}
		
		/// \{
		/**
		 * \brief Writes brief and detailed documentation for an entity
		 * \details Writes the brief and detailed documentation for the given entity
		 *   to the current file.
		 * \tparam TEntity Either a registered class or function, which provides the
		 *   members
		 *   - `string tooltip()`
		 *   - `string help()`
		 * \param entity Class or function to be documented
		 */
		template< class TEntity >
		void write_brief_detail_docu( const TEntity &entity );
		/**
		 * \brief Writes documentation for input parameters of given function
		 * \details For each input parameter of the given function the documentation
		 *   is written to the current file
		 * \tparam TFunction a function type usually either derived from 
		 *   ug::bridge::ExportedFunctionBase or ug::bridge::ExportedConstructor 
		 *   providing at least the members
		 *   - `int num_parameter()`
		 *   - `string parameter_name( size_t i )`
		 * \param function function to be documented
		 */
		template< class TFunction >
		void write_parameter_docu( const TFunction &function );
		/**
		 * \brief Writes documentation and declaration of given function
		 * \details Writes the full documentation and method signature of the given
		 *   function to the current file.
		 * \tparam TFunction a function type usually derived from 
		 *   ug::bridge::ExportedFunctionBase providing at least the members
		 *   - `string name()`
		 *   - `int num_parameter()`
		 *   - `ParameterInfo params_in()`
		 *   - `string parameter_name( size_t i )`
		 * \param function function to be written
		 * \param constant whether the function is const or not
		 */
		template< class TFunction >
		void write_generic_function( const TFunction &function, bool constant=false );
		/// \}
		
		/// \{
		/**
		 * \brief Generates parameter list for the given function
		 * \details Generates the parameter list as in function declerations for the
		 *   given registered method.
		 *   The enclosing brakets ('(' and ')') are also written.
		 * \tparam TFunction a function type usually either derived from 
		 *   ug::bridge::ExportedFunctionBase or ug::bridge::ExportedConstructor 
		 *   providing at least the members
		 *   - `int num_parameter()`
		 *   - `ParameterInfo params_in()`
		 *   - `string parameter_name( size_t i )`
		 * \param func method object to be used
		 * \returns parameter list
		 */
		template< class TFunction >
		string generate_parameter_list( const TFunction &func );
		/**
		 * \brief Writes docu on return value and returns return value as string
		 * \details Documentation on the return value of the given function \c method
		 *   is written to m_curr_file.
		 * 
		 *   A single return value is converted into a valid C++ string by utilizing
		 *   ug::bridge::ParameterToString().
		 *   If \c func has no return value, \c void is displayed and returned.
		 * \param method the method to generated the return value for
		 * \returns the return value of \c func as a string
		 * \todo Implement handling of multiple return values. Currently they are 
		 *   displayed as '()'.
		 *   This might be superfluous if there are no methods with more than one
		 *   return value registered.
		 */
		string generate_return_value( const bridge::ExportedFunctionBase &method );
		/// \}
		
		/// \{
		/**
		 * \brief Converts a group or class name to a portable ID
		 * \details It
		 *   - deletes all whitespaces
		 *   - replaces all occurrences of '/' by '_'
		 *   - transforms all characters to their lower case equivalents
		 * \param str initial name to convert
		 * \returns converted name
		 */
		string name_to_id( const string& str );
		/**
		 * \brief Tunes parameter names to be valid C++ variable names
		 * \details In case the given parameter name is empty, it is rendered as 
		 *   \c unnamed.
		 *   Otherwise it trims leading and trailing whitespaces and replaces the
		 *   remaining whitespaces by underscores.
		 * \param param parameter name to be preprocessed
		 * \returns sanitized parameter name
		 * \see sanitize_docu
		 */
		string sanitize_parameter_name( const string &param ) const;
		/**
		 * \brief Tunes parameter name to be rendered in docu
		 * \details In case the given parameter is empty, it is rendered as 
		 *   \c undocumented.
		 *   Otherwise it is returned as it.
		 * \param param_docu parameter to be preprocessed for docstring.
		 * \returns preprocessed parameter name
		 */
		string sanitize_docu( const string &param_docu ) const;
		/**
		 * \brief Tokenizes given group name into its separate parts
		 * \details Splits the given group name at the characters '_' and '/'.
		 * \param group group name to be tokenized
		 * \returns vector of strings of tokens
		 */
		vector<string> split_group_hieararchy( const string group );
		/**
		 * \brief Converts group hierarchy into namespaces and writes them to file
		 * \details Creates a namespace for each element of the group vector while
		 *   empty group names are ignored and the group name \em ug4 is replaced
		 *   by \em ug4bridge .
		 * \param group_hierarchy vector of groups as returned by split_group_hieararchy(group)
		 * \returns string with the closing namespace brackets
		 */
		string write_group_namespaces( vector<string> group_hierarchy );
		/// \}
		
	private:
		string m_output_dir;
		
		/// \brief Reference to the registry (to save countless calls to ug::bridge::GetUGRegistry())
		bridge::Registry &mr_reg;
		
		/// \brief Pointer to the currently processed class (if applicable)
		bridge::IExportedClass *m_curr_class;
		/// \brief Pointer to the currently processed group (if applicable)
		bridge::ClassGroupDesc *m_curr_group;
		/// \brief Name of current group as in \ug4 itself (including namespaces)
		string m_curr_group_name;
		
		/// \brief Map for keeping track of already processed classes
		/// \details Key is the class name, which maps to the file name this class
		///   has been written to.
		map<string, string> m_written_classes;
		
		/// \brief Current file stream to write to
		fstream m_curr_file;
		
		bool m_silent;
};

// end group apps_ugdocu
/// \}

}	// end namespace regDocu
}	// end namespace ug

#endif	// __UG__REGDOCU__CPP_GENERATOR_H

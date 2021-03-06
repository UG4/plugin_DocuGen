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

#ifndef __UG__REGDOCU__CPP_GENERATOR_H
#define __UG__REGDOCU__CPP_GENERATOR_H

#include <vector> // std::vector
#include <map>    // std::map
#include <set>    // std::set
#include <string> // std::string

#ifdef UG_BRIDGE
# include "bridge/bridge.h"
#endif
#include "registry/registry.h"
#include "registry/class_helper.h"
#include "class_hierarchy_provider.h"

namespace ug
{
namespace DocuGen
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
		CppGenerator( const string dir, ClassHierarchyProvider &chp, bool silent=false );
		/**
		 * \brief Destructor
		 * \details if the file stream is still open, it closes it
		 */
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
		/**
		 * \brief Writes some general docu on parent namespace
		 * \details also warning for the plugin namespace
		 */
		void write_group_definitions();
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
		 * \param str initial name to convert
		 * \returns converted name
		 */
		inline string name_to_id( const string& str );
		/**
		 * \brief Converts a value type to a string representation
		 * \details \ug4's internal classes are correctly substituted as well
		 *   as other registered classes and types.
		 * \param[in] par  parameter stack
		 * \param[in] i    index of the parameter from the parameter stack
		 * \return string representation of the i'th parameter type
		 */
		string parameter_to_string( const bridge::ParameterInfo &par, const int i ) const;
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
		inline string sanitize_parameter_name( const string &param ) const;
		/**
		 * \brief Detects empty tooltips and help and fills it with dummy content
		 * \details For an empty tooltip (e.g. a brief docu) the string "no brief documentation"
		 *   is used; for an empty help (e.g. a detailed docu) the string "no documentation"
		 *   is used
		 * \param[in] docstring either a tooltip or help text
		 * \param[in] is_brief  indicator whether the \c docstring is a tooltip
		 * \returns sanitized docstring
		 */
		inline string sanitize_docstring( const string &docstring, bool is_brief=false );
		/**
		 * \brief Tunes parameter name to be rendered in docu
		 * \details In case the given parameter is empty, it is rendered as 
		 *   \c undocumented.
		 *   Otherwise it is returned as it.
		 * \param param_docu parameter to be preprocessed for docstring.
		 * \returns preprocessed parameter name
		 */
		inline string sanitize_docu( const string &param_docu ) const;
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
		 * \param[in] group_hierarchy vector of groups as returned by split_group_hieararchy(group)
		 * \returns string with the closing namespace brackets
		 */
		string write_group_namespaces( vector<string> group_hierarchy, bool is_global_func=false );
		/// \}
		
	private:
		string m_output_dir;
		
		/// \brief Reference to the registry (to save countless calls to ug::bridge::GetUGRegistry())
		bridge::Registry &mr_reg;
		/// \brief Reference to a helper for retrieving the class group of a class
		ClassHierarchyProvider &mr_chp;
		
		/// \brief Pointer to the currently processed class (if applicable)
		bridge::IExportedClass *m_curr_class;
		/// \brief Pointer to the currently processed group (if applicable)
		bridge::ClassGroupDesc *m_curr_group;
		/// \brief Name of current group as in \ug4 itself (including namespaces)
		string m_curr_group_name;
		/// \brief Whether the currently processed class or function is registered by a plugin
		bool m_is_plugin;
		/// \brief Whether the currently processed function is global
		bool m_is_global;
		
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

}	// end namespace DocuGen
}	// end namespace ug

#endif	// __UG__REGDOCU__CPP_GENERATOR_H

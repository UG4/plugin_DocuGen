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

#include "common/log.h"

#include "bridge/bridge.h"

#include "class_hierarchy_provider.h"

namespace ug
{
namespace regDocu
{

using namespace std;

ClassHierarchyProvider::ClassHierarchyProvider() : 
	  class_to_group()
{
}

ClassHierarchyProvider::ClassHierarchyProvider( const ClassHierarchyProvider &other )
{
	this->class_to_group = other.class_to_group;
}

ClassHierarchyProvider& ClassHierarchyProvider::operator=( const ClassHierarchyProvider &other )
{
	this->class_to_group = other.class_to_group;
	return *this;
}

void ClassHierarchyProvider::init( bridge::Registry &reg )
{
	UG_LOG( "CHP: Initializing mapping for " << reg.num_class_groups() << " class groups ..." << endl );
	
	for ( size_t i_class_group = 0; i_class_group < reg.num_class_groups(); ++i_class_group ) {
		bridge::ClassGroupDesc *group = reg.get_class_group( i_class_group );
		for ( size_t i_class = 0; i_class < group->num_classes(); ++ i_class )
		{
			bridge::IExportedClass *klass = group->get_class( i_class );
			pair<string, string> class_group_names = make_pair<string, string>( klass->name(), group->name() );
			map<string, string>::iterator iter = class_to_group.find( klass->name() );

			if ( iter == class_to_group.end() ) {
				class_to_group.insert( class_group_names );
			} else {
				UG_WARNING( "Class '" << (class_group_names.first) << "' already mapped to group '" << iter->second << "'\n" );
			}
		}
	}
}

string ClassHierarchyProvider::get_group( const string class_name )
{
	map<string, string>::iterator value = class_to_group.find( class_name );
	if ( value != class_to_group.end() ) {
		return (*value).second;
	} else {
		return class_name;
	}
}

}	// namespace regDocu
}	// namespace ug

/**
 * \file apps/ugdocu/src/class_hierarchy_provider.cpp
 * \author Torbjörn Klatt
 * \date 2013-04-27
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

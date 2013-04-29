/**
 * \file apps/ugdocu/src/class_hierarchy_provider.h
 * \author Torbjörn Klatt
 * \date 2013-04-27
 */

#ifndef __UG__REGDOCU__CLASS_HIERARCHY_PROVIDER_H
#define __UG__REGDOCU__CLASS_HIERARCHY_PROVIDER_H

#include "common/log.h"

#include "bridge/bridge.h"

namespace ug
{
namespace regDocu
{

using namespace std;

/**
 * \addtogroup apps_ugdocu
 */
class ClassHierarchyProvider
{
	public:
		ClassHierarchyProvider();
		ClassHierarchyProvider( const ClassHierarchyProvider &other );
		
		ClassHierarchyProvider& operator=( const ClassHierarchyProvider &other );
		
		void init( bridge::Registry &reg );
		string get_group( const string class_name );

	private:
		map< string, string > class_to_group;
};

// end group apps_ugdocu
/// \}

}	// namespace regDocu
}	// namespace ug

#endif	// __UG__REGDOCU__CLASS_HIERARCHY_PROVIDER_H

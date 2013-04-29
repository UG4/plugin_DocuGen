/**
 * \file apps/ugdocu/src/ug_docu_class_description.h
 *
 *  \date	 	25.03.2013
 *  \author 	mrupp
 */

#include <iostream>

#include <string>
#include <vector>
#include "bridge/bridge.h"


#ifndef __UG__UGDOCUCLASSDESCRIPTION_H__
#define __UG__UGDOCUCLASSDESCRIPTION_H__

namespace ug
{
namespace regDocu
{

class UGDocuClassDescription
{
public:
	UGDocuClassDescription(const bridge::IExportedClass *klass) :
		  mp_class( klass )
		, mp_group( NULL )
	{}

	UGDocuClassDescription(const bridge::ClassGroupDesc *group) :
		  mp_class( NULL )
		, mp_group( group )
	{}

	const std::string name() const
	{
		if(mp_class) return mp_class->name();
		else if(mp_group) return mp_group->name();
		else return " ";
	}

	const std::string group_str() const
	{
		if(mp_class) return mp_class->group();
		if(mp_group)
		{
			if(mp_group->get_default_class()) return mp_group->get_default_class()->group();
			else if(mp_group->num_classes() > 0) return mp_group->get_class(0)->group();
			else return " ";
		}
		else return " ";
	}


	const bridge::IExportedClass *mp_class;
	const bridge::ClassGroupDesc *mp_group;
	std::string tag;
};

bool NameSortFunction(const UGDocuClassDescription &i, const UGDocuClassDescription &j);
bool GroupNameSortFunction(const UGDocuClassDescription &i, const UGDocuClassDescription &j);

UGDocuClassDescription *GetUGDocuClassDescription(std::vector<UGDocuClassDescription> &classes, const bridge::IExportedClass* c);
void GetGroups(std::vector<UGDocuClassDescription> &classes, std::vector<UGDocuClassDescription> &classesAndGroups,
		std::vector<UGDocuClassDescription> &classesAndGroupsAndImplementations );


}	// namespace regDocu
}	// namespace ug

#endif /* __UG__UGDOCUCLASSDESCRIPTION_H__ */

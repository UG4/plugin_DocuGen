/**
 * \file /ug4svn/apps/ugdocu/src/UGDocuClassDescription.h
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

namespace ug {

class UGDocuClassDescription
{
public:
	UGDocuClassDescription(const bridge::IExportedClass *_c)
	{
		c = _c;
		group = NULL;
	}

	UGDocuClassDescription(const bridge::ClassGroupDesc *_group)
	{
		group = _group;
		c = NULL;
	}

	std::string name() const
	{
		if(c) return c->name();
		else if(group) return group->name();
		else return " ";
	}

	std::string group_str() const
	{
		if(c) return c->group();
		if(group)
		{
			if(group->get_default_class()) return group->get_default_class()->group();
			else if(group->num_classes() > 0) return group->get_class(0)->group();
			else return " ";
		}
		else return " ";
	}


	const bridge::IExportedClass *c;
	const bridge::ClassGroupDesc *group;
	std::string tag;
};

bool NameSortFunction(const UGDocuClassDescription &i, const UGDocuClassDescription &j);
bool GroupNameSortFunction(const UGDocuClassDescription &i, const UGDocuClassDescription &j);

UGDocuClassDescription *GetUGDocuClassDescription(std::vector<UGDocuClassDescription> &classes, const bridge::IExportedClass* c);
void GetGroups(std::vector<UGDocuClassDescription> &classes, std::vector<UGDocuClassDescription> &classesAndGroups,
		std::vector<UGDocuClassDescription> &classesAndGroupsAndImplementations );


} // namespace ug

#endif /* __UG__UGDOCUCLASSDESCRIPTION_H__ */

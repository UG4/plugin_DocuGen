/*
 * Copyright (c) 2013-2016:  G-CSC, Goethe University Frankfurt
 * Author: Martin Rupp
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

/// \addtogroup apps_ugdocu
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

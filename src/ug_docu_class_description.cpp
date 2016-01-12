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

#include "ug_docu_class_description.h"
#include "ugdocu_misc.h"
using namespace std;
using namespace ug;
using namespace bridge;


namespace ug
{
namespace regDocu
{

extern vector<UGDocuClassDescription> classes;
extern vector<UGDocuClassDescription> classesAndGroups;
extern vector<UGDocuClassDescription> classesAndGroupsAndImplementations;

bool NameSortFunction(const UGDocuClassDescription &i, const UGDocuClassDescription &j)
{
	return i.name().compare(j.name()) < 0;
}

bool GroupNameSortFunction(const UGDocuClassDescription &i, const UGDocuClassDescription &j)
{
	int c = i.group_str().compare(j.group_str());
	if(c == 0)
		return i.name() < j.name();
	else
		return c < 0;
}

UGDocuClassDescription *GetUGDocuClassDescription(std::vector<UGDocuClassDescription> &classes, const IExportedClass* c)
{
	UGDocuClassDescription desc(c);
	std::vector<UGDocuClassDescription>::iterator it = lower_bound(classes.begin(), classes.end(), desc, NameSortFunction);
	if(it != classes.end() && (*it).mp_class == c)
		return &*it;
	else return NULL;
	//for(size_t i=0; i<classes.size(); i++)
		//if(classes[i].c == c) return &classes[i];
}


void GetGroups(std::map<string, UGRegistryGroup> &g)
{
	try{
	for(size_t i=0; i<classesAndGroups.size(); i++)
	{
		string group = classesAndGroups[i].group_str();
		g[group].classesAndGroups.push_back(classesAndGroups[i]);
		UG_LOG("CLASS: " << group << " " << classesAndGroups[i].name() << "\n");
	}

	Registry &reg = GetUGRegistry();

	std::vector<const bridge::ExportedFunction *> sortedFunctions;
	for(size_t i=0; i<reg.num_functions(); i++)
	{
		ExportedFunctionGroup &fu = reg.get_function_group(i);
		for(size_t j=0; j<fu.num_overloads(); j++)
		{
			g[fu.get_overload(j)->group()].functions.push_back(fu.get_overload(j));
			UG_LOG("FUNCTION: " << fu.get_overload(j)->group() << " " << fu.get_overload(j)->name() << "\n");
		}
	}

	for(std::map<string, UGRegistryGroup>::iterator it = g.begin(); it != g.end(); ++it)
	{
		std::vector<bridge::ExportedFunction*> &v = it->second.functions;
		sort(v.begin(), v.end(), ExportedFunctionsSort);
	}
	}UG_CATCH_THROW_FUNC();
}

void GetGroups(std::vector<UGDocuClassDescription> &classes, std::vector<UGDocuClassDescription> &classesAndGroups,
		std::vector<UGDocuClassDescription> &classesAndGroupsAndImplementations )
{
	try{
	Registry &reg = GetUGRegistry();

	for(size_t i=0; i<reg.num_classes(); ++i)
		classes.push_back(UGDocuClassDescription(&reg.get_class(i)));
	sort(classes.begin(), classes.end(), NameSortFunction);

	//for(size_t i=0; i<reg.num_classes(); ++i)
	//	UG_LOG(classes[i].name() << "\n");

	for(size_t i=0; i<reg.num_class_groups(); ++i)
	{
		ClassGroupDesc *g = reg.get_class_group(i);

		//UG_LOG("group " << g->name() << "\n");
		for(uint j=0; j<g->num_classes(); j++)
		{
			//UG_LOG("searching " << g->get_class(j)->name() << "\n");
			UGDocuClassDescription *d = GetUGDocuClassDescription(classes, g->get_class(j));
			if(d) { d->mp_group = g; d->tag = g->get_class_tag(j);  }
			//else UG_LOG("not found.\n");
		}
	}

	for(size_t i=0; i<classes.size(); ++i)
	{
		if(classes[i].mp_group == NULL)
			classesAndGroups.push_back(classes[i]);
	}
	classesAndGroupsAndImplementations = classes;

	for(size_t i=0; i<reg.num_class_groups(); ++i)
	{
		UGDocuClassDescription d(reg.get_class_group(i));
		classesAndGroups.push_back(d);
		classesAndGroupsAndImplementations.push_back(d);
	}

	sort(classesAndGroups.begin(), classesAndGroups.end(), NameSortFunction);

	/*
	for(size_t i=0; i<classesAndGroups.size(); i++)
	{
		UG_LOG(classesAndGroups[i].group_str() << "	" << classesAndGroups[i].name());
		if(classesAndGroups[i].c == NULL) UG_LOG("	(group)");
		UG_LOG("\n");
	}*/
	}UG_CATCH_THROW_FUNC();
}

}	// namespace regDocu
}	// namespace ug

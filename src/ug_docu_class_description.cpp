/**
 * \file /ug4svn/apps/ugdocu/src/UGDocuClassDescription.cpp
 *
 *  \date	 	25.03.2013
 *  \author 	mrupp
 */

#include "ug_docu_class_description.h"
#include "ugdocu_misc.h"
using namespace std;
using namespace ug;
using namespace bridge;

extern std::vector<UGDocuClassDescription> classes;
extern std::vector<UGDocuClassDescription> classesAndGroups;
extern std::vector<UGDocuClassDescription> classesAndGroupsAndImplementations;

namespace ug {

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
	if(it != classes.end() && (*it).c == c)
		return &*it;
	else return NULL;
	//for(size_t i=0; i<classes.size(); i++)
		//if(classes[i].c == c) return &classes[i];
}


void GetGroups(std::map<string, UGRegistryGroup> &g)
{
	for(int i=0; i<classesAndGroups.size(); i++)
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

}

void GetGroups(std::vector<UGDocuClassDescription> &classes, std::vector<UGDocuClassDescription> &classesAndGroups,
		std::vector<UGDocuClassDescription> &classesAndGroupsAndImplementations )
{
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
			if(d) { d->group = g; d->tag = g->get_class_tag(j);  }
			//else UG_LOG("not found.\n");
		}
	}

	for(size_t i=0; i<classes.size(); ++i)
	{
		if(classes[i].group == NULL)
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
}

} // namespace ug

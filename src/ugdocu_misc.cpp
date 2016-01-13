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
#include <sstream>

#include <string>

#include "ug.h"
#include "ugbase.h"

#include "bindings/lua/lua_util.h"
#include "bridge/bridge.h"
#include "registry/class_helper.h"
#include "ug_docu_class_description.h"

using namespace std;
using namespace ug;
using namespace bridge;


namespace ug
{
namespace DocuGen
{

extern vector<UGDocuClassDescription> classes;
extern vector<UGDocuClassDescription> classesAndGroups;
extern vector<UGDocuClassDescription> classesAndGroupsAndImplementations;

bool IsPluginGroup(string g)
{
	return StartsWith(g, "(Plugin) ");
}
string GetFilenameForGroup(string s, string dir)
{
	if(IsPluginGroup(s)) s = string("plugin.")+s.substr(9);
	return string(dir) + ReplaceAll(s, "/", ".") + "group.html";
}

bool ClassGroupDescSort(const ClassGroupDesc *i, const ClassGroupDesc *j)
{
	return i->name().compare(j->name()) < 0;
}

bool ExportedClassSort(const IExportedClass *i, const IExportedClass *j)
{
	return i->name().compare(j->name()) < 0;
}

bool ExportedFunctionsSort(const bridge::ExportedFunctionBase * i,
		const bridge::ExportedFunctionBase *j)
{
	return i->name() < j->name();
}
bool ExportedFunctionsGroupSort(const bridge::ExportedFunction * i,
		const bridge::ExportedFunction *j)
{
	int c = i->group().compare(j->group());
	if(c == 0)
		return i->name() < j->name();
	else
		return c < 0;
}


string tohtmlstring(const string &str)
{
	return str;
}


string GetClassGroup(string classname)
{
	const Registry &reg = GetUGRegistry();
	const IExportedClass*c = reg.get_class(classname);
	if(c == NULL) return classname;
	else
	{
		UGDocuClassDescription *d=GetUGDocuClassDescription(classes, c);
		if(d == NULL || d->mp_group == NULL) return classname;
		else return d->mp_group->name();
	}
}

string GetClassGroupStd(string classname)
{
	const Registry &reg = GetUGRegistry();
	const IExportedClass*c = reg.get_class(classname);
	if(c == NULL) return classname;
	else
	{
		UGDocuClassDescription *d=GetUGDocuClassDescription(classes, c);
		if(d == NULL || d->mp_group == NULL || d->mp_group->get_default_class() == NULL) return classname;
		else return d->mp_group->get_default_class()->name();
	}
}


void ParameterToString(ostream &file, const bridge::ParameterInfo &par, int i, bool bHTML)
{
	if(bHTML)
		file << XMLStringEscape(ParameterToString(par, i)) << " ";
	else
		file << ParameterToString(par, i) << " ";
}


void WriteClassHierarchy(ostream &file, ClassHierarchy &c)
{
	file << "<li>";
	if(!c.bGroup)
		file << "<a class=\"el\" href=\"" << c.name << ".html\">" << GetClassGroup(c.name) << "</a>";
	else
		file << GetClassGroup(c.name) << " ";
	if(c.subclasses.size())
	{
		file << "<ul>";
		for(size_t i=0; i<c.subclasses.size(); i++)
			WriteClassHierarchy(file, c.subclasses[i]);
		file << "</ul>" ;
	}
}




string GetBeautifiedTag(string tag)
{
	ReplaceAll(tag, ";", " ");
	return tag;
}


}	// namespace DocuGen
}	// namespace ug

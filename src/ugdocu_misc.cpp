/**
 * \file apps/ugdocu/src/ugdocu_misc.cpp
 *
 *  \date	 	25.03.2013
 *  \author 	mrupp
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
namespace regDocu
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


}	// namespace regDocu
}	// namespace ug

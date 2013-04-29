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
	return string(dir) + ReplaceAll(s, "/", ".") + "group..html";
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


bool ParameterToString(ostream &file, const bridge::ParameterInfo &par, int i, bool bHTML)
{
	if(par.is_vector(i)){
		if(bHTML) file << "std::vector&lt";
		else file << "std::vector<";
	}
	switch(par.type(i))
	{
	default:
	case Variant::VT_INVALID:
		file << "unknown ";
		break;
	case Variant::VT_BOOL:
		file << "bool ";
		break;

	case Variant::VT_INT:
		file << "integer ";
		break;

	case Variant::VT_SIZE_T:
		file << "size_t ";
		break;

	case Variant::VT_FLOAT:
	case Variant::VT_DOUBLE:
		file << "number ";
		break;

	case Variant::VT_CSTRING:
		file << "c_string ";
		break;

	case Variant::VT_STDSTRING:
		file << "std_string ";
		break;

	case Variant::VT_POINTER:
		if(bHTML)	file << "<a href=\"" << GetClassGroupStd(par.class_name(i)) << ".html\"" << ">";
		file << GetClassGroup(par.class_name(i));
		if(bHTML)	file << "</a>";
		file << " *";
		break;

	case Variant::VT_CONST_POINTER:
		file << "const ";
		if(bHTML)	file << "<a href=\"" << GetClassGroupStd(par.class_name(i)) << ".html\"" << ">";
		file << GetClassGroup(par.class_name(i));
		if(bHTML)	file << "</a>";
		file << " *";
		break;

	case Variant::VT_SMART_POINTER:
		if(bHTML)	file << "SmartPtr&lt;<a href=\"" << GetClassGroupStd(par.class_name(i)) << ".html\"" << ">" << GetClassGroup(par.class_name(i)) << "</a>&gt; ";
		else		file << "SmartPtr<" << GetClassGroup(par.class_name(i)) << "> ";
		break;

	case Variant::VT_CONST_SMART_POINTER:
		if(bHTML)	file << "const SmartPtr&lt;<a href=\"" << GetClassGroupStd(par.class_name(i)) << ".html\"" << ">" << GetClassGroup(par.class_name(i)) << "</a>&gt; ";
		else		file << "const SmartPtr<" << GetClassGroup(par.class_name(i)) << "> ";
		break;
	}
	if(par.is_vector(i)){
		if(bHTML) file << "&gt";
		else file << ">";
	}
	return true;
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

/**
 * \file createdocu.cpp
 *
 * \author Martin Rupp
 *
 * \date 18.10.2010
 *
 * Goethe-Center for Scientific Computing 2010.
 */

#include <iostream>
#include <sstream>

#include <string>

#include "ug.h"
#include "ugbase.h"

#include "bindings/lua/lua_util.h"
#include "registry/class_helper.h"
#include "bridge/bridge.h"
#include "common/util/parameter_parsing.h"
#include "compile_info/compile_info.h"
#include <stdio.h>
#include <time.h>

using namespace std;
using namespace ug;
using namespace bridge;


// to refresh this file, use xxd -i ugdocu.css > ugdocu.css.h
#include "ugdocu.css.h"

namespace ug {

namespace bridge
{
extern bool IsClassInParameters(const bridge::ParameterInfo &par, const char *classname);
}


} // namespace ug

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

void WriteHeader(fstream &file, const string &title)
{
	file << "<address style=\"align: right;\"><small>";

	time_t now = time(0);
	tm *local = localtime(&now);
	file << "ug4 bridge docu. SVN Revision " << UGSvnRevision() << ". Generated on " << asctime(local);
	file << "</small></address>" << endl;

	file << "<hr size=\"1\">";
	file << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">" << endl;
	file << "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=iso-8859-1\">";
	file << "<title>" << tohtmlstring(title) << "</title>" << endl;
	file << "<link href=\"ugdocu.css\" rel=\"stylesheet\" type=\"text/css\">" << endl;
	file << "</head><body>" << endl;

	//file << "<div class=\"qindex\"><a class=\"qindex\" href=\"hierarchy.html\">Class Hierarchy</a>";
	file << " | <a class=\"qindex\" href=\"index.html\">Class Index</a>";
	file << " | <a class=\"qindex\" href=\"groupindex.html\">Class Index by Group</a>";
	file << " | <a class=\"qindex\" href=\"functions.html\">Global Functions</a>";
	file << " | <a class=\"qindex\" href=\"groupedfunctions.html\">Global Functions by Group</a>";
	file << "</div>" << endl;
}

void WriteFooter(fstream &file)
{

	file << "</body>" << endl << "</html>" << endl;
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
		if(bHTML)	file << "<a href=\"" << par.class_name(i) << ".html\"" << ">";
		file << par.class_name(i);
		if(bHTML)	file << "</a>";
		file << " *";
		break;

	case Variant::VT_CONST_POINTER:
		file << "const ";
		if(bHTML)	file << "<a href=\"" << par.class_name(i) << ".html\"" << ">";
		file << par.class_name(i);
		if(bHTML)	file << "</a>";
		file << " *";
		break;
		
	case Variant::VT_SMART_POINTER:
		if(bHTML)	file << "SmartPtr&lt;<a href=\"" << par.class_name(i) << ".html\"" << ">" << par.class_name(i) << "</a>&gt; ";
		else		file << "SmartPtr<" << par.class_name(i) << "> ";
		break;

	case Variant::VT_CONST_SMART_POINTER:
		if(bHTML)	file << "const SmartPtr&lt;<a href=\"" << par.class_name(i) << ".html\"" << ">" << par.class_name(i) << "</a>&gt; ";
		else		file << "const SmartPtr<" << par.class_name(i) << "> ";
		break;
	}
	if(par.is_vector(i)){
		if(bHTML) file << "&gt";
		else file << ">";
	}
	return true;
}

template<typename T>
bool WriteParametersIn(ostream &file, const T &thefunc, bool bHTML=true)
{
	file << "(";
	for(size_t i=0; i < (size_t)thefunc.params_in().size(); ++i)
	{
		if(i>0) file << ", ";
		ParameterToString(file, thefunc.params_in(), i, bHTML);
		if(i<thefunc.num_parameter())
			file << thefunc.parameter_name(i);
	}
	file << ")";
	return true;
}
template<typename T>
bool WriteParametersOut(ostream &file, const T &thefunc, bool bHTML=true)
{
	if(thefunc.params_out().size() == 1)
	{
		ParameterToString(file, thefunc.params_out(), 0, bHTML);
		//file << " " << thefunc.return_name();
	}
	else if(thefunc.params_out().size() > 1)
	{
		file << "(";
		for(int i=0; i < thefunc.params_out().size(); ++i)
		{
			if(i>0) file << ", ";
			ParameterToString(file, thefunc.params_out(), i, bHTML);
		}
		file << ")";
	}
	else
	{
		file << "void ";
	}
	return true;
}

void WriteConstructorInfoHTML(ostream &file, string classname, const bridge::ExportedConstructor &thefunc,
		string group)
{
	// function name
	file << "<tr><td class=\"memItemLeft\" nowrap align=right valign=top>";
	file << "</td><td class=\"memItemRight\" valign=bottom>";
	file << classname << " ";
	WriteParametersIn(file, thefunc);	
	file << "</td></tr>";
	
	if(thefunc.tooltip().size() > 0)
	{
		file << "<tr><td class=\"mdescLeft\">&#160;</td><td class=\"mdescRight\">";
		file << "tooltip: " << thefunc.tooltip() << "<br/></td></tr>";
	}

	if(thefunc.help().size() > 0 && thefunc.help().compare("No help") != 0)
	{
		file << "<tr><td class=\"mdescLeft\">&#160;</td><td class=\"mdescRight\">";
		file << "help: " << thefunc.help() << "<br/></td></tr>";
	}
}



void WriteFunctionInfoHTML(ostream &file, const bridge::ExportedFunctionBase &thefunc,
		const IExportedClass *c = NULL, bool bConst = false)
{
	file << "<tr><td class=\"memItemLeft\" nowrap align=right valign=top>";
	WriteParametersOut(file, thefunc);

	file << "</td><td class=\"memItemRight\" valign=bottom>";
	if(bConst)
		file << " const ";
	if(c)
		file << "<a href=\"" << c->name() << ".html\"" << ">" << c->name() << "</a>::";

	file << thefunc.name() << " ";

	WriteParametersIn(file, thefunc);
	file << "</td></tr>";

	if(thefunc.return_name().size() > 0)
	{
		file << "<tr><td class=\"mdescLeft\">&#160;</td><td class=\"mdescRight\">";
				file << "returns " << thefunc.return_name() << "<br/></td></tr>";
	}

	if(thefunc.tooltip().size() > 0)
	{
		file << "<tr><td class=\"mdescLeft\">&#160;</td><td class=\"mdescRight\">";
		file << "tooltip: " << thefunc.tooltip() << "<br/></td></tr>";
	}

	if(thefunc.help().size() > 0 && thefunc.help().compare("No help") != 0)
	{
		file << "<tr><td class=\"mdescLeft\">&#160;</td><td class=\"mdescRight\">";
		file << "help: " << thefunc.help() << "<br/></td></tr>";
	}
}



void WriteFunctionInfoHTML(ostream &file, const bridge::ExportedFunctionBase &thefunc,
		const char *group)
{
	file << "<tr>";
	file << "<td class=\"mdescLeft\">" << group << "</td>";
	file << "<td class=\"memItemLeft\" nowrap align=right valign=top>";
	WriteParametersOut(file, thefunc);

	file << "</td><td class=\"memItemRight\" valign=bottom>";
	file << thefunc.name() << " ";

	WriteParametersIn(file, thefunc);
	file << "</td></tr>";

	if(thefunc.return_name().size() > 0)
	{
		file << "<tr><td class=\"mdescLeft\">&#160;</td><td class=\"mdescLeft\">&#160;</td><td class=\"mdescRight\">";
				file << "returns " << thefunc.return_name() << "<br/></td></tr>";
	}

	if(thefunc.tooltip().size() > 0)
	{
		file << "<tr><td class=\"mdescLeft\">&#160;</td><td class=\"mdescLeft\">&#160;</td><td class=\"mdescRight\">";
		file << "tooltip: " << thefunc.tooltip() << "<br/></td></tr>";
	}

	if(thefunc.help().size() > 0 && thefunc.help().compare("No help") != 0)
	{
		file << "<tr><<td class=\"mdescLeft\">&#160;</td><td class=\"mdescLeft\">&#160;</td><td class=\"mdescRight\">";
		file << "help: " << thefunc.help() << "<br/></td></tr>";
	}
}

void WriteFunctionHTMLCompleter(ostream &file, const bridge::ExportedFunctionBase &thefunc,
		const char *group, const char *pClass)
{
	WriteParametersOut(file, thefunc);
	file << thefunc.name() << " ";
	WriteParametersIn(file, thefunc);
	file << "<br>" ;
	if(pClass != NULL) 
		file << "<br>Member function of class <b>" << pClass << "</b>";
	if(thefunc.return_name().size() > 0)
		file << "<br>returns " << thefunc.return_name();
	
	if(thefunc.tooltip().size() > 0)
		file << "<br>tooltip: " << thefunc.tooltip();
	
	if(thefunc.help().size() > 0 && thefunc.help().compare("No help") != 0)
		file << "<br>help: " << thefunc.help();
	
	if(group != NULL)
		file << "<br>group: <b>" << group << "</b>";
}


void WriteClassHierarchy(ostream &file, ClassHierarchy &c)
{
	file << "<li>";
	if(!c.bGroup)
		file << "<a class=\"el\" href=\"" << c.name << ".html\">" << c.name << "</a>";
	else
		file << c.name << " ";
	if(c.subclasses.size())
	{
		file << "<ul>";
		for(size_t i=0; i<c.subclasses.size(); i++)
			WriteClassHierarchy(file, c.subclasses[i]);
		file << "</ul>" ;
	}
}


/**
 *
 * \param classname the class (and only this class) to print usage in functions/member functions of.
 */
bool WriteClassUsageExact(const string &preamble, ostream &file, const char *classname, bool OutParameters)
{
	Registry &reg = GetUGRegistry();
	bool bPreambleWritten = false;
	// check functions
	for(size_t i=0; i<reg.num_functions(); i++)
	{
		const bridge::ExportedFunctionBase &thefunc = reg.get_function(i);
		if((!OutParameters && IsClassInParameters(thefunc.params_in(), classname)) ||
				(OutParameters && IsClassInParameters(thefunc.params_out(), classname)))
		{
			if(bPreambleWritten==false) { file << preamble; bPreambleWritten=true; }
			WriteFunctionInfoHTML(file, thefunc);
		}
	}

	// check classes
	for(size_t i=0; i<reg.num_classes(); i++)
	{
		const IExportedClass &c = reg.get_class(i);
		for(size_t i=0; i<c.num_methods(); i++)
		{
			const ExportedMethodGroup &grp = c.get_method_group(i);
			for(size_t j=0; j< grp.num_overloads(); j++)
			{
				const bridge::ExportedMethod *thefunc = grp.get_overload(j);
				if((!OutParameters && IsClassInParameters(thefunc->params_in(), classname)) ||
						(OutParameters && IsClassInParameters(thefunc->params_out(), classname)))
				{
					if(bPreambleWritten==false) { file << preamble; bPreambleWritten=true; }
					WriteFunctionInfoHTML(file, *thefunc, &c, false);
				}
			}

		}

		for(size_t i=0; i<c.num_const_methods(); i++)
		{
			const ExportedMethodGroup &grp = c.get_const_method_group(i);
			for(size_t j=0; j< grp.num_overloads(); j++)
			{
				const bridge::ExportedMethod *thefunc = grp.get_overload(j);
				if((!OutParameters && IsClassInParameters(thefunc->params_in(), classname)) ||
						(OutParameters && IsClassInParameters(thefunc->params_out(), classname)))
				{
					if(bPreambleWritten==false) { file << preamble; bPreambleWritten=true; }
					WriteFunctionInfoHTML(file, *thefunc, &c, true);
				}
			}
		}
	}
	return true;
}

void PrintClassFunctionsHMTL(ostream &file, const IExportedClass *c, bool bInherited)
{
	if(c == NULL) return;
	
	if(c->num_constructors())
	{
		file << "<tr><td colspan=2><h3>";
		file << c->name() << " Constructors</h3></td></tr>";
		for(size_t i=0; i<c->num_constructors(); ++i)
			WriteConstructorInfoHTML(file, c->name(), c->get_constructor(i), c->group());
		file << "<tr><td><br></td></tr>";
	}
	if(c->num_methods() > 0)
	{
		std::vector<const bridge::ExportedFunctionBase *> sortedFunctions;
		for(size_t i=0; i<c->num_methods(); ++i)
		{
			const ExportedMethodGroup &grp = c->get_method_group(i);
			for(size_t j=0; j<grp.num_overloads(); j++)
				sortedFunctions.push_back(grp.get_overload(j));
		}
		sort(sortedFunctions.begin(), sortedFunctions.end(), ExportedFunctionsSort);

		file << "<tr><td colspan=2><h3>";
		if(bInherited) file << "Inherited ";
		file << c->name() << " Member Functions</h3></td></tr>";
		for(size_t i=0; i < sortedFunctions.size(); ++i)
			WriteFunctionInfoHTML(file, *sortedFunctions[i]);
		file << "<tr><td><br></td></tr>";
	}

	if(c->num_const_methods() > 0)
	{
		std::vector<const bridge::ExportedFunctionBase *> sortedFunctions;
		for(size_t i=0; i<c->num_const_methods(); ++i)
		{
			const ExportedMethodGroup &grp = c->get_const_method_group(i);
			for(size_t j=0; j<grp.num_overloads(); j++)
				sortedFunctions.push_back(grp.get_overload(j));
		}
		sort(sortedFunctions.begin(), sortedFunctions.end(), ExportedFunctionsSort);

		file << "<tr><td colspan=2><h3>";
		if(bInherited) file << " Inherited ";
		file << c->name() << " Const Member Functions</h3></td></tr>";

		for(size_t i=0; i < sortedFunctions.size(); ++i)
			WriteFunctionInfoHTML(file, *sortedFunctions[i]);
		file << "<tr><td><br></td></tr>";
	}
}

// write the ugdocu.css
//TODO: make sure dir exists and is writeable!
void WriteUGDocuCSS(const char *dir)
{
	UG_LOG("WriteUGDocuCSS... ");
	fstream ugdocucss((string(dir).append("ugdocu.css")).c_str(), ios::out);
	ugdocucss.write((char *)ugdocu_css, ugdocu_css_len);
	UG_LOG(ugdocu_css_len << " characters." << endl);
}


// print class hierarchy in hierarchy.html
void WriteClassHierarchy(const char *dir, ClassHierarchy &hierarchy)
{
	fstream hierarchyhtml((string(dir).append("hierarchy.html")).c_str(), ios::out);
	WriteHeader(hierarchyhtml, "Class Hierarchy");
	hierarchyhtml << "<h1>ugbridge Class Hierarchy (ug4)</h1>This inheritance list sorted hierarchically:<ul>";
	for(size_t i=0; i<hierarchy.subclasses.size(); i++)
		WriteClassHierarchy(hierarchyhtml, hierarchy.subclasses[i]);
	hierarchyhtml << "</ul>";
	WriteFooter(hierarchyhtml);
}

// < vom hoffer
std::string replaceAll(
		std::string target,
		const std::string oldstr,
		const std::string newstr) {

	// no substitution necessary
	if (oldstr == newstr) {
		return target;
	}

	for (size_t x = target.find(oldstr); x != std::string::npos; x = target.find(oldstr, x + newstr.size())) {
		target.erase(x, oldstr.length());
		target.insert(x, newstr);
	}

	return target;
}// >

class UGDocuClassDescription
{
public:
	UGDocuClassDescription(const IExportedClass *_c)
	{
		c = _c;
		group = NULL;
	}

	UGDocuClassDescription(const ClassGroupDesc *_group)
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


	const IExportedClass *c;
	const ClassGroupDesc *group;
	string tag;
};

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

string GetBeautifiedTag(string tag)
{
	replaceAll(tag, ";", " ");
	return tag;
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

void WriteClassCompleter(ostream &classhtml, UGDocuClassDescription *d, ClassHierarchy &hierarchy)
{
	Registry &reg = GetUGRegistry();
	const IExportedClass *pC = d->c;
	if(pC == NULL)
	{
		pC = d->group->get_default_class();
		if(pC == NULL)
		{
			classhtml << "Group <b>" << d->group->name() << "</b> has no default implementation.";
			return;
		}
		classhtml << "Class <b>" << pC->name() << "</b>, default implementation of group <b>" 
				<< d->group->name() << "</b><br>";
	}
	else if(d->group != NULL)
		classhtml << "Group <b>" << d->group->name() << "</b><br>";
	
	if(pC->is_instantiable())
		classhtml << "class has constructor. ";
	else
		classhtml << "class has no constructor. ";
	if(d->tag.size() > 0) classhtml << " Tag <b>" << d->tag << "</b>";
	classhtml << "<br>";
	if(pC->tooltip().size() != 0)
		classhtml << pC->tooltip() << "<br>";

	// print parent classes
	const vector<const char *> *pNames = pC->class_names();
	if(pNames && pNames->size() > 1)
	{
		classhtml << "Inheritance: ";
		for(vector<const char*>::const_reverse_iterator rit = pNames->rbegin(); rit < pNames->rend(); ++rit)
			classhtml << "<a href=\"" << (*rit) << ".html\">" << (*rit) << "</a> ";
		classhtml << "<br>";
	}

	
	// print member functions
	classhtml 	<< "<table>";
	PrintClassFunctionsHMTL(classhtml, pC, false);
	if(pNames)
	{
		// print inherited member functions
		for(size_t i=1; i<pNames->size(); i++)
			PrintClassFunctionsHMTL(classhtml, reg.get_class(pNames->at(i)), true);
	}

	
	classhtml << "</table>";
	return;

	// print usage of this class in other classes

	classhtml 	<< "Usage Information<br>";
	classhtml 	<< "<table>";
	
	string name = pC->name();

	// functions returning this class
	string str = string("<tr><td colspan=2><br><h3> Functions returning ") + string(name) + string("</h2></td></tr>");
	WriteClassUsageExact(str, classhtml, name.c_str(), true);

	// functions using this class or its parents
	if(pNames)
	{
		for(size_t i=0; i<pNames->size(); i++)
		{
			string str = string("<tr><td colspan=2><br><h3> Functions using ") + string(pNames->at(i))
					+ string("</h2></td></tr>\n");
			WriteClassUsageExact(str, classhtml, pNames->at(i), false);
		}
	}
	else
	{
		string str = string("<tr><td colspan=2><br><h3> Functions using ") + string(name)
							+ string("</h2></td></tr>\n");
		WriteClassUsageExact(str, classhtml, name.c_str(), false);
	}
	classhtml << "</table>";
}

// write html file for a class
void WriteClassHTML(const char *dir, UGDocuClassDescription *d, ClassHierarchy &hierarchy)
{
	Registry &reg = GetUGRegistry();

	const IExportedClass &c = *d->c;
	string name = c.name();

	fstream classhtml((string(dir) + name + ".html").c_str(), ios::out);
	WriteHeader(classhtml, name);

	if(d->group == NULL)
		classhtml 	<< "<h1>" << name << " Class Reference</h1>";
	else
	{
		classhtml << "<h1>" << d->group->name() << " Class Reference</h1>";
		classhtml << name << "<br>" << d->tag << "<br>";
		if(d->group->get_default_class() == d->c)
			classhtml << "(default implementation of group " << d->group->name() << ")<br>";
	}

	if(c.tooltip().size() != 0)
		classhtml << "<p align=\"center\">" << c.tooltip() << "</p><br>";
	if(c.is_instantiable())
		classhtml << "class has constructor<br>";
	else
		classhtml << "class has no constructor<br>";


	classhtml << "<br>Group <b>" << c.group() << "</b><br>";

	classhtml << "<hr>\n";

	classhtml << "<a href=\"http://cave1.gcsc.uni-frankfurt.de/job/ug-doc/doxygen/"
			"classug_1_1";
	string groupname;
	if(d->group == NULL)
		groupname = name;
	else groupname = d->group->name();
	for(uint i=0; i<groupname.size(); i++)
	{
		if(groupname[i] >= 'A' && groupname[i] <= 'Z') classhtml << '_' <<
				(char)(groupname[i]-('A'-'a'));
		else classhtml << groupname[i];
	}
	classhtml << ".html\">Doxygen Documentation</a>\n";

	// print parent classes
	const vector<const char *> *pNames = c.class_names();
	if(pNames && pNames->size() > 1)
	{
		for(vector<const char*>::const_reverse_iterator rit = pNames->rbegin(); rit < pNames->rend(); ++rit)
		{
			classhtml << "<ul>";
			classhtml << "<li><a class=\"el\" href=\"" << (*rit) << ".html\">" << (*rit) << "</a>";
		}
		for(size_t i=0; i<pNames->size(); i++)
			classhtml << "</ul>";
	}

	// print member functions

	classhtml 	<< "<table border=0 cellpadding=0 cellspacing=0>"
				<< "<tr><td></td></tr>";

	PrintClassFunctionsHMTL(classhtml, &c, false);
	if(pNames)
	{
		// print inherited member functions
		for(size_t i=1; i<pNames->size(); i++)
			PrintClassFunctionsHMTL(classhtml, reg.get_class(pNames->at(i)), true);
	}

	classhtml << "</table>";

	// print usage of this class in other classes

	classhtml 	<< "<hr> <h1>Usage Information</h1>";

	classhtml 	<< "<table border=0 cellpadding=0 cellspacing=0>"
						<< "<tr><td></td></tr>";

	// functions returning this class
	string str = string("<tr><td colspan=2><br><h3> Functions returning ") + string(name) + string("</h2></td></tr>\n");
	WriteClassUsageExact(str, classhtml, name.c_str(), true);

	// functions using this class or its parents
	if(pNames)
	{
		for(size_t i=0; i<pNames->size(); i++)
		{
			string str = string("<tr><td colspan=2><br><h3> Functions using ") + string(pNames->at(i))
					+ string("</h2></td></tr>\n");
			WriteClassUsageExact(str, classhtml, pNames->at(i), false);
		}
	}
	else
	{
		string str = string("<tr><td colspan=2><br><h3> Functions using ") + string(name)
							+ string("</h2></td></tr>\n");
		WriteClassUsageExact(str, classhtml, name.c_str(), false);
	}
	classhtml << "</table>";

	// print subclasses of this class
	ClassHierarchy *theclass = hierarchy.find_class(name.c_str());
	if(theclass && theclass->subclasses.size() > 0)
	{
		classhtml 	<< "<h1>Subclasses</h1><ul>";
		WriteClassHierarchy(classhtml, *theclass);
		classhtml << "</ul>";
	}

	if(d->group != NULL)
	{
		classhtml 	<< "<hr> <h1>Other Implementations of " << d->group->name() << "</h1>";
		classhtml << "<ul>";
		for(size_t j=0; j<d->group->num_classes(); j++)
		{
			classhtml << "<li>" << "<a class=\"el\" href=\"" << d->group->get_class(j)->name() << ".html\">" << d->group->get_class(j)->name() << "</a> (" << GetBeautifiedTag(d->group->get_class_tag(j)) << ")";
			if(d->group->get_default_class() == d->group->get_class(j))
				classhtml << " (default)";
			classhtml << "\n";
		}
		classhtml << "</ul>";
	}

	// write doxygen
	// search in annotated.html for amg< or amg&lt, use the html file
	//<tr><td class="indexkey"><a class="el" href="classug_1_1_function_pattern.html">ug::FunctionPattern</a></td><td class="indexvalue"></td></tr>


	WriteFooter(classhtml);
}

// write alphabetical class index in index.html
void WriteClassIndex(const char *dir, std::vector<UGDocuClassDescription> &classesAndGroups, bool bGroup)
{
	UG_LOG("WriteClassIndex" << (bGroup?" by group " : "") << "... ");
//	Registry &reg = GetUGRegistry();

	fstream indexhtml((string(dir).append(bGroup ? "groupindex.html" : "index.html")).c_str(), ios::out);

	if(bGroup)
	{
		WriteHeader(indexhtml, "Class Index by Group");
		indexhtml << "<h1>ug4 Class Index by Group</h1>";
		sort(classesAndGroups.begin(), classesAndGroups.end(), GroupNameSortFunction);
	}
	else
	{
		WriteHeader(indexhtml, "Class Index");
		indexhtml << "<h1>ug4 Class Index</h1>";
		sort(classesAndGroups.begin(), classesAndGroups.end(), NameSortFunction);
	}

	indexhtml 	<< "<table border=0 cellpadding=0 cellspacing=0>"
					<< "<tr><td></td></tr>";
	for(size_t i=0; i<classesAndGroups.size(); i++)
	{
		indexhtml << "<tr><td class=\"memItemLeft\" nowrap align=right valign=top>";

		UGDocuClassDescription &c = classesAndGroups[i];
		indexhtml << c.group_str();
		indexhtml << " ";
		indexhtml << "</td>";
		indexhtml << "<td class=\"memItemRight\" valign=bottom>";
		if(c.c == NULL) // group
		{
			indexhtml << "<a class=\"el\" href=\"" << c.group->get_default_class()->name() << ".html\">" << c.group->name() << "</a>\n";
			/*indexhtml << "<p> <font size=\"1\">\n";
			indexhtml << "<ul>";
			for(size_t j=0; j<c.group->num_classes(); j++)
			{
				indexhtml << "<li>" << "<a href=\"" << c.group->get_class(j)->name() << ".html\">" << c.group->get_class(j)->name() << "</a> (" << GetBeautifiedTag(c.group->get_class_tag(j)) << ")";
				if(c.group->get_default_class() == c.group->get_class(j))
					indexhtml << " (default)";
				indexhtml << "\n";
			}
			indexhtml << "</ul>";
			indexhtml << "</font></p>\n";*/
		}
		else
		{
			indexhtml << "<a class=\"el\" href=\"" << c.name() << ".html\">" << c.name() << "</a>";
		}

		indexhtml << "</td></tr>";
	}

	indexhtml 	<< "</table>";
	WriteFooter(indexhtml);
	UG_LOG(classesAndGroups.size() << " class groups written. " << endl);
}

// write alphabetical class index in index.html
void WriteGroupClassIndex(const char *dir)
{
	UG_LOG("WriteGroupClassIndex... ");
	Registry &reg = GetUGRegistry();

	vector<string> class_names;
	for(size_t i=0; i<reg.num_class_groups(); ++i)
	{
		IExportedClass *c = reg.get_class_group(i)->get_default_class();
		class_names.push_back(c->group() + "/ " + c->name());
	}

	fstream indexhtml((string(dir).append("groupindex.html")).c_str(), ios::out);

	WriteHeader(indexhtml, "Class Index by Group");
	indexhtml << "<h1>ug4 Class Index by Group</h1>";

	for(size_t i=0; i<class_names.size(); i++)
	{
		string &str = class_names[i];
		size_t pos = str.find_last_of("/");
		indexhtml << "<li>" << str.substr(0, pos) << " <a class=\"el\" href=\"" << str.substr(pos+1) << ".html\">" << str.substr(pos+1) << "</a>";
	}

	indexhtml 	<< "</table>";
	WriteFooter(indexhtml);
	UG_LOG(class_names.size() << " class names written. " );
}


// write functions index
template<typename TSortFunction>
void WriteGlobalFunctions(const char *dir, const char *filename,
		TSortFunction sortFunction)
{
	UG_LOG("WriteGlobalFunctions (" << filename << ") ... ");
	Registry &reg = GetUGRegistry();

	std::vector<const bridge::ExportedFunction *> sortedFunctions;
	for(size_t i=0; i<reg.num_functions(); i++)
	{
		ExportedFunctionGroup &fu = reg.get_function_group(i);
		for(size_t j=0; j<fu.num_overloads(); j++)
			sortedFunctions.push_back(fu.get_overload(j));
	}
	sort(sortedFunctions.begin(), sortedFunctions.end(), sortFunction);

	fstream funchtml((string(dir).append(filename)).c_str(), ios::out);
	WriteHeader(funchtml, "Global Functions Index by Group");
	funchtml 	<< "<table border=0 cellpadding=0 cellspacing=0>"
				<< "<tr><td></td></tr>";
	for(size_t i=0; i<sortedFunctions.size(); i++)
		WriteFunctionInfoHTML(funchtml, *sortedFunctions[i], sortedFunctions[i]->group().c_str());
	funchtml 	<< "</table>";
	WriteFooter(funchtml);
	UG_LOG(reg.num_functions() << " functions written." << endl);
}


void WriteConstructorCompleter(ostream &f, string classname, const bridge::ExportedConstructor &thefunc,
		string group)
{
	// function name
	f << "constructor\n" << classname << "\n";
	// returntype
	f << "\n";
	//if(pClass != NULL) f << pClass << ":";
	// signature
	f << classname << " ";
	WriteParametersIn(f, thefunc, false);	
	f << "\n";
	// html
	f << classname << " ";
	WriteParametersIn(f, thefunc);
	f << "<br>Constructor of class <b>" << classname << "</b>";
	if(thefunc.tooltip().size() > 0)
		f << "<br>tooltip: " << thefunc.tooltip();
	if(thefunc.help().size() > 0 && thefunc.help().compare("No help") != 0)
		f << "<br>help: " << thefunc.help();
	if(group.length() != 0)
		f << "<br>group: <b>" << group << "</b>";	
	f << "\n";
}

void WriteFunctionCompleter(ostream &f, const char *desc, const bridge::ExportedFunctionBase &thefunc,
		string group, const char *pClass=NULL, bool bConst=false)
{
	// function name
	f << desc << "\n" << thefunc.name() << "\n";
	// returntype
	if(thefunc.params_out().size()==0)
		f << "void\n";
	else
		f << ParameterToString(thefunc.params_out(), 0) << "\n";
	// signature
	WriteParametersOut(f, thefunc, false);
	//if(pClass != NULL) f << pClass << ":";
	f << thefunc.name() << " ";
	WriteParametersIn(f, thefunc, false);
	if(bConst) f << " const";
	f << "\n";
	// html
	WriteFunctionHTMLCompleter(f, thefunc, group.c_str(), pClass);	
	f << "\n";
}


/*int MyUGInit(int *argcp, char ***argvp, int parallelOutputProcRank)
{
	PROFILE_FUNC();
	bool success = true;

	static bool firstCall = true;
	if (firstCall) {
		firstCall = false;

#ifdef UG_PARALLEL
		pcl::Init(argcp, argvp);
		GetLogAssistant().set_output_process(parallelOutputProcRank);
#endif

		success &= InitPaths((*argvp)[0]);

#ifdef UG_BRIDGE
		try{
			bridge::InitBridge();
		}
		catch(UGError& err)
		{
			success &= false;
			UG_LOG("ERROR in UGInit: InitBridge failed!\n");
		}
#endif

#ifdef UG_PLUGINS
	#ifdef UG_EMBEDDED_PLUGINS
		InitializeEmbeddedPlugins(&bridge::GetUGRegistry(), "(Embedded Plugin) ug4/");
	#else
		if(!LoadPlugins(ug::PathProvider::get_path(PLUGIN_PATH).c_str(), "(Plugin) ug4/"))
		{
			success &= false;
			UG_LOG("ERROR in UGInit: LoadPlugins failed!\n");
		}
	#endif
#endif
	}

	// convert boolean success == true to int = 0.
	return !success;
}*/

int main(int argc, char* argv[])
{
	UGInit(&argc, &argv);
	if(FindParam("-silent", argc, argv))
		GetLogAssistant().enable_terminal_output(false);
	
	ug::script::RegisterDefaultLuaBridge(&bridge::GetUGRegistry());

	LOG("****************************************************************\n");
	LOG("* ugdocu - v0.1.0\n");
	LOG("* arguments:\n");
	LOG("*   -d directory.\toutput directory for the html files\n");
	LOG("****************************************************************\n");

	std::vector<UGDocuClassDescription> classes;
	std::vector<UGDocuClassDescription> classesAndGroups;
	std::vector<UGDocuClassDescription> classesAndGroupsAndImplementations;

	GetGroups(classes, classesAndGroups, classesAndGroupsAndImplementations);

	
	
	Registry &reg = GetUGRegistry();
	// init registry with cpualgebra and dim == 2
#if defined UG_CPU_1
	AlgebraType algebra("CPU", 1);
#elif defined UG_CRS_1
    AlgebraType algebra("CRS", 1);
#else
#error "no suitable algebra found"
#endif
	const int dim = 2;
	InitUG(dim, algebra);

	ClassHierarchy hierarchy;
	UG_LOG("GetClassHierarchy... ");
	GetClassHierarchy(hierarchy, reg);
	UG_LOG(hierarchy.subclasses.size() << " base classes, " << reg.num_class_groups() << " total. " << endl);
	
	
	const char* baseDir = NULL;	
	if(ParamToString(&baseDir, "-d", argc, argv))
	{
		// Write HTML docu
		string s_dir = baseDir;		
		if(baseDir[strlen(baseDir)-1]!='/')
			s_dir.append("/");
		const char *dir = s_dir.c_str();
		LOG("Writing html files to \"" << dir << "\"" << endl);

		// get registry



		WriteUGDocuCSS(dir);

		// print class hierarchy in hierarchy.html


		UG_LOG("WriteClassHierarchy... ");
		WriteClassHierarchy(dir, hierarchy);


		// write html file for each class
		UG_LOG(endl << "WriteClasses... ");
		for(size_t i=0; i<reg.num_classes(); ++i)
			WriteClassHTML(dir, GetUGDocuClassDescription(classes, &reg.get_class(i)), hierarchy);
		UG_LOG(reg.num_classes() << " classes written." << endl);

		WriteClassIndex(dir, classesAndGroups, false);
		WriteClassIndex(dir, classesAndGroups, true);
		//WriteGroupClassIndex(dir, classesAndGroups);

		WriteGlobalFunctions(dir, "functions.html", ExportedFunctionsSort);
		WriteGlobalFunctions(dir, "groupedfunctions.html", ExportedFunctionsGroupSort);

		UG_LOG("done." << endl);
	}

	// Write ug4CompletionList.txt 
	/*
	function
	name
	returntype
	signature
	html
	.
	class
	name
	 class hierachy
	html
	memberfunction name
	returntype
	signature
	html

	memberfunction
	name
	returntype
	signature
	html
*/	
	{
		std::string ug4CompletionFile = PathProvider::get_path(ROOT_PATH) + "/apps/ugdocu/myUg4CompletionList.txt";
		Registry &reg = GetUGRegistry();
		fstream f(ug4CompletionFile.c_str(), ios::out);
        UG_LOG("Writing completion info to " << ug4CompletionFile << " ...\n");
		f << "UG4COMPLETER VERSION 1\n";
		for(size_t i=0; i<classesAndGroupsAndImplementations.size(); i++)
		{
			//cout << classesAndGroupsAndImplementations[i].name() << "\n";
			// class\nclassname
			f << "class\n" << classesAndGroupsAndImplementations[i].name() << "\n";			
			
			// class hierachy
			const IExportedClass *c = classesAndGroupsAndImplementations[i].c;
			if(c == NULL)
				c = classesAndGroupsAndImplementations[i].group->get_default_class();			
			
			// inheritance
			if(c != NULL)
			{
				const vector<const char *> *pNames = c->class_names();
				if(pNames)
					for(vector<const char*>::const_reverse_iterator rit = pNames->rbegin(); rit < pNames->rend(); ++rit)
						if(strcmp((*rit), c->name().c_str()) != 0)
							f << (*rit) << " ";
				
			}
			f << "\n";
			
			// html			
			WriteClassCompleter(f, &classesAndGroupsAndImplementations[i], hierarchy);
			f << "\n";
			
			// memberfunctions
			
			if(c != NULL)
			{
				for(size_t i=0; i<c->num_constructors(); ++i)
					WriteConstructorCompleter(f, c->name(), c->get_constructor(i), c->group());
				
				for(size_t i=0; i<c->num_methods(); ++i)
				{
					const ExportedMethodGroup &grp = c->get_method_group(i);
					for(size_t j=0; j<grp.num_overloads(); j++)
						WriteFunctionCompleter(f, "memberfunction", *grp.get_overload(j), c->group(), c->name().c_str());
				}
				for(size_t i=0; i<c->num_const_methods(); ++i)
				{
					const ExportedMethodGroup &grp = c->get_const_method_group(i);
					for(size_t j=0; j<grp.num_overloads(); j++)
						WriteFunctionCompleter(f, "memberfunction", *grp.get_overload(j), c->group(), c->name().c_str(), true);
				}
			}
			f << ";\n";
		}
		UG_LOG("Wrote " << classesAndGroupsAndImplementations.size() << " classes/groups.\n");
		
		for(size_t i=0; i<reg.num_functions(); i++)
		{
			ExportedFunctionGroup &fg = reg.get_function_group(i);
			for(size_t j=0; j<fg.num_overloads(); j++)
				WriteFunctionCompleter(f, "function", *fg.get_overload(j), fg.get_overload(j)->group(), NULL, false);
		}
		UG_LOG("Wrote " << reg.num_functions() << " global functions.\n");
		UG_LOG("done!");
		
		if(FindParam("-silent", argc, argv))
		{
			GetLogAssistant().enable_terminal_output(true);
			cout << "Wrote ug4 completion file to " << ug4CompletionFile << ", " << classesAndGroupsAndImplementations.size() << " classes/groups, " << reg.num_functions() << " global functions.\n";
			GetLogAssistant().enable_terminal_output(false);
		}
			
	}

	
	UGFinalize();
	return 0;
}


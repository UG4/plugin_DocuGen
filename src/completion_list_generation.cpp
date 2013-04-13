/**
 * \file apps/ugdocu/src/completion_list_generation.cpp
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
#include "registry/class_helper.h"
#include "bridge/bridge.h"
#include "common/util/parameter_parsing.h"
#include "compile_info/compile_info.h"
#include "ug_docu_class_description.h"
#include "ugdocu_misc.h"
#include "html_generation.h"

#ifdef UG_PLUGINS
	#include "common/util/plugin_util.h"
	#ifdef UG_EMBEDDED_PLUGINS
		#include "embedded_plugins.h"
	#endif
#endif

using namespace std;
using namespace ug;
using namespace bridge;


namespace ug{

void WriteConstructorCompleter(ostream &f, string classname, const bridge::ExportedConstructor &thefunc,
		string group);

void WriteFunctionCompleter(ostream &f, const char *desc, const bridge::ExportedFunctionBase &thefunc,
		string group, const char *pClass=NULL, bool bConst=false);

void WriteCompletionList(std::vector<UGDocuClassDescription> classesAndGroupsAndImplementations, bool bSilent);
void WriteFunctionHTMLCompleter(ostream &file, const bridge::ExportedFunctionBase &thefunc,
		const char *group, const char *pClass);

void WriteClassCompleter(ostream &classhtml, UGDocuClassDescription *d, ClassHierarchy &hierarchy);


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
		f << "<br>Registry Group: <b>" << group << "</b>";
	f << "\n";
}

void WriteFunctionCompleter(ostream &f, const char *desc, const bridge::ExportedFunctionBase &thefunc,
		string group, const char *pClass, bool bConst)
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
void WriteCompletionList(std::vector<UGDocuClassDescription> &classesAndGroupsAndImplementations, bool bSilent, ClassHierarchy &hierarchy)
{
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
	UG_LOG("Wrote " << classesAndGroupsAndImplementations.size() << " classes/classgroups.\n");

	for(size_t i=0; i<reg.num_functions(); i++)
	{
		ExportedFunctionGroup &fg = reg.get_function_group(i);
		for(size_t j=0; j<fg.num_overloads(); j++)
			WriteFunctionCompleter(f, "function", *fg.get_overload(j), fg.get_overload(j)->group(), NULL, false);
	}
	UG_LOG("Wrote " << reg.num_functions() << " global functions.\n");
	UG_LOG("done!");

	if(bSilent)
	{
		GetLogAssistant().enable_terminal_output(true);
		cout << "Wrote ug4 completion file to " << ug4CompletionFile << ", " << classesAndGroupsAndImplementations.size() << " classes/classgroups, " << reg.num_functions() << " global functions.\n";
		GetLogAssistant().enable_terminal_output(false);
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
		file << "<br>Registry Group: <b>" << group << "</b>";
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
			classhtml << "Classgroup <b>" << d->group->name() << "</b> has no default implementation.";
			return;
		}
		classhtml << "Class <b>" << pC->name() << "</b>, default implementation of classgroup <b>"
				<< d->group->name() << "</b><br>";
	}
	else if(d->group != NULL)
		classhtml << "Classgroup <b>" << d->group->name() << "</b><br>";

	classhtml << "Registry Group: <b>" << pC->group() << "</b><br>";



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

}

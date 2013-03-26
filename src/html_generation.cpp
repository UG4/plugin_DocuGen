/**
 * \file /ug4svn/apps/ugdocu/src/html_generation.cpp
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
#include "common/util/plugin_util.h"

#include "html_generation.h"

// to refresh this file, use xxd -i ugdocu.css > ugdocu.css.h
#include "ugdocu.css.h"
// to refresh this file, use xxd -i clickEventHandler.txt > clickEventHandler.txt.h
#include "clickEventHandler.txt.h"


using namespace std;
using namespace ug;
using namespace bridge;

namespace ug{
string GetClassGroup(string classname);
template<typename T>
void remove_doubles(vector<T> &v)
{
	std::vector<T> v2;
	for(size_t i=0; i<v.size(); i++)
	{
		size_t j;
		for(j=i+1; j<v.size(); j++)
		{
			if(j == i) continue;
			if(v[i] == v[j]) break;
		}
		if(j == v.size())
			v2.push_back(v[i]);
	}
	swap(v, v2);
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
	file << clickEventHandler_txt << endl;
	file << "</head><body>" << endl;

	//file << "<div class=\"qindex\"><a class=\"qindex\" href=\"hierarchy.html\">Class Hierarchy</a>";
	file << " | <a class=\"qindex\" href=\"index.html\">Class Index</a>";
	file << " | <a class=\"qindex\" href=\"groupindex.html\">Class Index by Group</a>";
	file << " | <a class=\"qindex\" href=\"functions.html\">Global Functions</a>";
	file << " | <a class=\"qindex\" href=\"groupedfunctions.html\">Global Functions by Group</a>";
	file << " | <a class=\"qindex\" href=\"groups_index.html\">Groups</a>";
	file << "</div>" << endl;
}

void WriteFooter(fstream &file)
{

	file << "</body>" << endl << "</html>" << endl;
}


string ConstructorInfoHTML(string classname, const bridge::ExportedConstructor &thefunc,
		string group)
{
	stringstream file;

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
	return file.str();
}



string FunctionInfoHTML(const bridge::ExportedFunctionBase &thefunc,
		const IExportedClass *c, bool bConst)
{
	stringstream file;
	file << "<tr><td class=\"memItemLeft\" nowrap align=right valign=top>";
	WriteParametersOut(file, thefunc);

	file << "</td><td class=\"memItemRight\" valign=bottom>";
	if(bConst)
		file << " const ";
	if(c)
		file << "<a href=\"" << c->name() << ".html\"" << ">" << GetClassGroup(c->name()) << "</a>::";

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
	return file.str();
}



string FunctionInfoHTML(const bridge::ExportedFunctionBase &thefunc,
		const char *group)
{
	stringstream file;
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
	return file.str();
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
		vector<string> vstr;
		const bridge::ExportedFunctionBase &thefunc = reg.get_function(i);
		if((!OutParameters && IsClassInParameters(thefunc.params_in(), classname)) ||
				(OutParameters && IsClassInParameters(thefunc.params_out(), classname)))
		{
			if(bPreambleWritten==false) { file << preamble; bPreambleWritten=true; }
			vstr.push_back(FunctionInfoHTML(thefunc));
		}
		remove_doubles(vstr);
		for(vector<string>::iterator it = vstr.begin(); it != vstr.end(); ++it)
			file << *it;

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
					file << FunctionInfoHTML(*thefunc, &c, false);
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
					FunctionInfoHTML(*thefunc, &c, true);
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
		file << GetClassGroup(c->name()) << " Constructors</h3></td></tr>";
		for(size_t i=0; i<c->num_constructors(); ++i)
			file << ConstructorInfoHTML(c->name(), c->get_constructor(i), c->group());
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
		file << GetClassGroup(c->name()) << " Member Functions</h3></td></tr>";
		for(size_t i=0; i < sortedFunctions.size(); ++i)
			file << FunctionInfoHTML(*sortedFunctions[i]);
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
		file << GetClassGroup(c->name()) << " Const Member Functions</h3></td></tr>";

		for(size_t i=0; i < sortedFunctions.size(); ++i)
			file << FunctionInfoHTML(*sortedFunctions[i]);
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


	classhtml << "<br>Group <b><a href=\"" << GetFilenameForGroup(c.group(), dir) << "\">" << c.group() << "</a></b><br>";

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
			classhtml << "<li><a class=\"el\" href=\"" << (*rit) << ".html\">" << GetClassGroup(*rit) << "</a>";
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
			indexhtml << "<a class=\"el\" href=\"" << c.name() << ".html\">" << c.name() << "</a>\n";
		}

		indexhtml << "</td></tr>";
	}

	indexhtml 	<< "</table>";
	WriteFooter(indexhtml);
	UG_LOG(classesAndGroups.size() << " class groups written. " << endl);
}


void GetGroups(std::map<string, UGRegistryGroup> &g);

// write alphabetical class index in index.html
void WriteGroups(const char *dir, std::vector<UGDocuClassDescription> &classesAndGroups)
{
	UG_LOG("WriteGroups...");
//	Registry &reg = GetUGRegistry();

	std::map<string, UGRegistryGroup> groups;
	GetGroups(groups);

	std::map<string, string> groupsstring;

	for(map<string, UGRegistryGroup>::iterator it = groups.begin(); it != groups.end(); ++it)
	{
		string g = it->first;
		UG_LOG("group '" << g << "'\n");
		stringstream ss;
		ss << "\n";
		ss<< "<h1>Group " << g << "</h1>\n";

		ss << "<h2>Classes</h2>\n";
		ss << "<table border=0 cellpadding=0 cellspacing=0><tr><td></td></tr>";

		for(size_t i=0; i< it->second.classesAndGroups.size(); i++)
		{
			UGDocuClassDescription &c = it->second.classesAndGroups[i];
			//if(strcmp(c.group_str().c_str(), g.c_str()) != 0) continue;
			ss << "<tr><td class=\"memItemLeft\" nowrap align=right valign=top>";

			ss << c.group_str();
			ss << " ";
			ss << "</td>";
			ss << "<td class=\"memItemRight\" valign=bottom>";
			if(c.c == NULL) // group
				ss << "<a class=\"el\" href=\"" << c.group->get_default_class()->name() << ".html\">" << c.group->name() << "</a>\n";
			else
				ss << "<a class=\"el\" href=\"" << c.name() << ".html\">" << c.name() << "</a>";
			ss << "</td></tr>\n";
		}

		ss << "</table>";
		ss << "<h2>Functions</h2>\n";
		ss << "<table border=0 cellpadding=0 cellspacing=0>"
									<< "<tr><td></td></tr>";
		vector<string> vstr;
		for(size_t i=0; i< it->second.functions.size(); i++)
		{
			ExportedFunction *e = it->second.functions[i];
			vstr.push_back(FunctionInfoHTML(*e, g.c_str()));
		}
		remove_doubles(vstr);
		for(vector<string>::iterator it = vstr.begin(); it != vstr.end(); ++it)
			ss << *it;
		ss << "</table>";

		fstream f(GetFilenameForGroup(g, dir).c_str(), ios::out);
		WriteHeader(f, (string("Group ")+g).c_str());
		f << ss.str();
		WriteFooter(f);
		groupsstring[it->first] = ss.str();
	}

	fstream indexhtml((string(dir).append("groups_index.html")).c_str(), ios::out);

	WriteHeader(indexhtml, "Groups");
	indexhtml << "<h1>Groups</h1>\n";
	indexhtml << "<ul id=\"LinkedList1\" class=\"LinkedList\">\n";
	for(map<string, UGRegistryGroup>::iterator it = groups.begin(); it != groups.end(); ++it)
	{
		string g = it->first;
		if(IsPluginGroup(g)==true) continue;
		string filename = GetFilenameForGroup(g, dir);
		if(g.compare("") == 0) g = "(empty group)";
		//indexhtml << "<li><a class=\"el\" href=\"" << filename << "\">" << g << "</a>\n";
		indexhtml << "<li>" << g << "\n";
		indexhtml << "<ul><li>" << groupsstring[it->first] << "</ul>\n";
	}
	indexhtml << "</ul>\n";
	indexhtml << "<h1>Plugin Groups</h1>\n";
	indexhtml << "<ul id=\"LinkedList2\" class=\"LinkedList\">\n";
	//indexhtml << "<ul>";
	for(map<string, UGRegistryGroup>::iterator it = groups.begin(); it != groups.end(); ++it)
	{
		string g = it->first;
		if(IsPluginGroup(g)==false) continue;
		string filename = GetFilenameForGroup(g, dir);
		//indexhtml << "<li><a class=\"el\" href=\"" << filename << "\">" << g << "</a>\n";
		//indexhtml << "<ul><li>test<li>test2<li>test3</ul>";
		indexhtml << "<li>" << g << "\n";
		indexhtml << "<ul><li>" << groupsstring[it->first] << "</ul>\n";
	}

	indexhtml 	<< "</ul>";
	WriteFooter(indexhtml);
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
	vector<string> vstr;
	for(size_t i=0; i<sortedFunctions.size(); i++)
		vstr.push_back(FunctionInfoHTML(*sortedFunctions[i], sortedFunctions[i]->group().c_str()));
	remove_doubles(vstr);
	for(vector<string>::iterator it = vstr.begin(); it != vstr.end(); ++it)
		funchtml << *it;
	funchtml 	<< "</table>";
	WriteFooter(funchtml);

	UG_LOG(reg.num_functions() << " functions written." << endl);
}


void WriteHTMLDocu(std::vector<UGDocuClassDescription> &classes, std::vector<UGDocuClassDescription> &classesAndGroups, const char *dir, ClassHierarchy &hierarchy)
{
	Registry &reg = GetUGRegistry();

	WriteUGDocuCSS(dir);

	UG_LOG("WriteClassHierarchy... ");
	WriteClassHierarchy(dir, hierarchy);

	// write html file for each class
	UG_LOG(endl << "WriteClasses... ");
	for(size_t i=0; i<reg.num_classes(); ++i)
		WriteClassHTML(dir, GetUGDocuClassDescription(classes, &reg.get_class(i)), hierarchy);
	UG_LOG(reg.num_classes() << " classes written." << endl);

	WriteClassIndex(dir, classesAndGroups, false);
	WriteClassIndex(dir, classesAndGroups, true);
	WriteGroups(dir, classesAndGroups);
	//WriteGroupClassIndex(dir, classesAndGroups);

	WriteGlobalFunctions(dir, "functions.html", ExportedFunctionsSort);
	WriteGlobalFunctions(dir, "groupedfunctions.html", ExportedFunctionsGroupSort);

	UG_LOG("done." << endl);
}


} // namespace ug

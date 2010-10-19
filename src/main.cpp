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

#include "ug_script/ug_script.h"

#include <stdio.h>
#include <time.h>

using namespace std;
using namespace ug;
using namespace bridge;

// to refresh this file, use   xxd -i ugdocu.css > ugdocu.css.h
#include "ugdocu.css.h";

namespace ug{

namespace bridge
{
extern bool IsClassInParameters(const bridge::ParameterStack &par, const char *classname);
extern const IExportedClass *FindClass(const char* classname);
}


} // namespace ug


struct ClassHierarchy
{
	string name;
	vector<ClassHierarchy> subclasses;
};


string tohtmlstring(const string &str)
{
	return str;
}

void WriteHeader(fstream &file, const string &title)
{
	file << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">" << endl;
	file << "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=iso-8859-1\">";
	file << "<title>" << tohtmlstring(title) << "</title>" << endl;
	file << "<link href=\"ugdocu.css\" rel=\"stylesheet\" type=\"text/css\">" << endl;
	file << "</head><body>" << endl;

	file << "<div class=\"qindex\"><a class=\"qindex\" href=\"hierarchy.html\">Class Hierarchy</a>";
	file << " | <a class=\"qindex\" href=\"index.html\">Class Index</a>";
	file << " | <a class=\"qindex\" href=\"functions.html\">Global Functions</a>";
	file << "</div>" << endl;
}

void WriteFooter(fstream &file)
{
	file << "<hr size=\"1\"><address style=\"align: right;\"><small>";

	time_t now = time(0);
	tm *local = localtime(&now);
	file << "Generated on " << asctime(local);
	file << "</small></address>" << endl;

	file << "</body>" << endl << "</html>" << endl;
}

bool ParameterToHTMLString(fstream &file, const bridge::ParameterStack &par, int i)
{
	switch(par.get_type(i))
	{
	default:
	case PT_UNKNOWN:
		file << "unknown";
		return true;
	case PT_BOOL:
		file << "bool";
		return true;

	case PT_INTEGER:
		file << "integer";
		return true;

	case PT_NUMBER:
		file << "number";
		return true;

	case PT_STRING:
		file << "string";
		return true;

	case PT_POINTER:
		file << "<a href=\"" << par.class_name(i) << ".html\"" << ">" << par.class_name(i) << "</a> *";
		return true;

	case PT_CONST_POINTER:
		file << "const <a href=\"" << par.class_name(i) << ".html\"" << ">" << par.class_name(i) << "</a> *";
		return true;
	}
	return false;
}

bool WriteParameters(fstream &file, const bridge::ParameterStack &par)
{
	file << "(";
	for(int i=0; i<par.size(); ++i)
	{
		if(i>0) file << ", ";
		ParameterToHTMLString(file, par, i);
	}
	file << ")";
	return true;
}

void WriteFunctionInfo(fstream &file, const bridge::ExportedFunctionBase &thefunc,
		const IExportedClass *c = NULL, bool bConst = false)
{
	file << "<tr><td class=\"memItemLeft\" nowrap align=right valign=top>";
	const bridge::ParameterStack &parout = thefunc.params_out();
	if(parout.size() > 0)
		WriteParameters(file, parout);

	file << "</td><td class=\"memItemRight\" valign=bottom>";
	if(bConst)
		file << " const ";
	if(c)
		file << "<a href=\"" << c->name() << ".html\"" << ">" << c->name() << "</a>::";

	file << thefunc.name() << " ";

	WriteParameters(file, thefunc.params_in());
	file << "</td></tr>" << endl;
}

void InsertClass(ClassHierarchy &classes, const IExportedClass &c)
{
	const vector<const char *> *pNames = c.class_names();

	if(pNames == NULL) return;

	ClassHierarchy *base = &classes;

	for(vector<const char*>::const_reverse_iterator rit = pNames->rbegin(); rit < pNames->rend(); ++rit)
	{
		const char *thename = (*rit);
		size_t j;
		for(j=0; j<base->subclasses.size(); j++)
		{
			if(base->subclasses.at(j).name.compare(thename) == 0)
			{
				base = &base->subclasses.at(j);
				break;
			}
		}

		if(j == base->subclasses.size())
		{
			ClassHierarchy newclass;
			newclass.name = thename;
			base->subclasses.push_back(newclass);
		}
	}
}

ClassHierarchy *FindClassInHierarchy(ClassHierarchy &classes, const char *name)
{
	if(classes.name.compare(name) == 0)
		return &classes;
	for(size_t i=0; i<classes.subclasses.size(); i++)
	{
		ClassHierarchy *c = FindClassInHierarchy(classes.subclasses[i], name);
		if(c) return c;
	}
	return NULL;
}

void WriteClassHierarchy(fstream &file, ClassHierarchy &c)
{
	file << "<li><a class=\"el\" href=\"" << c.name << ".html\">" << c.name << "</a>" << endl;
	if(c.subclasses.size())
	{
		file << "<ul>" << endl;
		for(size_t i=0; i<c.subclasses.size(); i++)
			WriteClassHierarchy(file, c.subclasses[i]);
		file << "</ul>" << endl;
	}
}


/**
 *
 * \param classname the class (and only this class) to print usage in functions/member functions of.
 */
bool WriteClassUsageExact(fstream &file, const char *classname, bool OutParameters)
{
	bridge::Registry &reg = GetUGRegistry();
	// check functions
	for(size_t i=0; i<reg.num_functions(); i++)
	{
		const bridge::ExportedFunctionBase &thefunc = reg.get_function(i);
		if(!OutParameters && IsClassInParameters(thefunc.params_in(), classname) ||
				OutParameters && IsClassInParameters(thefunc.params_out(), classname))
			WriteFunctionInfo(file, thefunc);
	}

	// check classes
	for(size_t i=0; i<reg.num_classes(); i++)
	{
		const IExportedClass &c = reg.get_class(i);
		for(size_t i=0; i<c.num_methods(); i++)
		{
			const bridge::ExportedFunctionBase &thefunc = c.get_method(i);
			if(!OutParameters && IsClassInParameters(thefunc.params_in(), classname) ||
					OutParameters && IsClassInParameters(thefunc.params_out(), classname))
				WriteFunctionInfo(file, thefunc, &c, false);

		}

		for(size_t i=0; i<c.num_const_methods(); i++)
		{
			const bridge::ExportedFunctionBase &thefunc = c.get_const_method(i);
			if(!OutParameters && IsClassInParameters(thefunc.params_in(), classname) ||
					OutParameters && IsClassInParameters(thefunc.params_out(), classname))
				WriteFunctionInfo(file, thefunc, &c, true);
		}
	}
	return true;
}

void PrintClassFunctionsHMTL(fstream &file, const IExportedClass *c, bool bInherited)
{
	if(c->num_methods() > 0)
	{
		file << "<tr><td colspan=2><br><h2>";
		if(bInherited) file << " Inherited ";
		file << c->name() << " Member Functions</h2></td></tr>" << endl;
		for(size_t k=0; k<c->num_methods(); ++k)
			WriteFunctionInfo(file, c->get_method(k));
	}

	if(c->num_const_methods() > 0)
	{
		file << "<tr><td colspan=2><br><h2>";
		if(bInherited) file << " Inherited ";
		file << c->name() << " Const Member Functions</h2></td></tr>" << endl;

		for(size_t k=0; k<c->num_const_methods(); ++k)
			WriteFunctionInfo(file, c->get_const_method(k));
	}
}

// write the ugdocu.css
void WriteUGDocuCSS(const char *dir)
{
	UG_LOG("WriteUGDocuCSS... ");
	fstream ugdocucss((string(dir).append("ugdocu.css")).c_str(), ios::out);
	ugdocucss.write((char *)ugdocu_css, ugdocu_css_len);
	UG_LOG(ugdocu_css_len << " characters." << endl);
}


void GetClassHierarchy(ClassHierarchy &hierarchy)
{
	hierarchy.subclasses.clear();
	hierarchy.name = "UGBase";
	bridge::Registry &reg = GetUGRegistry();
	for(size_t i=0; i<reg.num_classes(); ++i)
		InsertClass(hierarchy, reg.get_class(i));
}

// print class hierarchy in hierarchy.html
void WriteClassHierarchy(const char *dir, ClassHierarchy &hierarchy)
{
	fstream hierarchyhtml((string(dir).append("hierarchy.html")).c_str(), ios::out);
	WriteHeader(hierarchyhtml, "Class Hierarchy");
	hierarchyhtml << "<h1>ug4 Class Hierarchy</h1>This inheritance list is not sorted:<ul>" << endl;
	for(size_t i=0; i<hierarchy.subclasses.size(); i++)
		WriteClassHierarchy(hierarchyhtml, hierarchy.subclasses[i]);
	hierarchyhtml << "</ul>"<<endl;
	WriteFooter(hierarchyhtml);
}

// write html file for a class
void WriteClass(const char *dir, const IExportedClass &c, ClassHierarchy &hierarchy)
{
	string name = c.name();

	fstream classhtml((string(dir) + name + ".html").c_str(), ios::out);
	WriteHeader(classhtml, name);

	classhtml 	<< "<h1>" << name << " Class Reference</h1>" << endl;

	// print parent classes
	const vector<const char *> *pNames = c.class_names();
	if(pNames && pNames->size() > 1)
	{
		for(vector<const char*>::const_reverse_iterator rit = pNames->rbegin(); rit < pNames->rend(); ++rit)
		{
			classhtml << "<ul>" << endl;
			classhtml << "<li><a class=\"el\" href=\"" << (*rit) << ".html\">" << (*rit) << "</a>" << endl;
		}
		for(size_t i=0; i<pNames->size(); i++)
			classhtml << "</ul>" << endl;
	}

	// print member functions

	classhtml 	<< "<table border=0 cellpadding=0 cellspacing=0>" << endl
				<< "<tr><td></td></tr>" << endl;

	PrintClassFunctionsHMTL(classhtml, &c, false);
	if(pNames)
	{
		// print inherited member functions
		for(size_t i=1; i<pNames->size(); i++)
			PrintClassFunctionsHMTL(classhtml, FindClass(pNames->at(i)), true);
	}

	classhtml << "</table>" << endl;

	// print usage of this class in other classes

	classhtml 	<< "<h1>Usage Information</h1>" << endl;

	classhtml 	<< "<table border=0 cellpadding=0 cellspacing=0>" << endl
						<< "<tr><td></td></tr>" << endl;

	// functions returning this class
	classhtml 	<< "<tr><td colspan=2><br><h2> Functions returning " << name << "</h2></td></tr>" << endl;
	WriteClassUsageExact(classhtml, name.c_str(), true);

	// functions using this class or its parents
	if(pNames)
	{
		for(size_t i=0; i<pNames->size(); i++)
		{
			classhtml << "<tr><td colspan=2><br><h2> Functions using " << pNames->at(i) << "</h2></td></tr>" << endl;
			WriteClassUsageExact(classhtml, pNames->at(i), false);
		}
	}
	else
		WriteClassUsageExact(classhtml, name.c_str(), false);
	classhtml << "</table>" << endl;

	// print subclasses of this class
	ClassHierarchy *theclass = FindClassInHierarchy(hierarchy, name.c_str());
	if(theclass && theclass->subclasses.size() > 0)
	{
		classhtml 	<< "<h1>Subclasses</h1><ul>" << endl;
		WriteClassHierarchy(classhtml, *theclass);
		classhtml << "</ul>";
	}

	WriteFooter(classhtml);
}

// write alphabetical class index in index.html
void WriteClassIndex(const char *dir)
{
	UG_LOG("WriteClassIndex... ")
	bridge::Registry &reg = GetUGRegistry();

	vector<string> class_names;
	for(size_t i=0; i<reg.num_classes(); ++i)
		class_names.push_back(reg.get_class(i).name());
	sort(class_names.begin(), class_names.end());


	fstream indexhtml((string(dir).append("index.html")).c_str(), ios::out);

	WriteHeader(indexhtml, "Class Index");
	indexhtml << "<h1>ug4 Class Index</h1>" << endl;
	for(size_t i=0; i<class_names.size(); i++)
		indexhtml << "<li><a class=\"el\" href=\"" << class_names[i] << ".html\">" << class_names[i] << "</a>" << endl;
	WriteFooter(indexhtml);
	UG_LOG(class_names.size() << " class names written. " << endl);
}

// write functions index (functions.html). todo: sort alphabetically
void WriteGlobalFunctions(const char *dir)
{
	UG_LOG("WriteGlobalFunctions...")
	bridge::Registry &reg = GetUGRegistry();

	fstream funchtml((string(dir).append("functions.html")).c_str(), ios::out);
	WriteHeader(funchtml, "Global Functions Index");
	funchtml 	<< "<table border=0 cellpadding=0 cellspacing=0>" << endl
				<< "<tr><td></td></tr>" << endl;
	for(size_t i=0; i<reg.num_functions(); i++)
		WriteFunctionInfo(funchtml, reg.get_function(i));
	funchtml 	<< "</table>" << endl;
	WriteFooter(funchtml);
	UG_LOG(reg.num_functions() << " functions written." << endl);
}

int main(int argc, char* argv[])
{
	UGInit(argc, argv, 0);

	LOG("****************************************************************\n");
	LOG("* ugdocu - v0.1.0\n");
	LOG("* arguments:\n");
	LOG("*   -d directory.\toutput directory for the html files\n");
	LOG("****************************************************************\n");

	int dirParamIndex = GetParamIndex("-d", argc, argv);

	if(dirParamIndex == -1)
	{
		LOG("error: no directory. Please specify directory.\n");
		UGFinalize();
		return 0;
	}
	const char *dir = argv[dirParamIndex+1];
	LOG("Writing html files to \"" << dir << "\"" << endl);

	bridge::Registry &reg = GetUGRegistry();


	WriteUGDocuCSS(dir);

	// print class hierarchy in hierarchy.html
	ClassHierarchy hierarchy;
	UG_LOG("GetClassHierarchy... ")
	GetClassHierarchy(hierarchy);

	UG_LOG(hierarchy.subclasses.size() << " base classes, " << reg.num_classes() << " total. " << endl);
	UG_LOG("WriteClassHierarchy... ");
	WriteClassHierarchy(dir, hierarchy);


	// write html file for each class
	UG_LOG(endl << "WriteClasses... ")
	for(size_t i=0; i<reg.num_classes(); ++i)
		WriteClass(dir, reg.get_class(i), hierarchy);
	UG_LOG(reg.num_classes() << " classes written." << endl);

	WriteClassIndex(dir);

	WriteGlobalFunctions(dir);
	UG_LOG("done." << endl);

	UGFinalize();
}


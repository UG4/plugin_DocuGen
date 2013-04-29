/**
 * \file apps/ugdocu/src/html_generation.h
 *
 *  \date	 	25.03.2013
 *  \author 	mrupp
 */

#ifndef __UG__HTML_GENERATION_H__
#define __UG__HTML_GENERATION_H__

#include <string>
#include "bridge/bridge.h"
#include "ugdocu_misc.h"
#include "ug_docu_class_description.h"
#include "registry/class_helper.h"

namespace ug
{
namespace regDocu
{

void WriteHeader(std::fstream &file, const std::string &title);
void WriteFooter(std::fstream &file);
void WriteUGDocuCSS(const char *dir);
std::string ConstructorInfoHTML(std::string classname, const bridge::ExportedConstructor &thefunc,	std::string group);
std::string FunctionInfoHTML(const bridge::ExportedFunctionBase &thefunc, const bridge::IExportedClass *c = NULL, bool bConst = false);
std::string FunctionInfoHTML(const bridge::ExportedFunctionBase &thefunc,	const char *group);
bool WriteClassUsageExact(const std::string &preamble, std::ostream &file, const char *classname, bool OutParameters);
void PrintClassFunctionsHMTL(std::ostream &file, const bridge::IExportedClass *c, bool bInherited);

void WriteClassHierarchy(const char *dir, bridge::ClassHierarchy &hierarchy);
void WriteClassHTML(const char *dir, UGDocuClassDescription *d, bridge::ClassHierarchy &hierarchy);
void WriteClassIndex(const char *dir, std::vector<UGDocuClassDescription> &classesAndGroups, bool bGroup);

//void WriteGlobalFunctions(const char *dir, const char *filename, bool sortFunction(const bridge::ExportedFunction *,const bridge::ExportedFunction *j));

void WriteHTMLDocu(std::vector<UGDocuClassDescription> &classes, std::vector<UGDocuClassDescription> &classesAndGroups, const char *dir, bridge::ClassHierarchy &hierarchy);

}	// namespace regDocu
}	// namespace ug

#endif /* __UG__HTML_GENERATION_H__ */

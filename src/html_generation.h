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

/// \addtogroup apps_ugdocu
/// \{

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

// end group apps_ugdocu
/// \}

}	// namespace regDocu
}	// namespace ug

#endif /* __UG__HTML_GENERATION_H__ */

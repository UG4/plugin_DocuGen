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

#ifndef __UG__UGDOCU_MISC_H__
#define __UG__UGDOCU_MISC_H__

#include "ug_docu_class_description.h"
#include "registry/class_helper.h"

namespace ug
{
namespace regDocu
{

/// \addtogroup apps_ugdocu
/// \{

bool IsPluginGroup(std::string g);
std::string GetFilenameForGroup(std::string s, std::string dir="");
bool ClassGroupDescSort(const bridge::ClassGroupDesc *i, const bridge::ClassGroupDesc *j);
bool ExportedClassSort(const bridge::IExportedClass *i, const bridge::IExportedClass *j);

bool ExportedFunctionsSort(const bridge::ExportedFunctionBase * i,
		const bridge::ExportedFunctionBase *j);
bool ExportedFunctionsGroupSort(const bridge::ExportedFunction * i,
		const bridge::ExportedFunction *j);

std::string tohtmlstring(const std::string &str);

void ParameterToString(std::ostream &file, const bridge::ParameterInfo &par, int i, bool bHTML);

template<typename T>
static void WriteParametersIn(std::ostream &file, const T &thefunc, bool bHTML=true)
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
}
template<typename T>
static void WriteParametersOut(std::ostream &file, const T &thefunc, bool bHTML=true)
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
}


void WriteClassHierarchy(std::ostream &file, bridge::ClassHierarchy &c);


class UGRegistryGroup
{
public:
	std::vector<UGDocuClassDescription> classesAndGroups;
	std::vector<bridge::ExportedFunction*> functions;
};


std::string GetBeautifiedTag(std::string tag);

// end group apps_ugdocu
/// \}

}	// namespace regDocu
}	// namespace ug

#endif /* __UG__UGDOCU_MISC_H__ */

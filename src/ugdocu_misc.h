/**
 * \file apps/ugdocu/src/ugdocu_misc.h
 *
 *  \date	 	25.03.2013
 *  \author 	mrupp
 */

#ifndef __UG__UGDOCU_MISC_H__
#define __UG__UGDOCU_MISC_H__

#include "ug_docu_class_description.h"
#include "registry/class_helper.h"

namespace ug{

bool IsPluginGroup(std::string g);
std::string GetFilenameForGroup(std::string s, std::string dir="");
bool ClassGroupDescSort(const bridge::ClassGroupDesc *i, const bridge::ClassGroupDesc *j);
bool ExportedClassSort(const bridge::IExportedClass *i, const bridge::IExportedClass *j);

bool ExportedFunctionsSort(const bridge::ExportedFunctionBase * i,
		const bridge::ExportedFunctionBase *j);
bool ExportedFunctionsGroupSort(const bridge::ExportedFunction * i,
		const bridge::ExportedFunction *j);

std::string tohtmlstring(const std::string &str);

bool ParameterToString(std::ostream &file, const bridge::ParameterInfo &par, int i, bool bHTML);

template<typename T>
static bool WriteParametersIn(std::ostream &file, const T &thefunc, bool bHTML=true)
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
static bool WriteParametersOut(std::ostream &file, const T &thefunc, bool bHTML=true)
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


void WriteClassHierarchy(std::ostream &file, bridge::ClassHierarchy &c);


class UGRegistryGroup
{
public:
	std::vector<UGDocuClassDescription> classesAndGroups;
	std::vector<bridge::ExportedFunction*> functions;
};


std::string GetBeautifiedTag(std::string tag);



} // namespace ug

#endif /* __UG__UGDOCU_MISC_H__ */

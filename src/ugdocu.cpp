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
#include "common/util/plugin_util.h"
#include "ugdocu_misc.h"
#include "html_generation.h"

using namespace std;
using namespace ug;
using namespace bridge;



namespace ug {

namespace bridge
{
extern bool IsClassInParameters(const bridge::ParameterInfo &par, const char *classname);
}

void WriteCompletionList(std::vector<UGDocuClassDescription> &classesAndGroupsAndImplementations, bool bSilent, ClassHierarchy &hierarchy);

} // namespace ug



int MyUGInit(int *argcp, char ***argvp, int parallelOutputProcRank=-1)
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
		InitializeEmbeddedPlugins(&bridge::GetUGRegistry(), "(Plugin) ug4/");
	#else
		if(!ug::LoadPlugins(ug::PathProvider::get_path(PLUGIN_PATH).c_str(), "(Plugin) ug4/"))
		{
			success &= false;
			UG_LOG("ERROR in UGInit: LoadPlugins failed!\n");
		}
	#endif
#endif
	}

	// convert boolean success == true to int = 0.
	return !success;
}

std::vector<UGDocuClassDescription> classes;
std::vector<UGDocuClassDescription> classesAndGroups;
std::vector<UGDocuClassDescription> classesAndGroupsAndImplementations;

int main(int argc, char* argv[])
{
	MyUGInit(&argc, &argv);
	if(FindParam("-silent", argc, argv))
		GetLogAssistant().enable_terminal_output(false);

	ug::script::RegisterDefaultLuaBridge(&bridge::GetUGRegistry());

	LOG("****************************************************************\n");
	LOG("* ugdocu - v0.1.1\n");
	LOG("* arguments:\n");
	LOG("*   -d directory.\toutput directory for the html files\n");
	LOG("****************************************************************\n");



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
		WriteHTMLDocu(classes, classesAndGroups, dir, hierarchy);

	}

	WriteCompletionList(classesAndGroupsAndImplementations, FindParam("-silent", argc, argv), hierarchy);

	
	UGFinalize();
	return 0;
}

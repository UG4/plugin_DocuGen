/**
 * \file apps/ugdocu/src/ugdocu.cpp
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
#include "common/util/parameter_parsing.h"
#include "compile_info/compile_info.h"

#ifdef UG_BRIDGE
# include "bridge/bridge.h"
#endif

#ifdef UG_PLUGINS
# include "common/util/plugin_util.h"
# ifdef UG_EMBEDDED_PLUGINS
#  include "embedded_plugins.h"
# endif
#endif

#include "ugdocu_misc.h"
#include "html_generation.h"
#include "cpp_generator.h"

using namespace std;
using namespace ug;
using namespace bridge;



namespace ug
{

namespace bridge
{
extern bool IsClassInParameters(const bridge::ParameterInfo &par, const char *classname);
}


namespace regDocu
{

void WriteCompletionList(std::vector<UGDocuClassDescription> &classesAndGroupsAndImplementations, bool bSilent, ClassHierarchy &hierarchy);

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
		std::cout << "Using Embedded Plugins.\n");
		InitializeEmbeddedPlugins(&bridge::GetUGRegistry(), "(Plugin) ug4/");
	#else
		std::string pluginpath = ug::PathProvider::get_path(PLUGIN_PATH);
		std::cout << "Using Plugin Path " << pluginpath <<"\n";
		if(!ug::LoadPlugins(pluginpath.c_str(), "(Plugin) ug4/"))
		{
			success &= false;
			UG_LOG("ERROR in UGInit: LoadPlugins failed!\n");
		}
	#endif
#else
	std::cout << "WARNING: UG_PLUGINS NOT DEFINED!!! Plugins will not be loaded.\n";
#endif
	}

	// convert boolean success == true to int = 0.
	return !success;
}

vector<UGDocuClassDescription> classes;
vector<UGDocuClassDescription> classesAndGroups;
vector<UGDocuClassDescription> classesAndGroupsAndImplementations;

}	// namespace regDocu
}	// namespace ug

int main(int argc, char* argv[])
{
	regDocu::MyUGInit(&argc, &argv);
	if(FindParam("-silent", argc, argv))
		GetLogAssistant().enable_terminal_output(false);

	ug::script::RegisterDefaultLuaBridge(&bridge::GetUGRegistry());

	LOG("****************************************************************\n");
	LOG("* ugdocu - v0.2.0\n");
	LOG("* arguments:\n");
	LOG("*   -d       output directory for the html/c++ files\n");
	LOG("*   -html    generate legacy HTML bridge docu\n");
// #ifdef UG_CXX11
	LOG("*   -cpp     generate dummy C++ sources of registered entities\n");
// #endif
	LOG("*   -list    generate completion list\n");
	LOG("*   -silent  don't print verbose progress info\n");
	LOG("****************************************************************\n");

	const char* baseDir = NULL;
	if ( ! ParamToString( &baseDir, "-d", argc, argv ) ) {
		UG_THROW( "No output directory given. Mandatory. Abborting!" );
	}
	string s_dir = baseDir;
	if ( baseDir[strlen(baseDir)-1] != '/' ) {
		s_dir.append( "/" );
	}
	const char *dir = s_dir.c_str();
	
	bool generate_html = FindParam( "-html", argc, argv );
// #ifdef UG_CXX11
	bool generate_cpp = FindParam( "-cpp", argc, argv );
// #endif
	bool generate_list = FindParam( "-list", argc, argv );
	
	bool silent = FindParam( "-silent", argc, argv );
	
	if ( generate_html || generate_list ) {
		regDocu::GetGroups(regDocu::classes, regDocu::classesAndGroups, regDocu::classesAndGroupsAndImplementations);
	}
	
	Registry &reg = GetUGRegistry();
	ClassHierarchy hierarchy;
	
	// 	init registry with cpualgebra and dim == 2
#if defined UG_CPU_1
	AlgebraType algebra("CPU", 1);
#elif defined UG_CRS_1
	AlgebraType algebra("CRS", 1);
#else
# error "No suitable Algebra found."
#endif
	const int dim = 2;
	InitUG(dim, algebra);

	if ( generate_html || generate_list ) {
		GetClassHierarchy( hierarchy, reg );
		UG_LOG("GetClassHierarchy... ");
		UG_LOG(hierarchy.subclasses.size() << " base classes, " << reg.num_class_groups() << " total. " << endl);
	}
	
	if ( generate_html ) {
		// Write HTML docu
		LOG("Writing html files to \"" << dir << "\"" << endl);
		regDocu::WriteHTMLDocu(regDocu::classes, regDocu::classesAndGroups, dir, hierarchy);
	}
	
// #ifdef UG_CXX11
	if ( generate_cpp ) {
		// Write C++ files
		regDocu::CppGenerator cppgen( dir, silent );
		cppgen.generate_cpp_files();
	}
// #endif

	if ( generate_list ) {
		regDocu::WriteCompletionList(regDocu::classesAndGroupsAndImplementations, silent, hierarchy);
	}
	
	UGFinalize();
	return 0;
}

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
#include "class_hierarchy_provider.h"
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

/**
 * \brief App for generating documentation of registered classes and methods
 */
namespace regDocu
{

/**
 * \defgroup apps_ugdocu UGRegistry Documentation
 * \ingroup apps
 * \brief App for generating documentation of registered classes and methods
 * \details This app generates the documentation of all registered classes and 
 *   methods of the linked libug4.
 *   Depending on command line parameters either HTML or C++ files are generated.
 * 
 *   <b>Usage:</b>
 *   
 *       ugdocu -d <path> [-html] [-cpp] [-list] [-silent]
 *   
 *   <b>Parameters:</b>
 *   - `-d` (required)<br />
 *     path of the output directory for the html, cpp files and completion list
 *   - `-html` (optional)<br />
 *     generates legacy HTML docu
 *   - `-cpp` (optional)<br />
 *     generates dummy C++ code from registered functions ready to be parsed
 *     by Doxygen
 *   - `-list` (optional)<br />
 *     generates completion list
 *   - `-silent` (optional)<br />
 *     don't print verbose progress info; only warnings
 * \{
 */

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

// end group apps_ugdocu
/// \}

}	// namespace regDocu
}	// namespace ug

/// \addtogroup apps_ugdocu
int main(int argc, char* argv[])
{
	regDocu::MyUGInit(&argc, &argv);
	if(FindParam("-silent", argc, argv))
		GetLogAssistant().enable_terminal_output(false);
	try
	{

		ug::script::RegisterDefaultLuaBridge(&bridge::GetUGRegistry());
	
		LOG("****************************************************************\n");
		LOG("* ugdocu - v0.2.0\n");
		LOG("* arguments:\n");
		LOG("*   -d       output directory for the html/c++ files\n");
		LOG("*   -html    generate legacy HTML bridge docu\n");
		LOG("*   -cpp     generate dummy C++ sources of registered entities\n");
		LOG("*   -list    generate completion list\n");
		LOG("*   -silent  don't print verbose progress info\n");
		LOG("****************************************************************\n");
	

		string dir=".";

		bool generate_html = FindParam( "-html", argc, argv );
		bool generate_cpp = FindParam( "-cpp", argc, argv );
		bool generate_list = FindParam( "-list", argc, argv );

		if ((generate_html || generate_cpp))
		{
			const char* baseDir = NULL;
			if(! ParamToString( &baseDir, "-d", argc, argv ) )
			{	UG_THROW( "No output directory given. Mandatory. Abborting!" ); }
			dir = baseDir;
			if ( baseDir[strlen(baseDir)-1] != '/' ) {
				dir.append( "/" );
			}
		}
	
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
			regDocu::WriteHTMLDocu(regDocu::classes, regDocu::classesAndGroups, dir.c_str(), hierarchy);
		}

		if ( generate_cpp ) {
			regDocu::ClassHierarchyProvider chp;
			chp.init( reg );
			// Write C++ files
			regDocu::CppGenerator cppgen( dir, chp, silent );
			cppgen.generate_cpp_files();
		}
	
		if ( generate_list ) {
			regDocu::WriteCompletionList(regDocu::classesAndGroupsAndImplementations, silent, hierarchy);
		}
	
	}
	catch(UGError &err)
	{
		PathProvider::clear_current_path_stack();
		UG_LOG("UGError:\n");
		for(size_t i=0; i<err.num_msg(); i++)
			UG_LOG(err.get_file(i) << ":" << err.get_line(i) << " : " << err.get_msg(i));
		UG_LOG("\n");
	}
	UGFinalize();
	
	return 0;
}

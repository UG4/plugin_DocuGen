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
	int errors = 0;
	PROFILE_FUNC();

	pcl::Init(argcp, argvp);
	GetLogAssistant().set_output_process(parallelOutputProcRank);

//	INIT PATH
	try{

		if(InitPaths((*argvp)[0]) == false)
		{
			UG_ERR_LOG("Path Initialization failed. Expect file access problem.\n")
			UG_ERR_LOG("Check environment variable UG4_ROOT.\n")
			errors |= 1;
		}
	}
	catch(UGError& err)
	{
	//	if ugerror is throw, an internal fatal error occured, we termiate shell
		UG_ERR_LOG("UGError occurred during Path Initialization:\n");
		for(size_t i=0; i<err.num_msg(); i++)
			UG_ERR_LOG(err.get_file(i) << ":" << err.get_line(i) << " : " << err.get_msg(i) << "\n");
		errors |= 2;
	}
	catch(...){
		UG_ERR_LOG("Unknown error received during Path Initialization.\n");
		errors |= 2;
	}

//	INIT STANDARD BRIDGE
	try{
		bridge::InitBridge();
	}
	catch(UGError& err)
	{
		UG_ERR_LOG("UGError occurred during Standard Bridge Initialization:\n");
		for(size_t i=0; i<err.num_msg(); i++)
			UG_ERR_LOG(err.get_file(i) << ":" << err.get_line(i) << " : " << err.get_msg(i) << "\n");
		errors |= 2;
	}
	catch(...){
		UG_ERR_LOG("Unknown error received during Standard Bridge Initialization.\n");
		errors |= 2;
	}

//	INIT PLUGINS
	try{
		if(UGInitPlugins() == false)
		{
			UG_ERR_LOG("Some error at Plugin initialization.\n");
			errors |= 1;
		}

	}
	catch(UGError &err)
	{
	//	if registration of plugin fails, we do abort the shell
	//	this could be skipped if registering of plugin would be done more
	//	carefully. (try/catch in load plugins)
		PathProvider::clear_current_path_stack();
		UG_ERR_LOG("UGError occurred during Plugin Initialization:\n");
		for(size_t i=0; i<err.num_msg(); i++)
			UG_ERR_LOG(err.get_file(i) << ":" << err.get_line(i) << " : " << err.get_msg(i) << "\n");
		errors |= 2;
	}
	catch(...){
		UG_ERR_LOG("Unknown error received during Plugin Initialization.\n");
		errors |= 2;
	}

	return errors;
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
	int errors = regDocu::MyUGInit(&argc, &argv);
	if((errors & 2) == 0)
	{
		if(FindParam("-silent", argc, argv))
			GetLogAssistant().enable_terminal_output(false);
		try
		{
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
			errors &= 4;
			PathProvider::clear_current_path_stack();
			UG_ERR_LOG("UGError in Docu Generation:\n");

			for(size_t i=0; i<err.num_msg(); i++)
				UG_ERR_LOG(err.get_file(i) << ":" << err.get_line(i) << " : " << err.get_msg(i) << "\n");
		}
	}
	GetLogAssistant().enable_terminal_output(true);
	if(errors != 0)
	{
		UG_LOG("\n[[[--- ugdocu/ugshell Initialization Errors ------\n")
		UG_LOG("ugdocu Completition generation noticed some errors during startup of ugshell.\n");

		UG_LOG("Note that these are not compilation errors, but MAY cause problems in the use of ugshell.\n");
		if(errors >= 2)
		{	UG_LOG("Completition could NOT be generated due to fatal ugshell init errors.\n");}
	}

	UGFinalize();

	if(errors != 0)
	{
		UG_LOG("------ ugdocu/ugshell Initialization Errors ---]]]\n\n");
	}
	
	return 0;
}

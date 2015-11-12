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


vector<UGDocuClassDescription> classes;
vector<UGDocuClassDescription> classesAndGroups;
vector<UGDocuClassDescription> classesAndGroupsAndImplementations;

// end group apps_ugdocu
/// \}

}	// namespace regDocu

/// \addtogroup apps_ugdocu
int GenerateScriptReferenceDocu(
		const char* baseDir,
		bool silent,
		bool genHtml,
		bool genCpp,
		bool genList)
{
	int errors = 0;
	if(silent)
		GetLogAssistant().enable_terminal_output(false);
	try
	{

		ug::script::RegisterDefaultLuaBridge(&bridge::GetUGRegistry());

		LOG("****************************************************************\n");
		LOG("* ugdocu - v0.2.0\n");
		LOG("****************************************************************\n");


		string dir=".";

		if ((genHtml || genCpp))
		{
			dir = baseDir;
			if ( baseDir[strlen(baseDir)-1] != '/' ) {
				dir.append( "/" );
			}
		}

		if ( genHtml || genList ) {
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

		if ( genHtml || genList ) {
			GetClassHierarchy( hierarchy, reg );
			UG_LOG("GetClassHierarchy... ");
			UG_LOG(hierarchy.subclasses.size() << " base classes, " << reg.num_class_groups() << " total. " << endl);
		}

		if ( genHtml ) {
			// Write HTML docu
			LOG("Writing html files to \"" << dir << "\"" << endl);
			regDocu::WriteHTMLDocu(regDocu::classes, regDocu::classesAndGroups, dir.c_str(), hierarchy);
		}

		if ( genCpp ) {
			regDocu::ClassHierarchyProvider chp;
			chp.init( reg );
			// Write C++ files
			regDocu::CppGenerator cppgen( dir, chp, silent );
			cppgen.generate_cpp_files();
		}

		if ( genList ) {
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

	GetLogAssistant().enable_terminal_output(true);
	if(errors != 0)
	{
		UG_LOG("\n[[[--- ugdocu/ugshell Initialization Errors ------\n")
		UG_LOG("ugdocu Completition generation noticed some errors during startup of ugshell.\n");

		UG_LOG("Note that these are not compilation errors, but MAY cause problems in the use of ugshell.\n");
		if(errors >= 2)
		{	UG_LOG("Completition could NOT be generated due to fatal ugshell init errors.\n");}
	}

	if(errors != 0)
	{
		UG_LOG("------ ugdocu/ugshell Initialization Errors ---]]]\n\n");
	}
	
	return 0;
}

extern "C" void
InitUGPlugin_ugdocu(Registry* reg, string grp)
{
	grp.append("/ugdocu");
	reg->add_function (	"GenerateScriptReferenceDocu",
						&GenerateScriptReferenceDocu,
						grp,
						"",
						"baseDir # silent # genHtml # genCpp # genList",
						"generates scripting reference documentation.");
}

}	// namespace ug

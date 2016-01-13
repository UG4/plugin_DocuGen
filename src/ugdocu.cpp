/*
 * Copyright (c) 2010-2016:  G-CSC, Goethe University Frankfurt
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
namespace DocuGen
{

/**
 * \defgroup UGDocu UGRegistry Documentation
 * \ingroup plugins
 * \brief Plugin for generating documentation of registered classes and methods
 * \details This plugin generates the documentation of all registered classes and 
 *   methods of the linked libug4.
 *   Depending on command line parameters either HTML or C++ files are generated.
 * 
 *   <b>Usage:</b>
 *   
 *       ugshell -call GenerateScriptReferenceDocu\\(\\"regdocu\\", true, false, true, false\\)
 *   
 *   <b>Parameters:</b>
 *   - string outputPath<br />
 *     path of the output directory for the html, cpp files and completion list
 *   - bool silent<br />
 *     don't print verbose progress info, only warnings
 *   - bool generateHTML<br />
 *     generates legacy HTML docu
 *   - bool generateCPP<br />
 *     generates dummy C++ code from registered functions ready to be parsed
 *     by Doxygen
 *   - bool generateList<br />
 *     generates completion list
 * \{
 */

void WriteCompletionList(std::vector<UGDocuClassDescription> &classesAndGroupsAndImplementations, bool bSilent, ClassHierarchy &hierarchy);


vector<UGDocuClassDescription> classes;
vector<UGDocuClassDescription> classesAndGroups;
vector<UGDocuClassDescription> classesAndGroupsAndImplementations;

// end group apps_ugdocu
/// \}

}	// namespace DocuGen

/// \addtogroup DocuGen
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
			DocuGen::GetGroups(DocuGen::classes, DocuGen::classesAndGroups, DocuGen::classesAndGroupsAndImplementations);
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
			DocuGen::WriteHTMLDocu(DocuGen::classes, DocuGen::classesAndGroups, dir.c_str(), hierarchy);
		}

		if ( genCpp ) {
			DocuGen::ClassHierarchyProvider chp;
			chp.init( reg );
			// Write C++ files
			DocuGen::CppGenerator cppgen( dir, chp, silent );
			cppgen.generate_cpp_files();
		}

		if ( genList ) {
			DocuGen::WriteCompletionList(DocuGen::classesAndGroupsAndImplementations, silent, hierarchy);
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
InitUGPlugin_DocuGen(Registry* reg, string grp)
{
	grp.append("/DocuGen");
	reg->add_function (	"GenerateScriptReferenceDocu",
						&GenerateScriptReferenceDocu,
						grp,
						"",
						"baseDir # silent # genHtml # genCpp # genList",
						"generates scripting reference documentation.");
}

}	// namespace ug

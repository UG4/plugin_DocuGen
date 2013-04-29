/*
 *
 */

#ifndef __UG__REGDOCU__CPP_GENERATOR_H
#define __UG__REGDOCU__CPP_GENERATOR_H

#include <memory> // std::shared_ptr
#include <vector> // std::vector
#include <map>    // std::map
#include <string> // std::string

#ifdef UG_BRIDGE
# include "bridge/bridge.h"
#endif
#include "registry/registry.h"
#include "registry/class_helper.h"

namespace ug
{
namespace regDocu
{

using namespace std;

class CppGenerator
{
	public:
		CppGenerator( const string dir, bool silent=false );
		~CppGenerator();
		
		void generate_cpp_files();
		
	private:
		void generate_class_group();
		void generate_global_functions();
		
		void generate_class_docu();
		void generate_class_cpp();
		
		void generate_class_constructors();
		void generate_class_public_methods();
		void generate_class_public_members();
		
		template< class TEntity >
		void write_brief_detail_docu( const TEntity &entity );
		template< class TFunction >
		void write_parameter_docu( const TFunction &function );
		template< class TFunction >
		void write_generic_function( const TFunction &function );
		
		template< class TFunction >
		void generate_parameter_list( const TFunction &param_in );
		string generate_return_value( const bridge::ExportedFunctionBase &method );
		
		static string name_to_id( const string& str );
		string sanitize_parameter_name( const string &param ) const;
		string sanitize_docu( const string &param_docu ) const;
		vector<string> split_group_hieararchy( const string group );
		
	private:
		string m_output_dir;
		bridge::Registry &mr_reg;
		
		bridge::IExportedClass *m_curr_class;
		bridge::ClassGroupDesc *m_curr_group;
		map<string, string> m_written_classes;
		
		fstream m_curr_file;
		
		bool m_silent;
};

}	// end namespace regDocu
}	// end namespace ug

#endif	// __UG__REGDOCU__CPP_GENERATOR_H

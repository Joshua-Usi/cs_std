#include "xml.hpp"
#include "../console.hpp"
#include "cs_std_to_rapid.hpp"
#include "rapid_to_cs_std.hpp"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"

namespace cs_std::xml
{
	document::document(const std::string& xml)
	{
		rapidxml::xml_document<char> doc{};
		try
		{
			doc.parse<rapidxml::parse_declaration_node | rapidxml::parse_no_data_nodes>((char*)xml.c_str());
		}
		catch (const rapidxml::parse_error& e)
		{
			console::log("Parse Error: ", e.what());
		}
		internal::rapid_to_cs_std(*this, &doc);
	}
	std::string document::stringify() const
	{
		std::string outputString{};

		rapidxml::xml_document<char> doc;
		internal::cs_std_to_rapid(*this, &doc);
		rapidxml::print(&outputString, doc, 0);

		return outputString;
	}
}
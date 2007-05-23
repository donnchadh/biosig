#include <string>

template <typename TContainer>

void stringtokenizer(TContainer& container, const std::string& in, const char* const delimiters = " \t\n"){
	
	const std::string::size_type len = in.length();
	std::string::size_type i = 0;

	while (i < len){
		i = in.find_first_not_of(delimiters, i);

		if (i == std::string::npos)
			return;
		std::string::size_type j = in.find_first_of(delimiters, i);
		if (j == std::string::npos){
			container.push_back(in.substr(i));
			return;
		}
		else
			container.push_back(in.substr(i, j-i));
		i = j + 1;
	}
}

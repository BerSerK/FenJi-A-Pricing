#ifndef __CSVROW_H__
#define __CSVROW_H__

#include <iterator>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

class CSVRow
{
public:
  std::string const& operator[](std::size_t index) const
  {
    return m_data[index];
  }
  std::size_t size() const
  {
    return m_data.size();
  }
  std::string trim(const std::string& str,
                 const std::string& whitespace = " \t\n\"\'\r")
  {
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
      return ""; // no content
    
    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;
    
    return str.substr(strBegin, strRange);
  }
  
  void readNextRow(std::istream& str)
  {
    std::string         line;
    std::getline(str,line);

    std::stringstream   lineStream(line);
    std::string         cell;

    m_data.clear();
    while(std::getline(lineStream,cell,','))
      {
	cell = trim(cell);
	m_data.push_back(cell);
      }
  }
  
  void print() {
    for ( size_t i = 0; i < m_data.size(); i ++ ) {
      std::cout << std::setw(16) << m_data[ i ];
    }
    std::cout << std::endl;
  }
private:
  std::vector<std::string>    m_data;
};

#endif // __CSVROW_H__

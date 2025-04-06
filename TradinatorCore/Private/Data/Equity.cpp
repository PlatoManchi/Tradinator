#include "Data/Equity.h"

#include <format>
#include <fstream>
#include <sstream>

void Equity::FromString(std::string str)
{
	std::vector<std::string> split_strings;
	int prev_index = 0;
	for (int i = 0; i <= str.size(); ++i)
	{
		if (i == str.size())
		{
			split_strings.push_back(str.substr(prev_index, str.size() - prev_index));
		}
		else if (str[i] == ',')
		{
			split_strings.push_back(str.substr(prev_index, i - prev_index));
			prev_index = i + 1;
		}
	}

	m_symbol = split_strings[0];
	m_name = split_strings[1];
	m_series = split_strings[2];

	std::tm tm = {};
	std::stringstream ss(split_strings[3]);
	ss >> std::get_time(&tm, "%d-%b-%y");
	m_date_of_listing = std::chrono::system_clock::from_time_t(std::mktime(&tm));

	m_paid_up_value = std::stoi(split_strings[4]);
	m_market_lot = std::stoi(split_strings[5]);
	m_isin_number = split_strings[6];
	m_face_value = std::stoi(split_strings[7]);
}

#define READ_STRING_FROM_STREAM(stream, string_name, string_size, buffer)			\
stream.read(reinterpret_cast<char*>(&string_size), sizeof(string_size));			\
buffer = new char[string_size + 1];													\
stream.read(reinterpret_cast<char*>(buffer), string_size);							\
buffer[string_size] = '/0';															\
string_name = buffer;																\
delete[] buffer;																	\
buffer = nullptr;																	\

void Equity::ReadFromStream(std::istream& stream)									
{
	std::size_t string_size;
	char* buffer = nullptr;
	
	READ_STRING_FROM_STREAM(stream, m_symbol, string_size, buffer);
	READ_STRING_FROM_STREAM(stream, m_name, string_size, buffer);
	READ_STRING_FROM_STREAM(stream, m_series, string_size, buffer);

	std::chrono::system_clock::rep count;
	stream.read(reinterpret_cast<char*>(&count), sizeof(count));
	std::chrono::system_clock::duration duration_since_epoch(count);
	std::chrono::system_clock::time_point date_of_listing(duration_since_epoch);
	m_date_of_listing = date_of_listing;

	stream.read(reinterpret_cast<char*>(&m_paid_up_value), sizeof(m_paid_up_value));
	stream.read(reinterpret_cast<char*>(&m_market_lot), sizeof(m_market_lot));
	READ_STRING_FROM_STREAM(stream, m_isin_number, string_size, buffer);
	stream.read(reinterpret_cast<char*>(&m_face_value), sizeof(m_face_value));
}

#define WRITE_STRING_TO_STREAM(stream, string_name, string_size)						\
string_size = string_name.size();														\
/* write the size of string first */													\
stream.write(reinterpret_cast<const char*>(&string_size), sizeof(string_size));			\
/* write the string contents */															\
stream.write(reinterpret_cast<const char*>(string_name.c_str()), string_size);			\

void Equity::WriteToFile(std::ofstream& stream)
{
	std::size_t string_size;
	// Writting in binary format
	WRITE_STRING_TO_STREAM(stream, m_symbol, string_size);
	WRITE_STRING_TO_STREAM(stream, m_name, string_size);
	WRITE_STRING_TO_STREAM(stream, m_series, string_size);

	std::chrono::system_clock::rep count = m_date_of_listing.time_since_epoch().count();
	stream.write(reinterpret_cast<const char*>(&count), sizeof(count));

	stream.write(reinterpret_cast<const char*>(&m_paid_up_value), sizeof(m_paid_up_value));
	stream.write(reinterpret_cast<const char*>(&m_market_lot), sizeof(m_market_lot));
	WRITE_STRING_TO_STREAM(stream, m_isin_number, string_size);
	stream.write(reinterpret_cast<const char*>(&m_face_value), sizeof(m_face_value));
}

std::string Equity::ToString() const
{
	return std::format("{},{},{},{},{},{},{},{}"
		, m_symbol
		, m_name
		, m_series
		, std::format("{:%d-%b-%y}", m_date_of_listing)
		, m_paid_up_value
		, m_market_lot
		, m_isin_number
		, m_face_value);
}


std::ofstream& operator << (std::ofstream& stream, Equity& equity)
{
	equity.WriteToFile(stream);

	return stream;
}

std::ostream& operator << (std::ostream& stream, Equity& equity)
{
	stream << equity.ToString();

	return stream;
}

std::istream& operator >> (std::istream& stream, Equity& equity)
{
	equity.ReadFromStream(stream);
	return stream;
}
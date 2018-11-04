#pragma once

#include <ctime>
#include <string>

#pragma warning( push )
#pragma warning( disable : 4996)

std::string get_time_string(int timestamp) {
	const time_t rawtime = timestamp;
	struct tm * dt;
	char timestr[30];
	char buffer[30];


	dt = localtime(&rawtime);
	// use any strftime format spec here
	strftime(timestr, sizeof(timestr), "%d.%m.%Y %H:%M:%S", dt);
	sprintf_s(buffer, "%s", timestr);

	return buffer;
}

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
/*
std::string split implementation by using delimiter as a character.
*/

std::vector<std::string> split(std::string strToSplit, char delimeter)
{
	

	std::stringstream ss(strToSplit);
	std::string item;
	std::vector<std::string> splittedStrings;
	while (std::getline(ss, item, delimeter))
	{
		splittedStrings.push_back(item);
	}
	return splittedStrings;
}

//char checkmark(char* variable) {
//	char ad;
//	if (variable == "1") {
//		ad = '✓';
//	}
//	else {
//		ad = '✘';
//	}
//
//	return ad;
//}

//void erase(std::vector<std::string> &vectorname, int n, int from, int to)
//{
//	std::string new_vector_entry = vectorname[n].erase(from, to);
//	vectorname[n] = new_vector_entry;
//}

#pragma warning( pop )
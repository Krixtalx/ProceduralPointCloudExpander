#pragma once
#include <unordered_map>

constexpr std::unordered_map<std::string, unsigned> lasClassification{
	{"Never Classified", 0},
	{"Unclassified", 1},
	{"Ground", 2},
	{"Low vegetation", 3},
	{"Medium vegetation", 4},
	{"High vegetation", 5},
	{"Building", 6},
	{"Low point noise", 7},
	{"Reserved", 8},
	{"Water", 9},
	{"Rail", 10},
	{"Road surface", 11},
	{"Reserved", 12}
};
/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "DetectIndentationStyle.h"

#include "doc/CEditDoc.h"
#include "config/app_constants.h"
#include <fstream>
#include <regex>

namespace {

namespace fs = std::filesystem;

struct EditorConfig {
	bool root = false;
	enum struct IndentStyle {
		Tab,
		Space,
	};
	//enum struct EOL {
	//	lf,
	//	cr,
	//	crlf,
	//};
	struct Section {
		std::string name;
		std::optional<IndentStyle> indent_style;
		std::optional<int> indent_size; // -1 : use tab_width if specified
		std::optional<int> tab_width;
		//std::optional<bool> trim_trailing_whitespace;
		//std::optional<bool> insert_final_newline;
	};
	std::vector<Section> sections;
};

const std::regex rePropLine{ R"(^\s*(\w+)\s*=\s*(\w+)\s*$)" };

constexpr std::string_view trim(std::string_view s) noexcept {
	constexpr std::string_view trim_chars = " \t";
	const auto first = s.find_first_not_of(trim_chars);
	if (first == std::string_view::npos) return {};
	const auto last = s.find_last_not_of(trim_chars);
	return s.substr(first, (last - first + 1));
}

inline bool strieq(std::string_view lhs, std::string_view rhs) {
	return 0 == _strnicmp(lhs.data(), rhs.data(), std::min(lhs.size(), rhs.size()));
}

inline void tolower(std::string& s) {
	for (char& c : s) c = (char)std::tolower(c);
}

struct EditorConfigParser {
	bool Parse(const fs::path& configPath, EditorConfig& config) {
		std::ifstream file(configPath, std::ios::in);
		if (!file) {
			return false;
		}
		uint8_t buff[3];
		constexpr uint8_t bom[3] = {0xEF, 0xBB, 0xBF};
		file.read((char*)buff, 3);
		if (file.gcount() < 3) {
			return false;
		}
		if (0 != memcmp(buff, bom, 3)) {
			file.seekg(0);
		}
		std::string rawLine;
		std::match_results<std::string_view::const_iterator> matches;
		EditorConfig::Section* section = nullptr;
		while (std::getline(file, rawLine)) {
			std::string_view line = trim(rawLine);
			if (line.starts_with('[') && line.ends_with(']')) {
				config.sections.resize(config.sections.size() + 1);
				section = &config.sections.back();
				section->name = line.substr(1, line.size() - 2);
			}
			else if (line.starts_with('#') || line.starts_with(';')) {
				// comment
			}
			else if (std::regex_match(line.cbegin(), line.cend(), matches, rePropLine) && matches.size() == 3) {
				std::string key = matches[1].str();
				tolower(key);
				const std::string_view value(&(*matches[2].first), matches[2].length());
				if (section) {
					if (key == "indent_style") {
						if (strieq(value, "tab")) {
							section->indent_style = EditorConfig::IndentStyle::Tab;
						}
						else if (strieq(value, "space")) {
							section->indent_style = EditorConfig::IndentStyle::Space;
						}
					}
					else if (key == "indent_size") {
						if (strieq(value, "tab")) {
							section->indent_size = -1;
						}
						else {
							int indent_size;
							if (std::errc() != std::from_chars(value.data(), value.data() + value.size(), indent_size).ec) {
								return false;
							}
							section->indent_size = indent_size;
						}
					}
					else if (key == "tab_width") {
						int tab_width;
						if (std::errc() != std::from_chars(key.data(), key.data() + key.size(), tab_width).ec) {
							return false;
						}
						section->tab_width = tab_width;
					}
				}
				else {
					if (key == "root") {
						config.root = strieq(value, "true");
					}
				}
			}
		}
		return true;
	}
};

bool glob_matches_extension(std::string_view glob, std::string_view extension)
{
	std::string g = std::string(glob);
	std::string e = std::string(extension);
	tolower(g);
	tolower(e);
	// *.ext
	if (g.starts_with("*" + e)) {
		return true;
	}
	// *.{h,cpp,ts}
	if (g.starts_with("*.{") && g.ends_with("}")) {
		const auto end = g.size() - 1;
		size_t pos = 3;
		while (pos < end) {
			size_t comma = g.find(',', pos);
			if (comma == std::string::npos || comma > end) {
				comma = end;
			}
			std::string_view candidate(g.data() + pos, comma - pos);
			if (e.substr(1) == candidate) {
				return true;
			}
			if (comma == end) break;
			pos = comma + 1;
		}
	}
	return false;
}

bool FindEditorConfig(const CEditDoc* pcDoc, IndentationStyle& style)
{
	fs::path path = pcDoc->m_cDocFile.GetFilePath();
	if (!path.has_extension()) {
		return false;
	}
	const auto extension = path.extension().string();
	EditorConfigParser parser;
	const auto rootPath = path.root_path();
	while (path.has_parent_path()) {
		path = path.parent_path();
		auto configPath = path / ".editorconfig";
		if (fs::exists(configPath)) {
			EditorConfig config;
			if (parser.Parse(configPath.wstring(), config)) {
				for (auto& section : config.sections) {
					if (glob_matches_extension(section.name, extension)) {
						if (section.indent_style) {
							if (section.indent_style == EditorConfig::IndentStyle::Tab) {
								style.character = IndentationStyle::Character::Tabs;
								if (section.tab_width.has_value()) {
									style.tabSpace = section.tab_width.value();
									return true;
								}
								else if (section.indent_size.has_value()) {
									style.tabSpace = section.indent_size.value();
									return true;
								}
							}
							else if (section.indent_style == EditorConfig::IndentStyle::Space) {
								style.character = IndentationStyle::Character::Spaces;
								if (section.indent_size.has_value()) {
									style.tabSpace = *section.indent_size;
									if (style.tabSpace == -1 && section.tab_width.has_value()) {
										style.tabSpace = section.tab_width.value();
									}
									return true;
								}
							}
						}
						return false;
					}
				}
				if (config.root) {
					return false;
				}
			}
		}
		if (path == rootPath) {
			break;
		}
	}
	return false;
}

} // namespace

void DetectIndentationStyle(const CEditDoc* pcDoc, size_t nMaxLinesToCheck, IndentationStyle& style)
{
	if (FindEditorConfig(pcDoc, style)) {
		return;
	}
	const auto& cDocLineMgr = pcDoc->m_cDocLineMgr;
	int nSpaceUsed = 0;
	int nTabUsed = 0;
	style.character = IndentationStyle::Character::Unknown;
	// 各行の行頭の文字が半角空白かタブ文字かをカウントする
	for (size_t i=0; i<nMaxLinesToCheck; ++i) {
		const CDocLine* pLine = cDocLineMgr.GetLine(CLogicInt((int)i));
		if (pLine == nullptr) {
			break;
		}
		auto len = pLine->GetLengthWithoutEOL();
		// check leading letters
		if (len == 0) {
			continue;
		}
		const wchar_t* str = pLine->GetPtr();
		wchar_t c = str[0];
		if (c == '\t') ++nTabUsed;
		else if (c == ' ') ++nSpaceUsed;
	}
	// 4倍以上行数に差がある場合は明確な差があると判断して、インデントに使われている文字種別を決定する
	if (nSpaceUsed > nTabUsed * 4) style.character = IndentationStyle::Character::Spaces;
	else if (nTabUsed > nSpaceUsed * 4) style.character = IndentationStyle::Character::Tabs;

	// 半角空白でインデントが行われていると判断した場合、前の行とのインデント差の頻度を調べて最頻値のインデント差をタブ幅とする
	if (style.character == IndentationStyle::Character::Spaces) {
		// https://heathermoor.medium.com/detecting-code-indentation-eff3ed0fb56b
		std::array<int, TABSPACE_MAX+1> indents{}; // # spaces indent -> # times seen
		int last = 0; // # leading spaces in the last line we saw
		for (size_t i=0; i<nMaxLinesToCheck; ++i) {
			const CDocLine* pLine = cDocLineMgr.GetLine(CLogicInt((int)i));
			if (pLine == nullptr) {
				break;
			}
			auto len = pLine->GetLengthWithoutEOL();
			// check leading letters
			if (len == 0) {
				continue;
			}
			const wchar_t* str = pLine->GetPtr();
			int width = 0;
			while (*str++ == ' ')
				++width;
			int indent = abs(width - last);
			if (indent > 1 && indent < int(indents.size())) {
				++indents[indent];
			}
			last = width;
		}
		// find most frequent non-zero width difference
		int max = 0;
		int maxIdx = -1;
		for (size_t i=1; i<indents.size(); ++i) {
			auto freq = indents[i];
			if (freq > max) {
				max = freq;
				maxIdx = (int)i;
			}
		}
		if (maxIdx != -1) {
			style.tabSpace = maxIdx;
		}
	}
}

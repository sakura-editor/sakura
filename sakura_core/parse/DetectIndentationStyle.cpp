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
		std::optional<int> indent_size;
		std::optional<int> tab_width;
		//std::optional<bool> trim_trailing_whitespace;
		//std::optional<bool> insert_final_newline;
	};
	std::vector<Section> sections;
};

const std::regex rePropLine{ R"(^\s*(\w+)\s*=\s*(\w+)\s*$)" };

struct EditorConfigParser {
	bool Parse(const fs::path& configPath, EditorConfig& config) {
		std::ifstream file(configPath, std::ios::binary);
		if (!file) {
			return false;
		}
		uint8_t buff[3];
		constexpr uint8_t bom[3] = {0xEF, 0xBB, 0xBF};
		file.read((char*)buff, 3);
		if (0 != memcmp(buff, bom, 3)) {
			file.seekg(0);
		}
		bool ret = true;
		std::string line;
		std::smatch matches;
		EditorConfig::Section* section = nullptr;
		while (std::getline(file, line)) {
			if (line.starts_with('[') && line.ends_with(']')) {
				config.sections.resize(config.sections.size() + 1);
				section = &config.sections.back();
				section->name = line.substr(1, line.size() - 2);
			}
			else if (line.starts_with('#') || line.starts_with(';')) {
				// comment
			}
			else if (std::regex_match(line, matches, rePropLine) && matches.size() == 3) {
				const auto& key = matches[1];
				const auto& value = matches[2];
				if (section) {
					if (key == "indent_style") {
						if (value == "tab") {
							section->indent_style = EditorConfig::IndentStyle::Tab;
						}
						else if (value == "space") {
							section->indent_style = EditorConfig::IndentStyle::Space;
						}
					}
					else if (key == "indent_size") {
						section->indent_size = atoi(value.str().c_str());
					}
					else if (key == "tab_width") {
						section->tab_width = atoi(value.str().c_str());
					}
				}
				else {
					if (key == "root") {
						config.root = value == "true";
					}
				}
			}
		}
		return ret;
	}
};

bool glob_matches_extension(std::string_view glob, std::string_view extension)
{
	std::string g = std::string(glob);
	std::string e = std::string(extension);
	for (char& c : g) c = std::tolower(c);
	for (char& c : e) c = std::tolower(c);
	// *.ext
	if (g.starts_with("*.") && g.substr(2) == e) {
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
								if (section.tab_width) {
									style.tabSpace = *section.tab_width;
									return true;
								}
								else if (section.indent_size) {
									style.tabSpace = *section.indent_size;
									return true;
								}
							}
							else if (section.indent_style == EditorConfig::IndentStyle::Space) {
								style.character = IndentationStyle::Character::Spaces;
								if (section.indent_size) {
									style.tabSpace = *section.indent_size;
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
			if (indent > 1 && indent < indents.size()) {
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

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>

class Table {
public:
	enum class HeaderOrientation {
		COLUMN, ROW, NONE
	};

private:
	struct {
		const unsigned char horizontal = 196;
		const unsigned char vertical = 179;
		struct {
			const unsigned char left, middle, right;
		} top = {218,194,191}, middle = {195,197,180}, botton = {192,193, 217};
	} border;

	struct Cell {
		std::string text;

		explicit Cell() : text() {}
		~Cell() { text.clear(); }

		Cell(const Cell& other) = delete;
		Cell(const Cell&& other) = delete;

		Cell& operator=(const Cell& other) = delete;
		Cell& operator=(const Cell&& other) = delete;

		const Cell& operator=(const std::string& value) {
			text = value;
			return *this;
		}

		const Cell& operator=(const char* value) {
			text = value;
			return *this;
		}

		const Cell& operator=(const char& value) {
			text = value;
			return *this;
		}

		template<typename T>
		const Cell& operator=(const T& value) requires std::integral<T> || std::floating_point<T> {
			text = std::to_string(value);
			return *this;
		}

		const Cell& operator=(const bool& value) {
			text = value ? "true" : "false";
			return *this;
		}
	};

	HeaderOrientation headerOrientation;
	std::string title;
	size_t columnsForPage;
	std::vector<std::unique_ptr<std::pair<size_t, bool>>> columnsWidth;
	std::vector<std::string> header;
	std::vector<std::unique_ptr<std::vector<std::unique_ptr<Cell>>>> data;

public:
	Table() = delete;
	explicit Table(const std::string& title, const size_t rowCount, const size_t columnCount, HeaderOrientation headerOrientation = HeaderOrientation::NONE)
		: headerOrientation(headerOrientation), title(title), columnsForPage(4) {
		setRowCount(rowCount);
		setColumnCount(columnCount);
	}
	explicit Table(const std::string& title, const HeaderOrientation headerOrientation = HeaderOrientation::NONE) : Table("", 0, 0, headerOrientation) {}
	explicit Table(const size_t rowCount, const size_t columnCount) : Table("", rowCount, columnCount) {}

	void addHeader(const std::initializer_list<std::string>& header) {
		if (headerOrientation == HeaderOrientation::NONE)
			return;

		size_t i = 0;
		if (headerOrientation == HeaderOrientation::COLUMN) {
			i = getColumnCount();
			setColumnCount(i + header.size());
		} else if (headerOrientation == HeaderOrientation::ROW) {
			i = getRowCount();
			setRowCount(i + header.size());
		}

		const size_t oldCount = i;
		i = 0;
		for (auto& value : header) {
			this->header[oldCount + i] = value;
			if (headerOrientation == HeaderOrientation::COLUMN)
				updateColumnWidth(i + 1, value.size(), true);
			else if (headerOrientation == HeaderOrientation::ROW)
				updateColumnWidth(0, value.size(), true);
			++i;
		}
	}

	void addHeader(const std::string& value) {
		if (headerOrientation == HeaderOrientation::NONE)
			return;

		if (headerOrientation == HeaderOrientation::COLUMN)
			setColumnCount(getColumnCount() + 1);
		else if (headerOrientation == HeaderOrientation::ROW)
			setRowCount(getRowCount() + 1);

		header.back() = value;
		if (headerOrientation == HeaderOrientation::COLUMN)
			updateColumnWidth(getColumnCount(), value.size(), true);
		else if (headerOrientation == HeaderOrientation::ROW)
			updateColumnWidth(0, value.size(), true);
	}

	void updateHeaderAt(const size_t index, const std::string& value) {
		if (headerOrientation == HeaderOrientation::COLUMN) {
			if (index < getColumnCount())
				return;
		} else if (headerOrientation == HeaderOrientation::ROW) {
			if (index < getRowCount())
				return;
		} else if (headerOrientation == HeaderOrientation::NONE)
			return;

		header[index] = value;
		if (headerOrientation == HeaderOrientation::COLUMN)
			updateColumnWidth(index + 1, value.size());
		else if (headerOrientation == HeaderOrientation::ROW)
			updateColumnWidth(0, value.size());
	}

	template<typename T>
	void addRow(const std::initializer_list<T>& row) requires std::integral<T> || std::floating_point<T> || std::same_as<T, const char*> {
		setRowCount(getRowCount() + 1);
		const size_t columnCount = std::min(getColumnCount(), row.size());
		for (size_t i = 0; i < columnCount; ++i) {
			*data.back()->at(i) = *(row.begin() + i);
			updateColumnWidth(i + 1, data.back()->at(i)->text.size());
		}
	}

	template<typename T>
	void addColumn(const std::initializer_list<T>& column) requires std::integral<T> || std::floating_point<T> || std::same_as<T, const char*> {
		setColumnCount(getColumnCount() + 1);
		const size_t rowCount = std::min(getRowCount(), column.size());
		for (size_t i = 0; i < rowCount; ++i) {
			*data.at(i)->back() = *(column.begin() + i);
			updateColumnWidth(getColumnCount() + 1, data.at(i)->at(i)->text.size());
		}
	}

	template<typename T>
	void updateValueAt(const size_t row, const size_t column, const T value) requires std::integral<T> || std::floating_point<T> || std::same_as<T, const char*> {
		if (row < getRowCount() && column < getColumnCount()) {
			*data.at(row)->at(column) = value;
			updateColumnWidth(column + 1, data.at(row)->at(column)->text.size());
		}
	}

	const size_t getColumnCount() const {
		if (columnsWidth.empty())
			return 0;
		return columnsWidth.size() - 1;
	}

	void setColumnCount(const size_t columns) {
		while (columnsWidth.size() < columns + 1)
			columnsWidth.emplace_back(new std::pair<size_t, bool>(0, false));
		while (columnsWidth.size() > columns + 1)
			columnsWidth.pop_back();

		if (headerOrientation == HeaderOrientation::COLUMN) {
			while (header.size() < columns) {
				header.emplace_back();
				header.back() = "Column " + std::to_string(header.size());
				updateColumnWidth(header.size(), header.back().size());
			}
			while (header.size() > columns)
				header.pop_back();
		}

		for (auto& row : data) {
			while (row->size() < columns)
				row->emplace_back(new Cell());
			while (row->size() > columns)
				row->pop_back();
		}
	}

	const size_t getRowCount() const {
		return data.size();
	}

	void setRowCount(const size_t rows) {
		if (headerOrientation == HeaderOrientation::ROW) {
			while (header.size() < rows) {
				header.emplace_back();
				header.back() = "Row " + std::to_string(header.size());
				updateColumnWidth(0, header.back().size());
			}
			while (header.size() > rows)
				header.pop_back();
		}

		while (data.size() < rows) {
			data.emplace_back(new std::vector<std::unique_ptr<Cell>>());
			data.back()->reserve(getColumnCount());
			while (data.back()->size() < getColumnCount())
				data.back()->emplace_back(new Cell());
		}
		while (data.size() > rows) {
			data.back()->clear();
			data.pop_back();
		}
	}

	void setColumnWidth(size_t index, const size_t width) {
		if (index < getColumnCount()) {
			columnsWidth[++index]->first = width;
			columnsWidth[index]->second = true;
		}
	}

	void setColumnAutoWidth(size_t index) {
		if (index < getColumnCount()) {
			columnsWidth[++index]->second = false;
			columnsWidth[index]->first = headerOrientation == HeaderOrientation::COLUMN ? header[index].size() : 0;
			for (auto& row : data)
				columnsWidth[index]->first = std::max(columnsWidth[index]->first, row->at(index)->text.size());
		}
	}

	void print() {
		size_t tableWidth = getColumnCount() - 1;
		for (const auto& pair : columnsWidth)
			tableWidth += pair->first;

		std::cout << border.top.left;

		if (!title.empty()) {
			tableWidth = std::max(title.size(), tableWidth);
			repeat(border.horizontal, tableWidth);
			std::cout << border.top.right << std::endl;

			std::cout << border.vertical;
			std::cout << title;
			repeat(' ', tableWidth - title.size());
			std::cout << border.vertical << std::endl;

			std::cout << border.middle.left;
		}

		if (headerOrientation == HeaderOrientation::COLUMN) {
			for (size_t i = 0; i < getColumnCount(); ++i) {
				repeat(border.horizontal, columnsWidth[i + 1]->first);
				if (i < getColumnCount() - 1)
					std::cout << border.top.middle;
			}

			if (title.empty())
				std::cout << border.top.right << std::endl;
			else
				std::cout << border.middle.right << std::endl;

			std::cout << border.vertical;
			for (size_t i = 0; i < header.size(); ++i) {
				if (header.size() < columnsWidth[i + 1]->first) {
					std::cout << header[i];
					repeat(32, columnsWidth[i + 1]->first - header[i].size());
				} else
					std::cout << header[i].substr(0, columnsWidth[i + 1]->first);
				std::cout << border.vertical;
			}
			std::cout << std::endl;
		}

		for (size_t r = 0; r < getRowCount(); ++r) {
			std::cout << border.middle.left;
			for (size_t i = 0; i < getColumnCount(); ++i) {
				repeat(border.horizontal, columnsWidth[i + 1]->first);
				if (i < getColumnCount() - 1)
					std::cout << border.middle.middle;
			}
			std::cout << border.middle.right << std::endl;

			std::cout << border.vertical;
			for (size_t c = 0; c < getColumnCount(); ++c) {
				if (data[r]->at(c)->text.empty())
					repeat(32, columnsWidth[c + 1]->first);
				else if (data[r]->at(c)->text.size() < columnsWidth[c + 1]->first) {
					std::cout << data[r]->at(c)->text;
					repeat(32, columnsWidth[c + 1]->first - data[r]->at(c)->text.size());
				} else
					std::cout << data[r]->at(c)->text.substr(0, columnsWidth[c + 1]->first);
				std::cout << border.vertical;
			}
			std::cout << std::endl;
		}

		std::cout << border.botton.left;
		for (size_t i = 0; i < getColumnCount(); ++i) {
			repeat(border.horizontal, columnsWidth[i + 1]->first);
			if (i < getColumnCount() - 1)
				std::cout << border.botton.middle;
		}
		std::cout << border.botton.right;
	}

	const HeaderOrientation getHeaderOrientation() const { return headerOrientation; }
	void setHeaderOrientation(const HeaderOrientation headerOrientation) { this->headerOrientation = headerOrientation; }

	const std::string getTitle() const { return title; }
	void setTitle(const std::string& title) { this->title = title; }

	const size_t getColumnsForPage() const { return columnsForPage; }
	void setColumnsForPage(size_t columns) {
		if (columns < 3)
			columns = 3;
		columnsForPage = columns;
	}

private:
	void updateColumnWidth(const size_t index, const size_t width, const bool force = false) {
		if (index <= columnsWidth.size()) {
			if (force == true)
				columnsWidth[index]->first = width;
			else if (columnsWidth[index]->second == false)
				columnsWidth[index]->first = std::max(columnsWidth[index]->first, width);
		}
	}

	void repeat(const unsigned char caracter, const size_t times) const {
		for (size_t i = 0; i < times; ++i)
			std::cout << caracter;
	}
};
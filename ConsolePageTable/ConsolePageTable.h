#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>

class Table {
public:
	enum class HeaderOrientation { COLUMN, ROW, NONE };

private:
	struct Line {
		const unsigned char left, middle, right;
	};
	struct {
		const unsigned char horizontal = 196;
		const unsigned char vertical = 179;
		Line top = {218,194,191}, middle = {195,197,180}, botton = {192,193, 217};
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
	unsigned short columnsForPage;
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

		const size_t oldSize = i;
		i = 0;
		for (auto& value : header) {
			this->header[oldSize + i] = value;
			++i;
			if (headerOrientation == HeaderOrientation::COLUMN)
				columnsWidth[oldSize + i]->first = value.size();
			else if (headerOrientation == HeaderOrientation::ROW)
				columnsWidth.front()->first = value.size();
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
			columnsWidth[getColumnCount()]->first = value.size();
		else if (headerOrientation == HeaderOrientation::ROW)
			columnsWidth.front()->first = value.size();
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
			updateColumnWidth(getColumnCount(), data.at(i)->back()->text.size());
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
		const unsigned short lenght = static_cast<unsigned short>(getColumnCount()) / columnsForPage;
		for (unsigned short i = 0; i <= lenght; ++i)
			printPage(i);
	}

	const HeaderOrientation getHeaderOrientation() const { return headerOrientation; }
	void setHeaderOrientation(const HeaderOrientation headerOrientation) {
		this->headerOrientation = headerOrientation;
		if (headerOrientation == HeaderOrientation::COLUMN)
			setColumnCount(getColumnCount());
		else if (headerOrientation == HeaderOrientation::ROW)
			setRowCount(getRowCount());
	}

	const std::string getTitle() const { return title; }
	void setTitle(const std::string& title) { this->title = title; }

	const unsigned short getColumnsForPage() const { return columnsForPage; }
	void setColumnsForPage(unsigned short columns) {
		if (columns < 3)
			columns = 3;
		columnsForPage = columns;
	}

private:
	inline void updateColumnWidth(const size_t index, const size_t width) {
		if (index <= columnsWidth.size())
			if (columnsWidth[index]->second == false)
				columnsWidth[index]->first = std::max(columnsWidth[index]->first, width);
	}

	inline void repeat(const unsigned char caracter, const size_t times) const {
		for (size_t i = 0; i < times; ++i)
			std::cout << caracter;
	}

	inline void printLine(const unsigned short begin, const unsigned short end, const Line line) const {
		std::cout << line.left;
		for (unsigned short i = begin; i < end; ++i) {
			repeat(border.horizontal, columnsWidth[i + 1]->first);
			if (i < end - 1)
				std::cout << line.middle;
		}
		std::cout << line.right << std::endl;
	}

	inline void printCell(const std::string& text, const size_t width) const {
		std::cout << border.vertical;
		if (text.empty())
			repeat(32, width);
		else if (text.size() < width) {
			std::cout << text;
			repeat(32, width - text.size());
		} else
			std::cout << text.substr(0, width);
	}

	void printPage(const unsigned short page) {
		const unsigned short columnBegin = page * columnsForPage;
		const unsigned short columnEnd = std::min(columnBegin + columnsForPage, static_cast<int>(getColumnCount()));
		const unsigned short columnCount = columnEnd - columnBegin;

		size_t tableWidth = columnCount + columnsWidth.front()->first;
		if (tableWidth > 0)
			--tableWidth;
		for (size_t i = columnBegin; i < columnEnd; ++i)
			tableWidth += columnsWidth[i + 1]->first;

		if (!title.empty() && page == 0) {
			if (title.size() > tableWidth) {
				unsigned short value = static_cast<unsigned short>(title.size());
				if (columnCount > 0)
					value = value / columnCount;
				for (size_t i = columnBegin; i < columnEnd; ++i)
					columnsWidth[i + 1]->first = value;
				tableWidth = title.size() + columnCount;
				if (columnCount > 0)
					--tableWidth;
			}

			std::cout << border.top.left;
			repeat(border.horizontal, tableWidth);
			std::cout << border.top.right << std::endl;

			printCell(title, tableWidth);
			std::cout << border.vertical << std::endl;
		}

		if (headerOrientation == HeaderOrientation::COLUMN) {
			if (title.empty() || page > 0)
				printLine(columnBegin, columnEnd, border.top);
			else
				printLine(columnBegin, columnEnd, {border.middle.left, border.top.middle, border.middle.right});

			for (size_t i = columnBegin; i < columnEnd; ++i)
				printCell(header[i], columnsWidth[i + 1]->first);

			std::cout << border.vertical << std::endl;
			printLine(columnBegin, columnEnd, border.middle);
		} else {
			if (title.empty() || page > 0)
				printLine(columnBegin, columnEnd, border.top);
			else
				printLine(columnBegin, columnEnd, {border.middle.left, border.top.middle, border.middle.right});
		}

		for (size_t r = 0; r < getRowCount(); ++r) {
			if (headerOrientation == HeaderOrientation::ROW) {
				printCell(header[r], columnsWidth.front()->first);
				std::cout << border.vertical;
			}

			for (size_t c = columnBegin; c < columnEnd; ++c)
				printCell(data[r]->at(c)->text, columnsWidth[c + 1]->first);

			std::cout << border.vertical << std::endl;

			if (r < getRowCount() - 1)
				printLine(columnBegin, columnEnd, border.middle);
		}

		printLine(columnBegin, columnEnd, border.botton);
		std::cout << std::endl;
	}
};
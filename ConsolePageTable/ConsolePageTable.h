#pragma once
#include <string>
#include <vector>
#include <iostream>

class PageTable {
public:
	enum class HeaderOrientation { COLUMN, ROW, NONE };

private:
	struct Line {
		const unsigned char left, middle, right;
	};
	struct {
		const unsigned char clearSpace = 32;
		const unsigned char horizontal = 196;
		const unsigned char vertical = 179;
		Line top = { 218,194,191 }, middle = { 195,197,180 }, botton = { 192,193, 217 };
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

		const Cell& operator=(const bool& value) {
			text = value ? "true" : "false";
			return *this;
		}

		template<typename T>
		const Cell& operator=(const T& value) requires std::integral<T> || std::floating_point<T> {
			text = std::to_string(value);
			return *this;
		}
	};

	HeaderOrientation headerOrientation;
	unsigned short columnsForPage;
	std::string title;
	std::vector<std::unique_ptr<std::pair<size_t, bool>>> columnsWidth;
	std::vector<std::string> header;
	std::vector<std::unique_ptr<std::vector<std::unique_ptr<Cell>>>> data;

public:
	PageTable() = delete;
	explicit PageTable(const std::string& title, const size_t rowCount, const size_t columnCount, const HeaderOrientation headerOrientation = HeaderOrientation::NONE);
	explicit PageTable(const std::string& title, const HeaderOrientation headerOrientation = HeaderOrientation::NONE);
	explicit PageTable(const size_t rowCount, const size_t columnCount);
	~PageTable();

	void addHeader(const std::initializer_list<std::string>& header);

	void addHeader(const std::string& value);

	void updateHeaderAt(const size_t index, const std::string& value);

	template<typename T>
		requires std::integral<T> || std::floating_point<T> || std::same_as<T, const char*>
	void addRow(const std::initializer_list<T>&row) {
		setRowCount(getRowCount() + 1);
		const size_t columnCount = std::min(getColumnCount(), row.size());
		for (size_t i = 0; i < columnCount; ++i) {
			*data.back()->at(i) = *(row.begin() + i);
			updateColumnWidth(i + 1, data.back()->at(i)->text.size());
		}
	}

	template<typename T>
		requires std::integral<T> || std::floating_point<T> || std::same_as<T, const char*> || std::same_as<T, std::string>
	void addRow(const T row[], const size_t size) {
		setRowCount(getRowCount() + 1);
		const size_t columnCount = std::min(getColumnCount(), size);
		for (size_t i = 0; i < columnCount; ++i) {
			*data.back()->at(i) = row[i];
			updateColumnWidth(i + 1, data.back()->at(i)->text.size());
		}
	}

	template<typename Iterator>
	void addRow(Iterator begin, const Iterator end) {
		setRowCount(getRowCount() + 1);
		const size_t columnCount = getColumnCount();
		for (size_t i = 0; i < columnCount && begin != end; ++i, ++begin) {
			*data.back()->at(i) = *begin;
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
	void addColumn(const T column[], const size_t size)  requires std::integral<T> || std::floating_point<T> || std::same_as<T, const char*> || std::same_as<T, std::string> {
		setColumnCount(getColumnCount() + 1);
		const size_t rowCount = std::min(getRowCount(), size);
		for (size_t i = 0; i < rowCount; ++i) {
			*data.at(i)->back() = column[i];
			updateColumnWidth(getColumnCount(), data.at(i)->back()->text.size());
		}
	}

	// the requirements remain to be confirmed
	template<typename Iterator>
	void addColumn(Iterator begin, const Iterator end) {
		setColumnCount(getColumnCount() + 1);
		const size_t rowCount = getRowCount();
		for (size_t i = 0; i < rowCount && begin != end; ++i, ++begin) {
			*data.at(i)->back() = *begin;
			updateColumnWidth(getColumnCount(), data.at(i)->back()->text.size());
		}
	}

	template<typename T>
	void updateValueAt(const size_t row, const size_t column, const T value) requires std::integral<T> || std::floating_point<T> || std::same_as<T, const char*> || std::same_as<T, std::string> {
		if (row < getRowCount() && column < getColumnCount()) {
			*data.at(row)->at(column) = value;
			updateColumnWidth(column + 1, data.at(row)->at(column)->text.size());
		}
	}

	size_t getColumnCount() const;

	void setColumnCount(const size_t columns);

	size_t getRowCount() const;

	void setRowCount(const size_t rows);

	void setColumnMaxWidth(size_t index, const size_t width);

	void setColumnAutoWidth(size_t index);

	void print();

	HeaderOrientation getHeaderOrientation() const;
	void setHeaderOrientation(const HeaderOrientation headerOrientation);

	unsigned short getColumnsForPage() const;
	void setColumnsForPage(unsigned short columns);

	const std::string getTitle() const;
	void setTitle(const std::string& title);

private:
	void updateColumnWidth(const size_t index, const size_t width, const bool force = false);

	inline void repeat(const unsigned char caracter, const size_t times) const;

	void printLine(size_t begin, const size_t end, const Line line) const;

	void printText(const std::string& text, const size_t width) const;

	void printPage(const unsigned short page);
};
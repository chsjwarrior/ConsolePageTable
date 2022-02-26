#include "ConsolePageTable.h"

PageTable::PageTable(const std::string& title, const size_t rowCount, const size_t columnCount, const HeaderOrientation headerOrientation)
	: headerOrientation(headerOrientation), columnsForPage(4), title(title) {
	setRowCount(rowCount);
	setColumnCount(columnCount);
}
PageTable::PageTable(const std::string& title, const HeaderOrientation headerOrientation) : PageTable(title, 0, 0, headerOrientation) {}
PageTable::PageTable(const size_t rowCount, const size_t columnCount) : PageTable("", rowCount, columnCount) {}
PageTable::~PageTable() {
	title.clear();
	columnsWidth.clear();
	header.clear();
	for (auto& r : data)
		r->clear();
	data.clear();
}

void PageTable::addHeader(const std::initializer_list<std::string>& header) {
	if (headerOrientation == HeaderOrientation::NONE)
		return;

	size_t i = 0;
	if (headerOrientation == HeaderOrientation::COLUMN) {
		i = getColumnCount();
		setColumnCount(i + header.size());
	}
	else if (headerOrientation == HeaderOrientation::ROW) {
		i = getRowCount();
		setRowCount(i + header.size());
	}

	const size_t oldSize = i;
	for (i = 0; auto & value : header) {
		this->header[oldSize + i] = value;
		++i;
		if (headerOrientation == HeaderOrientation::COLUMN)
			updateColumnWidth(oldSize + i, value.size(), true);
		else if (headerOrientation == HeaderOrientation::ROW)
			updateColumnWidth(0, value.size(), true);
	}
}

void PageTable::addHeader(const std::string& value) {
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

void PageTable::updateHeaderAt(const size_t index, const std::string& value) {
	if (headerOrientation == HeaderOrientation::NONE)
		return;

	if (headerOrientation == HeaderOrientation::COLUMN) {
		if (index >= getColumnCount())
			return;
	}
	else if (headerOrientation == HeaderOrientation::ROW) {
		if (index >= getRowCount())
			return;
	}

	header[index] = value;
	if (headerOrientation == HeaderOrientation::COLUMN)
		updateColumnWidth(index + 1, value.size(), true);
	else if (headerOrientation == HeaderOrientation::ROW)
		updateColumnWidth(0, value.size(), true);
}

//addRow
//addColumn

const size_t PageTable::getColumnCount() const {
	if (columnsWidth.empty())
		return 0;
	return columnsWidth.size() - 1;
}

void PageTable::setColumnCount(const size_t columns) {
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

const size_t PageTable::getRowCount() const {
	return data.size();
}

void PageTable::setRowCount(const size_t rows) {
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

void PageTable::setColumnMaxWidth(size_t index, const size_t width) {
	if (index < getColumnCount()) {
		columnsWidth[++index]->first = width;
		columnsWidth[index]->second = true;
	}
}

void PageTable::setColumnAutoWidth(size_t index) {
	if (index < getColumnCount()) {
		columnsWidth[++index]->second = false;
		columnsWidth[index]->first = headerOrientation == HeaderOrientation::COLUMN ? header[index - 1].size() : 0;
		for (auto& row : data)
			columnsWidth[index]->first = std::max(columnsWidth[index]->first, row->at(index - 1)->text.size());
	}
}

void PageTable::print() {
	if (getRowCount() + getColumnCount() > 1) {
		const unsigned short lenght = static_cast<unsigned short>(ceilf(static_cast<float>(getColumnCount()) / columnsForPage));
		for (unsigned short i = 0; i < lenght; ++i)
			printPage(i);
	}
}

const PageTable::HeaderOrientation PageTable::getHeaderOrientation() const { return headerOrientation; }
void PageTable::setHeaderOrientation(const HeaderOrientation headerOrientation) {
	this->headerOrientation = headerOrientation;
	if (headerOrientation == HeaderOrientation::COLUMN)
		setColumnCount(getColumnCount());
	else if (headerOrientation == HeaderOrientation::ROW)
		setRowCount(getRowCount());
	else
		header.clear();
}

const unsigned short PageTable::getColumnsForPage() const { return columnsForPage; }
void PageTable::setColumnsForPage(unsigned short columns) {
	if (columns < 3)
		columns = 3;
	columnsForPage = columns;
}

const std::string PageTable::getTitle() const { return title; }
void PageTable::setTitle(const std::string& title) { this->title = title; }

void PageTable::updateColumnWidth(const size_t index, const size_t width, const bool force) {
	if (columnsWidth[index]->second == false)
		if (force)
			columnsWidth[index]->first = width;
		else
			columnsWidth[index]->first = std::max(columnsWidth[index]->first, width);
}

inline void PageTable::repeat(const unsigned char caracter, const size_t times) const {
	for (size_t i = 0; i < times; ++i)
		std::cout << caracter;
}

void PageTable::printLine(size_t begin, const size_t end, const Line line) const {
	std::cout << line.left;
	if (headerOrientation == HeaderOrientation::ROW) {
		repeat(border.horizontal, columnsWidth.front()->first);
		std::cout << line.middle;
	}
	//if (begin != end)
	for (; begin < end; ++begin) {
		repeat(border.horizontal, columnsWidth[begin + 1]->first);
		if (begin < end - 1)
			std::cout << line.middle;
	}// else
		//repeat(border.horizontal, tableWidth);
	std::cout << line.right << std::endl;
}

void PageTable::printText(const std::string& text, const size_t width) const {
	if (text.empty())
		repeat(32, width);
	else if (text.size() < width) {
		std::cout << text;
		repeat(32, width - text.size());
	}
	else
		std::cout << text.substr(0, width);
}

void PageTable::printPage(const unsigned short page) {
	const size_t columnBegin = static_cast<size_t>(page) * columnsForPage;
	const size_t columnEnd = std::min(columnBegin + columnsForPage, getColumnCount());
	const size_t columnCount = columnEnd - columnBegin;

	size_t tableWidth = columnsWidth.front()->first;
	tableWidth += headerOrientation == HeaderOrientation::ROW ? columnCount : columnCount - 1;
	for (size_t i = columnBegin; i < columnEnd; ++i)
		tableWidth += columnsWidth[i + 1]->first;

	if (!title.empty() && page == 0) {
		if (title.size() > tableWidth) {
			tableWidth = columnsWidth.front()->first;
			size_t value = (title.size() - tableWidth) / columnCount;
			tableWidth += headerOrientation == HeaderOrientation::ROW ? columnCount : columnCount - 1;
			for (size_t i = columnBegin; i < columnEnd; ++i) {
				columnsWidth[i + 1]->first = value;
				tableWidth += value;
			}
		}
		std::cout << border.top.left;
		repeat(border.horizontal, tableWidth);
		std::cout << border.top.right << std::endl;

		std::cout << border.vertical;
		printText(title, tableWidth);
		std::cout << border.vertical << std::endl;

		printLine(columnBegin, columnEnd, { border.middle.left, border.top.middle, border.middle.right });
	}
	else
		printLine(columnBegin, columnEnd, border.top);

	if (headerOrientation == HeaderOrientation::COLUMN) {
		for (size_t i = columnBegin; i < columnEnd; ++i) {
			std::cout << border.vertical;
			printText(header[i], columnsWidth[i + 1]->first);
			if (i == columnEnd - 1)
				std::cout << border.vertical << std::endl;
		}
		printLine(columnBegin, columnEnd, border.middle);
	}

	for (size_t r = 0; r < getRowCount(); ++r) {
		if (headerOrientation == HeaderOrientation::ROW) {
			std::cout << border.vertical;
			printText(header[r], columnsWidth.front()->first);
		}

		for (size_t c = columnBegin; c < columnEnd; ++c) {
			std::cout << border.vertical;
			printText(data[r]->at(c)->text, columnsWidth[c + 1]->first);
		}

		std::cout << border.vertical << std::endl;

		if (r < getRowCount() - 1)
			printLine(columnBegin, columnEnd, border.middle);
	}

	printLine(columnBegin, columnEnd, border.botton);
	std::cout << std::endl;
}
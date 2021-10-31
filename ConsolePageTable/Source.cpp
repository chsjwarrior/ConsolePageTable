#include "ConsolePageTable.h"

int main() {
	Table table(1, 0);
	table.setHeaderOrientation(Table::HeaderOrientation::COLUMN);

	//table.addHeader({"Vi","Di","Pi"});

	/*
	table.addColumn({"1", "0", "nil"});
	table.addColumn({"2", "1", "1"});
	table.addColumn({"3", "2", "2"});
	table.addColumn({"4", "3", "3"});
	table.addColumn({"5", "4", "2"});
	*/
	table.print();

	return 0;
}
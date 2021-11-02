#include "ConsolePageTable.h"

int main() {
	Table table("01234567890123456789", 1, 3);
	table.setHeaderOrientation(Table::HeaderOrientation::NONE);

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
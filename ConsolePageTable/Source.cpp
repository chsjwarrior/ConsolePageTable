#include "ConsolePageTable.h"

int main() {
	Table table("Console Page Table", 0, 0);
	table.setHeaderOrientation(Table::HeaderOrientation::COLUMN);

	//table.addHeader({"codigo","descricao","quantidade","valor", "disponivel"});
	/*
	table.updateValueAt(0, 0, 1);
	table.updateValueAt(0, 1, "Computer");
	table.updateValueAt(0, 2, '1');
	table.updateValueAt(0, 3, 10000.00);
	table.updateValueAt(0, 4, true);
	*/

	table.print();

	return 0;
}
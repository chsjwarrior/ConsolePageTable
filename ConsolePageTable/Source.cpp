#include "ConsolePageTable.h"

int main() {
	Table table("Console Page Table", 1, 0);
	table.setHeaderOrientation(Table::HeaderOrientation::COLUMN);

	table.addHeader({"codigo","descricao","quantidade","valor"});

	table.print();

	return 0;
}
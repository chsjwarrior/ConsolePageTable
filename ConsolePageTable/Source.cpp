#include "ConsolePageTable.h"

int main() {
	PageTable table("01234567890123456789", 0, 9);
	table.setHeaderOrientation(PageTable::HeaderOrientation::ROW);
	table.setColumnsForPage(9);

	std::vector<char> array1 = {'A','B','C','D','E','F','G','H','I'};
	std::vector<int> array2 = {1,2,3,4,5,6,7,8,9};
	std::vector<char> array3 = {'N','A','A','A','A','B','F','F','H'};

	table.addRow(array1.begin(), array1.end());
	table.addRow(array2.begin(), array2.end());
	table.addRow(array3.begin(), array3.end());

	table.updateHeaderAt(0,"Vi");
	table.updateHeaderAt(1,"Di");
	table.updateHeaderAt(2,"Pi");

	table.print();	

	return 0;
}
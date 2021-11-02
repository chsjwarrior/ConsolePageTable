#include "ConsolePageTable.h"
#include <array>

int main() {
	PageTable table("01234567890123456789", 0, 5);
	table.setHeaderOrientation(PageTable::HeaderOrientation::NONE);

	//table.addHeader({"Vi","Di","Pi"});	

	table.print();	

	return 0;
}
#include "bplus_tree.hh"
#include <time.h> 

int main()
{
	BTree btree = BTree();
	// random test
	srand(time(NULL));
	for(int i = 0; i < 5000; ++i)
	{
		int k = rand()%4000;
		std::cout << "insert " << k << std::endl;
		btree.insert(k);
	}

	// sequential test
	for(int i = 0; i < 1000; ++i)
	{
		std::cout << "insert " << i << std::endl;
		btree.insert(i);
	}

	for(int i = 134; i < 578; ++i)
	{
		std::cout << "insert " << i << std::endl;
		btree.insert(i);
	}

	for(int i = 0; i < 3000; ++i)
	{
		int k = rand()%4000;
		if(k == 999)
			continue;
		std::cout << "remove " << k << std::endl;
		btree.remove(k);
		std::cout << "remove done" << std::endl;
	}

	btree.lookUp(0, 5000);

	std::cout << "single key test" << std::endl;
	btree.lookUp(999);

	srand(time(NULL));
	for(int i = 0; i < 5000; ++i)
	{
		int k = rand()%4000;
		std::cout << "insert " << k << std::endl;
		btree.insert(k);
	}

	btree.lookUp(349, 4000);

	return 0;
}
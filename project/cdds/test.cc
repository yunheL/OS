#include "bplus_tree.hh"
#include <time.h> 

void shuffle(int* arr, unsigned size)
{
<<<<<<< HEAD
	assert(size > 0);
	for(int i = 0; i < size; ++i)
=======
	BTree btree = BTree();
	// // random test
	// srand(time(NULL));
	// for(int i = 0; i < 5000; ++i)
	// {
	// 	int k = rand()%4000;
	// 	// std::cout << "insert " << k << std::endl;
	// 	btree.insert(k);
	// }

	// sequential test
	for(int i = 0; i < 1000000; ++i)
>>>>>>> c598ba7611e7c08e601c90067a0fb631a757f36e
	{
		int j = i + rand() % (size - i);

<<<<<<< HEAD
		assert(j < size);
		std::swap(arr[i], arr[j]);
	}
}

int main(int argc, char* argv[])
{
	assert(argc == 3);
	BTree btree = BTree();
	timespec time1, time2;

	int num = atoi(argv[1]);
	int test_num = atoi(argv[2]);

	int ctr = 1;
	unsigned time_arr[4] = {0, 0, 0, 0};
	unsigned mfenceTotal[4] = {0, 0, 0, 0};
	unsigned clflushTotal[4] = {0, 0, 0, 0};

	for(; ctr <= test_num; ++ctr)
	{
		// sequential insert & remove
		clock_gettime(CLOCK_MONOTONIC, &time1);
		for(int i = 0; i < num; ++i)
		{
			// std::cout << "insert " << i << std::endl;
			btree.insert(i);
		}
		clock_gettime(CLOCK_MONOTONIC, &time2);
		time_arr[0] += 1000000000*(time2.tv_sec - time1.tv_sec) + time2.tv_nsec - time1.tv_nsec;
		mfenceTotal[0] += mfenceCount;
		clflushTotal[0] += clflushCount;

		mfenceCount = 0;
		clflushCount = 0;


		clock_gettime(CLOCK_MONOTONIC, &time1);
		for(int i = 0; i < num/2; ++i)
		{
			// std::cout << "insert " << i << std::endl;
			btree.remove(i);
		}
		clock_gettime(CLOCK_MONOTONIC, &time2);
		time_arr[1] += 1000000000*(time2.tv_sec - time1.tv_sec) + time2.tv_nsec - time1.tv_nsec;
		mfenceTotal[1] += mfenceCount;
		clflushTotal[1] += clflushCount;

		mfenceCount = 0;
		clflushCount = 0;

		// random insert & remove
		srand(time(NULL));
		int* arr = new int[num];
		for(int i = 0; i < num; ++i)
		{
			arr[i] = i;
		}
		shuffle(arr, num);

		// for(int i = 0; i < num; ++i)
		// 	std::cout << arr[i] << std::endl;

		BTree btree_2 = BTree();
		// sequential insert & remove
		clock_gettime(CLOCK_MONOTONIC, &time1);
		for(int i = 0; i < num; ++i)
		{
			// std::cout << "insert " << i << std::endl;
			btree_2.insert(arr[i]);
		}
		clock_gettime(CLOCK_MONOTONIC, &time2);
		time_arr[2] += 1000000000*(time2.tv_sec - time1.tv_sec) + time2.tv_nsec - time1.tv_nsec;
		mfenceTotal[2] += mfenceCount;
		clflushTotal[2] += clflushCount;

		mfenceCount = 0;
		clflushCount = 0;


		clock_gettime(CLOCK_MONOTONIC, &time1);
		for(int i = 0; i < num/2; ++i)
		{
			// std::cout << "insert " << i << std::endl;
			btree_2.remove(arr[i]);
		}
		clock_gettime(CLOCK_MONOTONIC, &time2);
		time_arr[3] += 1000000000*(time2.tv_sec - time1.tv_sec) + time2.tv_nsec - time1.tv_nsec;
		mfenceTotal[3] += mfenceCount;
		clflushTotal[3] += clflushCount;

		mfenceCount = 0;
		clflushCount = 0;
	}

	std::cout << "sequential insert " << std::endl;
	std::cout << "time in ns = " << time_arr[0]/ctr << std::endl;
	std::cout << "# mfence = " << mfenceTotal[0]/ctr << std::endl;
	std::cout << "# clflush = " << clflushTotal[0]/ctr << std::endl;

	std::cout << "sequential remove " << std::endl;
	std::cout << "time in ns = " << time_arr[1]/ctr << std::endl;
	std::cout << "# mfence = " << mfenceTotal[1]/ctr << std::endl;
	std::cout << "# clflush = " << clflushTotal[1]/ctr << std::endl;

	std::cout << "random insert " << std::endl;
	std::cout << "time in ns = " << time_arr[2]/ctr << std::endl;
	std::cout << "# mfence = " << mfenceTotal[2]/ctr << std::endl;
	std::cout << "# clflush = " << clflushTotal[2]/ctr << std::endl;

	std::cout << "random remove " << std::endl;
	std::cout << "time in ns = " << time_arr[3]/ctr << std::endl;
	std::cout << "# mfence = " << mfenceTotal[3]/ctr << std::endl;
	std::cout << "# clflush = " << clflushTotal[3]/ctr << std::endl;
=======
//	for(int i = 0; i < 100; ++i)
//	{
		// std::cout << "insert " << i << std::endl;
//		btree.insert(i);
//	}

//	for(int i = 134; i < 578; ++i)
//	{
		// std::cout << "insert " << i << std::endl;
//		btree.insert(i);
//	}

	// for(int i = 0; i < 3000; ++i)
	// {
	// 	int k = rand()%4000;
	// 	if(k == 999)
	// 		continue;
	// 	// std::cout << "remove " << k << std::endl;
	// 	btree.remove(k);
	// 	// std::cout << "remove done" << std::endl;
	// }

//	for(int i = 0; i < 700; ++i)
//	{
//		btree.remove(i + 100);
		// std::cout << "remove done" << std::endl;
//	}

//	btree.lookUp(54, 913);

//	std::cout << "single key test" << std::endl;
	// btree.lookUp(0, 999);
//	btree.lookUp(933);
>>>>>>> c598ba7611e7c08e601c90067a0fb631a757f36e

	return 0;
}

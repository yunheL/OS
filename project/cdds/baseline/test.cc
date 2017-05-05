#include "bplus_tree.hh"
#include <time.h> 

void shuffle(int* arr, unsigned size)
{
	assert(size > 0);
	for(int i = 0; i < size; ++i)
	{
		int j = i + rand() % (size - i);

		assert(j < size);
		std::swap(arr[i], arr[j]);
	}
}

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		std::cout << "2 args needed" << std::endl;
		std::cout << "1st arg: # records inserted" << std::endl;
		std::cout << "2nd arg: # tests to run for average" << std::endl;
		exit(0);
	}
	timespec time1, time2;

	int num = atoi(argv[1]);
	int test_num = atoi(argv[2]);

	int ctr = 1;
	unsigned time_arr[4] = {0, 0, 0, 0};

	for(; ctr <= test_num; ++ctr)
	{
		BTree btree = BTree();
		// sequential insert & remove
		clock_gettime(CLOCK_MONOTONIC, &time1);
		for(int i = 0; i < num; ++i)
		{
			// std::cout << "insert " << i << std::endl;
			btree.insert(i);
		}
		clock_gettime(CLOCK_MONOTONIC, &time2);
		time_arr[0] += 1000000000*(time2.tv_sec - time1.tv_sec) + time2.tv_nsec - time1.tv_nsec;


		clock_gettime(CLOCK_MONOTONIC, &time1);
		for(int i = 0; i < num/2; ++i)
		{
			// std::cout << "insert " << i << std::endl;
			btree.remove(i);
		}
		clock_gettime(CLOCK_MONOTONIC, &time2);
		time_arr[1] += 1000000000*(time2.tv_sec - time1.tv_sec) + time2.tv_nsec - time1.tv_nsec;


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


		clock_gettime(CLOCK_MONOTONIC, &time1);
		for(int i = 0; i < num/2; ++i)
		{
			// std::cout << "insert " << i << std::endl;
			btree_2.remove(arr[i]);
		}
		clock_gettime(CLOCK_MONOTONIC, &time2);
		time_arr[3] += 1000000000*(time2.tv_sec - time1.tv_sec) + time2.tv_nsec - time1.tv_nsec;
	}

	std::cout << "sequential insert " << std::endl;
	std::cout << "time in ns = " << time_arr[0]/ctr << std::endl;

	std::cout << "sequential remove " << std::endl;
	std::cout << "time in ns = " << time_arr[1]/ctr << std::endl;

	std::cout << "random insert " << std::endl;
	std::cout << "time in ns = " << time_arr[2]/ctr << std::endl;

	std::cout << "random remove " << std::endl;
	std::cout << "time in ns = " << time_arr[3]/ctr << std::endl;

	return 0;
}
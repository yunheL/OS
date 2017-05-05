#include "bplus_tree.hh"
#include <time.h> 

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		std::cout << "2 args needed" << std::endl;
		std::cout << "1st arg: leaf node branching factor" << std::endl;
		std::cout << "2nd arg: # tests to run for average" << std::endl;
		exit(0);
	}

	timespec time1, time2;

	int degree = atoi(argv[1]);
	int test_num = atoi(argv[2]);

	int ctr = 1;
	unsigned time_arr[2] = {0, 0};

	for(; ctr <= test_num; ++ctr)
	{
		BTree btree = BTree();
		// individual split
		for(int i = 0; i < degree; ++i)
		{
			// std::cout << "insert " << i << std::endl;
			btree.insert(i);
		}
		clock_gettime(CLOCK_MONOTONIC, &time1);
		btree.insert(degree);
		clock_gettime(CLOCK_MONOTONIC, &time2);
		time_arr[0] += 1000000000*(time2.tv_sec - time1.tv_sec) + time2.tv_nsec - time1.tv_nsec;


		for(int i = 1; i <= degree * 2; ++i)
		{
			// std::cout << "insert " << i << std::endl;
			btree.insert(i + degree);
		}
		clock_gettime(CLOCK_MONOTONIC, &time1);
		btree.remove(degree);
		clock_gettime(CLOCK_MONOTONIC, &time2);
		time_arr[1] += 1000000000*(time2.tv_sec - time1.tv_sec) + time2.tv_nsec - time1.tv_nsec;
	}

	std::cout << "individual split " << std::endl;
	std::cout << "time in ns = " << time_arr[0]/ctr << std::endl;

	std::cout << "individual remove " << std::endl;
	std::cout << "time in ns = " << time_arr[1]/ctr << std::endl;

	return 0;
}
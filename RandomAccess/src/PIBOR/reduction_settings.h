#ifndef REDSETTINGS
#define REDSETTINGS

#include <omp.h>
#include <string.h>

using namespace std;

namespace redompss{
	typedef class Settings
	{
	public:
		SIZE_T _per_thread_storage_size; //byte
		SIZE_T _num_regions;             //#
		SIZE_T _buffer_size;             //byte
		static enum MODES{AUTOSET_NUM_REGIONS, AUTOSET_BUF_SIZE, AUTOSET_TOTAL_SIZE};

		void init (SIZE_T per_thread_storage_size, SIZE_T num_regions, SIZE_T buffer_size, int prio )
		{
			SIZE_T num_threads = omp_get_num_threads();
			switch(prio)
			{
				case AUTOSET_NUM_REGIONS:
				{
					_per_thread_storage_size = per_thread_storage_size;
					_num_regions             = _per_thread_storage_size / buffer_size;
					_buffer_size             = buffer_size;
					//cout << "NUM_REG:" << _num_regions << "," << _per_thread_storage_size << "," << _buffer_size << endl;
					return;
				}
				case AUTOSET_BUF_SIZE:
				{
					_per_thread_storage_size = per_thread_storage_size;
					_buffer_size              = _per_thread_storage_size / num_regions;
					_num_regions             = num_regions;
					return;
				}
				case AUTOSET_TOTAL_SIZE:
				{
					_num_regions             = num_regions;
					_buffer_size             = buffer_size;
					_per_thread_storage_size = num_regions *  buffer_size;
					return;
				}
			}
			cout << "[PIBOR] No valid settings." << endl;
		}

		//default
		Settings(int prio)
		{
			SIZE_T per_thread_storage_size; //byte
			SIZE_T num_regions;             //#
			SIZE_T buffer_size;             //byte

			char * p1 = getenv("NUM_REGIONS");
			char * p2 = getenv("BUFFER_SIZE");
			char * p3 = getenv("PER_THREAD_STORAGE_SIZE");

			if(p1 == NULL)
				num_regions = 64;
			else
				num_regions = stoi(p1);

			if(p2 == NULL)
				buffer_size = 1024*1024;
			else
				buffer_size = stoi(p2);

			if(p3 == NULL)
				per_thread_storage_size = buffer_size * num_regions;
			else
				per_thread_storage_size = stoi(p3);

			//cout << num_regions << "," << per_thread_storage_size << "," << buffer_size << endl;
			init(per_thread_storage_size, num_regions, buffer_size, prio );
		}

		Settings(SIZE_T per_thread_storage_size, SIZE_T num_regions, SIZE_T buffer_size, int prio )
		{
			init(per_thread_storage_size, num_regions, buffer_size, prio );

		}
	};
}

#endif

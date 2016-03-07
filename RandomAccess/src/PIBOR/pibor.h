/*PIBOR2*/
/*Jan Ciesko, Sergi Mateo, Xavi Teruel, BSC, 2015*/

#ifndef PIBORV2CMT
#define PIBORV2CMT

#include "reduction_storage.h"

namespace redompss{
	template <typename E, typename G>
	class PIBOR : public APPROACH_T<E,G>
	{

		#define  LOCK_BUSY 1
		#define LOCK_FREE 0

	public:
		typedef struct entry_t {
			E val;
			E * tag;
		};

		typedef struct red_buffer_t {
			entry_t * entries;
			SIZE_T fill;
		};

	private:
		//General parameters
		red_buffer_t * buffers;
		Settings * settings;
		SIZE_T threadID;

		//Pointer to original data structure (which is a pointer)
		E * orig;
		E * max_adr;
		G * locks;

		//Computed parameters
		SIZE_T num_buffer_entries;
		//SIZE_T region_size;
		int low_bits_offset;


		//Tools
		SIZE_T compute_log2(ptr_t in) {
			SIZE_T out;
			for (in *= 0.5, out = 0; in >= 1.0; in *= 0.5, out++)
				;
			return out;
		}

		inline void lock_acquire_variant_1(int lock_id) {
			volatile char * state = &this->locks[lock_id];
			if (!__sync_val_compare_and_swap(state, LOCK_FREE, LOCK_BUSY))
			return;

			nanos_instrument_begin_burst("reduction-lock", "Reduction Spin", "spin", "Spin");
			spin: while (*state == LOCK_BUSY) {}
			if (__sync_lock_test_and_set(state, LOCK_BUSY))
				goto spin;
				nanos_instrument_end_burst("reduction-lock", "spin");
			return;
		}

		inline int lock_acquire_variant_2(int lock_id) {
			return __sync_val_compare_and_swap(&this->locks[lock_id], LOCK_FREE, LOCK_BUSY);
		}

		inline void lock_acquire_variant_3(volatile char * lock) {
			if (!__sync_val_compare_and_swap(lock, LOCK_FREE, LOCK_BUSY))
			return;

			nanos_instrument_begin_burst("reduction-lock", "Reduction Spin", "spin",
			"Spin");
			spin: while (*lock == LOCK_BUSY) {}
			if (__sync_lock_test_and_set(lock, LOCK_BUSY))
			goto spin;
			nanos_instrument_end_burst("reduction-lock", "spin");
			return;
		}

		inline void lock_release_variant_1(int lock_id) {
			volatile char * state = &this->locks[lock_id];
			__sync_lock_release(state);
		}

		inline void lock_release_variant_2(int lock_id) {
			this->locks[lock_id] = (char) LOCK_FREE;
		}

		inline void lock_release_variant_3(volatile char * lock) {
			__sync_lock_release(lock);
		}

	public:
		PIBOR(Implementation <E,G> * c,  void (op)(E *, E*), Settings * s)
		{
			settings           = s;
			orig               = c->target;
			this->_op          = op;
			this->locks        = c->gdata;
			this->target_size  = c->target_size;

			//compute address offset to use most-significant bits as region identifier
			int regions;
			max_adr = (E*)this->target_size-1; //the highest address relative to 0

			int adr_norm_log = compute_log2((ptr_t)max_adr);
			for (int i = adr_norm_log; i >= 0; --i) {
				ptr_t msbits = ((ptr_t)max_adr >> i);
				if (msbits >= settings->_num_regions) {
					//cout << adr_norm_log << "," << i <<"," << settings->_num_regions << endl;
					low_bits_offset = i;
					regions = msbits + 1;
					break;
				}
			}

			settings->_num_regions = regions;
			num_buffer_entries     = settings->_buffer_size / sizeof(entry_t);
			threadID               = omp_get_thread_num();

			//prepare memory for buffers
			buffers = (red_buffer_t *) malloc (settings->_num_regions * sizeof(red_buffer_t));

			for(int i = 0; i < settings->_num_regions; ++i)
			{
				buffers[i].entries = NULL;
				buffers[i].fill = 0;
			}
			//cout << "PIBOR STATS:" << threadID <<"," <<orig  << ","<< this->target_size  << "," << settings->_num_regions << "," << num_buffer_entries << endl;
		}

		~PIBOR()
		{
			if(buffers != NULL){
				for (SIZE_T i = 0; i < settings->_num_regions; ++i)
				{
					if(buffers[i].entries != NULL)
						free (buffers[i].entries);
				}
				free(buffers);
				delete settings;
			}
		}

		void op(E * a , E * b)
		{
			entry_t * s = getStorage(a);
			s->tag =  a;
			s->val = *b;
		}

		void reducer()
		{
			for (SIZE_T i = 0; i < settings->_num_regions; ++i)
			{

				red_buffer_t * buf ( & buffers[i] );

				if(buf == NULL) continue;
				//cout << "Reducing 2:" << orig << "," << i << ","<<  buf->fill << "," << num_buffer_entries <<  endl;
				single_reduce(buf);
			}
		}

	private:
		void inline single_reduce(red_buffer_t * buf) {
			for (SIZE_T i = 0; i < buf->fill; i++) {
				this->_op(buf->entries[i].tag, &buf->entries[i].val);
			}
			buf->fill = 0;
		}

		inline entry_t * getStorage(E * a)
		{
			int i = ((char*)a - (char*)orig) >> low_bits_offset;

			red_buffer_t * buf ( & buffers[i] );

			if (buf->entries == NULL) {
					posix_memalign((void**) &buf->entries, 4096, settings->_buffer_size);
					buf->fill = 0;
			}

			if (buf->fill >= num_buffer_entries) {
				int j = i;
				//cout << "Reducing 1:" << orig << "," << i << "," << buf->fill <<"," << num_buffer_entries << endl;
				nanos_instrument_begin_burst("reduction-lock", "Reduction Spin", "spin", "Spin");
				while (buf->fill) {
					if (this->lock_acquire_variant_2(i)) {
						single_reduce(buf);
						this->lock_release_variant_2(i);
					} else {
						//try to reduce a buffer for another region
						j = ++j % settings->_num_regions;
						if (buffers[j].fill
								> num_buffer_entries - num_buffer_entries / 8) {
							if (!this->lock_acquire_variant_2(j)) {
								single_reduce(&buffers[j]);
								this->lock_release_variant_2(j);
							}
						}
					}
				}//cycle while buffer is not emptied
				nanos_instrument_end_burst("reduction-lock", "spin");
			}
			return &buf->entries[buf->fill++];
		}
	};
}
#endif

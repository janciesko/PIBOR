/*PIBOR2*/
/*Jan Ciesko, Sergi Mateo, Xavi Teruel, BSC, 2015*/

#ifndef REDSTRG
#define REDSTRG

typedef unsigned long int SIZE_T;
typedef uint64_t ptr_t;

#include "reduction_settings.h"

namespace redompss{
	template <typename E, typename G>
	virtual class APPROACH_T
	{
	public:
		E        * target;
		G        * gdata;
		SIZE_T     target_size;
		Settings * settings;


		//constructor
		//virtual APPROACH_T (Implementation <E,G> *,  void (op)(E *, E*), Settings * )=0;

		//this is the operation that implements the approach-specific operation (of _op)
		virtual void op (E *, E *) = 0;
		//this is implementor provided
		virtual void reducer() = 0;

		virtual ~APPROACH_T () {};

	protected:
		//this is user provided elemental function
		void (*_op) (E *, E *);
	};

	template <typename E, typename G>
	class Implementation
	{
	public:
		//This is is the global reduction data
		E * target;
		//target size
		SIZE_T target_size;
		//This is a approach to an approach (such as PIBOR)
		APPROACH_T<E, G> * approach;
		//Global data used by some approaches
		G * gdata;

		//this is the element-wise reducer
		inline void op (E * a, E * b){
			approach->op(a, b);
		}

		//This is the reducer for the approach
		void reduce () {
			approach->reducer();
			delete approach;
		}

		//Setter
		void init(APPROACH_T<E, G> * h, Implementation <E,G> * c ){
			approach    = h;
			target      = c->target;
			gdata       = c->gdata;
			target_size = c->target_size;
		}

		//Constructor
		Implementation(E * t, SIZE_T s, SIZE_T gds){
			target      = t;
			target_size = s;
			approach    = 0; //set this later in the initializer
			gdata       = (G * ) calloc (1,gds);
		}

		//Empty constructor (used by RT to create private copies)
		Implementation() {
			target      = 0;
			approach    = 0;
			target_size = 0;
			gdata       = 0;
		}

		~Implementation()
		{
			if(approach != NULL)
				delete approach;
		}

	};

}

#endif

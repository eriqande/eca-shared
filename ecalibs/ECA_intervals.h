
enum IntOnOff
{
	IntOff,  /* 0 means the interval is ending */
	IntOn	 /* 1 means the interval is beginning */
};




typedef struct
{
	enum IntOnOff Q;  /*  the "quality of the interval boundary (On or Off) */
	double v;	 /*  the real number value (position) of the interval boundary */

} IntBoundaryStruct;

typedef struct
{
	int NumBounds;   		/*  the number of interval boundaries (both on and off) */
	double lo;				/*  for the lowest lower boundary */
	double hi;				/*  for the highest higher boundary */
	IntBoundaryStruct *B;	/*  the boundaries themselves */

}  IntervalsStruct;



/* Function Prototypes*/


/******************************************************************************************/
/*                                                                                        */
/*		       MEMORY ALLOCATION/DEALLOCATION AND COPYING                                 */
/*                                                                                        */  
/******************************************************************************************/
IntervalsStruct *AllocIntervals(int NumBounds);
void DestroyIntervals(IntervalsStruct *T);
IntervalsStruct *CopyIntervals(IntervalsStruct *T);




/******************************************************************************************/
/*                                                                                        */
/*		       BASIC UTILITIES FOR SINGLE INTERVAL STRUCTURES                             */
/*                                                                                        */  
/******************************************************************************************/
void CondenseIntervals(IntervalsStruct **C);
void RemoveEmptyIntervals(IntervalsStruct **C);
void PrintIntervals(IntervalsStruct *A);




/******************************************************************************************/
/*                                                                                        */
/*		            FUNCTIONS FOR INTERSECTIONS, UNIONS, ETC                              */
/*                                                                                        */  
/******************************************************************************************/
IntervalsStruct *IntervalsIntersection(IntervalsStruct *T1, IntervalsStruct *T2);
IntervalsStruct *IntersectOfArrayOfIntervals(IntervalsStruct **A, int Num);
IntervalsStruct *IntervalsUnion(IntervalsStruct *T1, IntervalsStruct *T2);
IntervalsStruct *IntervalsSubtract(IntervalsStruct *T1, IntervalsStruct *T2);




/******************************************************************************************/
/*                                                                                        */
/*		            FUNCTIONS FOR QUERIES ABOUT INTERVALS                                 */
/*                                                                                        */  
/******************************************************************************************/
int RecursiveFindIntBoundary(double P, IntBoundaryStruct *A, int lo, int hi);
int PointIsInActiveInterval(double P, IntervalsStruct *T);





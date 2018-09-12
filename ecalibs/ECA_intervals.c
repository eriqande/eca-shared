
/*

	These are a series of functions for defining intervals and
	then performing intersections and unions, etc on them.
	
	This code is pretty ugly, and yucky.  We'll see how it goes.

*/
#include <stdio.h>
#include <stdlib.h>
#include "ECA_MemAlloc.h"
#include "eca_intervals.h"



/******************************************************************************************/
/*                                                                                        */
/*		       MEMORY ALLOCATION/DEALLOCATION AND COPYING                                 */
/*                                                                                        */  
/******************************************************************************************/
IntervalsStruct *AllocIntervals(int NumBounds)
{
	int i;
	IntervalsStruct *temp;
	
	if(NumBounds%2 != 0) {
		printf("\n\nWarning!  NumBounds not an even number in AllocIntervals()\n\n");
	}
	
	temp = (IntervalsStruct *)ECA_MALLOC(sizeof(IntervalsStruct));
	
	temp->NumBounds = NumBounds;
	
	/* then do some initializations to "neutral-ish values"  */
	temp->lo = 0.0;
	temp->hi = 0.0;

	if(NumBounds == 0)  {
		temp->B = NULL;  
		return(temp);
	}
	else {
		temp->B = (IntBoundaryStruct *)ECA_CALLOC((size_t)NumBounds, sizeof(IntBoundaryStruct));
		for(i=0;i<NumBounds;i++)  {
			temp->B[i].Q = (i%2==1) ?  IntOff : IntOn;
			temp->B[i].v = 0.0;
		}
	}
	
	return(temp);
	
}

void DestroyIntervals(IntervalsStruct *T)
{
	if( T != NULL)  {
		if( T->B != NULL)  {
			free( T->B);
		}
		free( T );
	}
}


/* Allocates memory to and copies values for another copy of an interval struct  */
IntervalsStruct *CopyIntervals(IntervalsStruct *T) 
{	
	int i;
	IntervalsStruct *R;
	
	R = AllocIntervals(T->NumBounds);
	
	if(T->NumBounds > 0) {  /* in this case, copy the values over */
		R->lo = T->lo;
		R->hi = T->hi;
		for(i=0;i<R->NumBounds;i++)  {
			R->B[i].Q = T->B[i].Q;
			R->B[i].v = T->B[i].v;
		}
		
	}
	return(R);
}


/******************************************************************************************/
/*                                                                                        */
/*		       BASIC UTILITIES FOR SINGLE INTERVAL STRUCTURES                             */
/*                                                                                        */  
/******************************************************************************************/

/*  this should be called after making a union of intervals to condense them down into the */
/*  minimum number of boundaries.   This removes "double boundaries of the following sort:
	(i) IntOff boundary occurs at the end of an active interval that happens to end
	exactly where an IntOn Boundary occurs, and a new interval starts.
*/
void CondenseIntervals(IntervalsStruct **C)
{
	int i;
	IntervalsStruct *temp;
	int Active;
	int NewNumBounds;
	
	/*  first, count the number of unique boundaries.  Cycle up to one */
	/*  before the last boundary */
	Active = 0;
	NewNumBounds = 0;
	for(i=0;i<(*C)->NumBounds-1;i++)  {
		if(Active==0 &&  (*C)->B[i].Q == IntOn) {
			Active = 1;
			NewNumBounds++;
		}
		else if(Active==1)  {
			if((*C)->B[i].v == (*C)->B[i+1].v)  /*  in this case, skip over the current bound and */
														 /*  the next bound */
					i++;
			else {
				Active = 0;
				NewNumBounds++;
			} 
		}
	}
	/*  add one more to NewNumBounds for the last one (if there was at least one that was found) */
	if(NewNumBounds != 0)
		NewNumBounds++;
	
	/*  allocate to temp */
	temp = AllocIntervals(NewNumBounds);
	
	/*  Then cycle again to condense things */
	Active = 0;
	NewNumBounds = 0;
	for(i=0;i<(*C)->NumBounds-1;i++)  {
		if(Active==0 &&  (*C)->B[i].Q == IntOn) {
			temp->B[NewNumBounds].Q = IntOn;
			temp->B[NewNumBounds].v =  (*C)->B[i].v;
			Active = 1;
			NewNumBounds++;
		}
		else if(Active==1)  {
			if((*C)->B[i].v == (*C)->B[i+1].v)  /*  in this case, skip over the current bound and */
														 /*  the next bound */
					i++;
			else {
				temp->B[NewNumBounds].Q = IntOff;
				temp->B[NewNumBounds].v =  (*C)->B[i].v;
				Active = 0;
				NewNumBounds++;
			} 
		}
	}
	
	if(NewNumBounds>0) {
		/*  and make the very last one an IntOff */
		temp->B[NewNumBounds].Q = IntOff;
		temp->B[NewNumBounds].v = (*C)->B[(*C)->NumBounds-1].v;
		
		/*  then set the lo and hi */
		temp->lo = temp->B[0].v;
		temp->hi = temp->B[temp->NumBounds-1].v;
	}
	
	/*  then obliterate C: */
	DestroyIntervals(*C);
	
	/*  Finally make C point to temp */
	(*C) = temp;
}


/*  Should be called after IntervalsSubtract in order to remove empty intervals.
	This should probably only be called after CondenseIntervals has been called  */
void RemoveEmptyIntervals(IntervalsStruct **C)
{
	int i;
	IntervalsStruct *temp;
	int NewNumBounds;
	
	/*  first, count the number of unique boundaries.  Cycle up to one */
	/*  before the last boundary */
	NewNumBounds = 0;
	for(i=0;i<(*C)->NumBounds-1;i+=2)  {
		if((*C)->B[i].Q != IntOn) {
			printf("\nBummer!  Even Boundary not IntOn in RemoveEmptyIntervals()\n\nExiting...");
			exit(1);
		}
		else if((*C)->B[i].v != (*C)->B[i+1].v)  {  /*  if the IntOn and IntOff boundaries are not identical */
			NewNumBounds+=2;
		}
	}
	
	
	/*  allocate to temp */
	temp = AllocIntervals(NewNumBounds);
	
	/*  Then cycle again to condense things */
	NewNumBounds = 0;
	for(i=0;i<(*C)->NumBounds-1;i+=2)  {
	
		if((*C)->B[i].v != (*C)->B[i+1].v)  {  /*  if the IntOn and IntOff boundaries are not identical */
			temp->B[NewNumBounds].Q = IntOn;
			temp->B[NewNumBounds].v =  (*C)->B[i].v;
			temp->B[NewNumBounds+1].Q = IntOff;
			temp->B[NewNumBounds+1].v =  (*C)->B[i+1].v;
			
			NewNumBounds+=2;
		}
	}
		
	/*  then set the lo and hi, only if there are actually bounds to do it with! */
	if(temp->NumBounds > 0) {
		temp->lo = temp->B[0].v;
		temp->hi = temp->B[temp->NumBounds-1].v;
	}
	
	/*  then obliterate C: */
	DestroyIntervals(*C);
	
	/*  Finally make C point to temp */
	(*C) = temp;
}

void PrintIntervals(IntervalsStruct *A)
{
	int i;
	
	printf("NumBounds = %d  lo = %.5f   hi = %.5f",A->NumBounds,A->lo,A->hi);
	for(i=0;i<A->NumBounds;i++)  {
		if(A->B[i].Q == IntOn) 
			printf("  [%.5f,",A->B[i].v);
		else
			printf("%.5f)",A->B[i].v);
	}
}


/******************************************************************************************/
/*                                                                                        */
/*		            FUNCTIONS FOR INTERSECTIONS, UNIONS, ETC                              */
/*                                                                                        */  
/******************************************************************************************/

/*  returns a pointer to an Intervals struct which holds the intersection */
/*  of the intervals found in T1 and T2 */
IntervalsStruct *IntervalsIntersection(IntervalsStruct *T1, IntervalsStruct *T2)
{
	IntervalsStruct *T3;
	int i,i1=0, i2 = 0;
	int N = T1->NumBounds + T2->NumBounds;
	int NumUp = 0;
	int Active = 0;
	int NewNumBounds = 0;
	double CurrentV;
	
	/*  if either of them have zero intervals, then the intersection will clearly */
	/*  have zero intervals: */
	if(T1->NumBounds == 0 || T2->NumBounds == 0) {
		T3 = AllocIntervals(0);
		return(T3);
	}
	
	/*  first go through it just to count up the number of boundaries that will be required */
	for(i=0;i<N;i++)  {
		if(T1->B[i1].v <= T2->B[i2].v)  {
			NumUp += T1->B[i1].Q == IntOn ? 1 : -1;
			i1++;
		}
		else  {
			NumUp += T2->B[i2].Q == IntOn ? 1 : -1;
			i2++;
		}
		if(NumUp > 2)  {
			printf("\n\nSorry.  Num Up > 2 in IntervalsIntersection. \n\nExiting to System...\n\n");
			exit(1);
		}
		else if(NumUp == 2)  {
			Active = 1;
			NewNumBounds++;
		}
		else if(NumUp == 1 && Active == 1) {
			Active = 0;
			NewNumBounds++;
		}
		/*  if you've gotten to the end of either list of boundaries, you know there won't */
		/*  be any more intersection here so just break out of the loop */
		if(i1 >= T1->NumBounds || i2 >= T2->NumBounds)
			break;
		
	}
	
	/*  Now that we know how many boundaries are needed.  Allocate space */
	/*  and assign values to those boundaries */
	T3 = AllocIntervals(NewNumBounds);
	
	/*  get out of here if it is the empty set: */
	if(NewNumBounds==0) 
		return(T3);
	
	Active = 0; NumUp = 0; NewNumBounds = 0; i1 = 0; i2 = 0;
	for(i=0;i<N;i++)  {
		if( T1->B[i1].v <= T2->B[i2].v)  {
			NumUp += T1->B[i1].Q == IntOn ? 1 : -1;
			CurrentV = T1->B[i1].v;
			i1++;
		}
		else  {
			NumUp += T2->B[i2].Q == IntOn ? 1 : -1;
			CurrentV = T2->B[i2].v;
			i2++;
		}
		if(NumUp > 2)  {
			printf("\n\nSorry.  Num Up > 2 in IntervalsIntersection. \n\nExiting to System...\n\n");
			exit(1);
		}
		else if(NumUp == 2)  {
			Active = 1;
			T3->B[NewNumBounds].Q = IntOn;
			T3->B[NewNumBounds].v = CurrentV;
			NewNumBounds++;
		}
		else if(NumUp == 1 && Active == 1) {
			Active = 0;
			T3->B[NewNumBounds].Q = IntOff;
			T3->B[NewNumBounds].v = CurrentV;
			NewNumBounds++;
		}
		
		/*  if you've gotten to the end of either list of boundaries, you know there won't */
		/*  be any more intersection here so just break out of the loop */
		if(i1 >= T1->NumBounds || i2 >= T2->NumBounds)
			break;
	}
	
	/*  that should have gotten all the information set in there.  Now we just need to  */
	/*  set the lo and the hi.  These should, by definition, be the values of the first  */
	/*  and the last boundaries */
	T3->lo = T3->B[0].v;
	T3->hi = T3->B[T3->NumBounds-1].v;
	
	/*  voila!  Now return the result */
	return(T3);
}


/*  find and return the intersection of a number of different intervals. */
/*  This returns the */
IntervalsStruct *IntersectOfArrayOfIntervals(IntervalsStruct **A, int Num)
{
	int i;
	IntervalsStruct *temp, *dtemp;
	
	
	dtemp = IntervalsIntersection(A[0],A[1]);
	
	if(Num==2)
	{
		temp = dtemp;
	}
	
	else {
		for(i=2;i<Num;i++)  {
			temp = IntervalsIntersection(dtemp,A[i]);
			DestroyIntervals(dtemp);
			dtemp = temp;
		}
	}
	/*  at the end of this, temp holds the result. */
	return(temp);
}




/*  returns a pointer to an Intervals struct which holds the union */
/*  of the intervals found in T1 and T2. */
/*  This is currently pretty clunky the way I have done it with  */
/*  the monstrous if statements.  I'm sure that I will eventually be  */
/*  able to clean this up considerably. */
IntervalsStruct *IntervalsUnion(IntervalsStruct *T1, IntervalsStruct *T2)
{
	IntervalsStruct *T3;
	int i,i1=0, i2 = 0;
	int N = T1->NumBounds + T2->NumBounds;
	int NumUp = 0;
	int Active = 0;
	int NewNumBounds = 0;
	double CurrentV;
	
	/*  if both have NumBounds = 0, then we return an empty set */
	if(T1->NumBounds==0 && T2->NumBounds==0) {
		T3 = AllocIntervals(0);
		return(T3);
	}
	/*  If one has NumBounds==0 and the other doesn't, then simply return  */
	/*  a copy of the one which has NumBounds > 0. */
	else if(T1->NumBounds==0 && T2->NumBounds > 0)  {
		T3 = CopyIntervals(T2);
		return(T3);
	}
	else if(T1->NumBounds > 0 && T2->NumBounds==0) {
		T3 = CopyIntervals(T1);
		return(T3);
	}
	
	
	/*  first go through it just to count up the number of boundaries that will be required */
	for(i=0;i<N;i++)  {
		if( (T1->B[i1].v <= T2->B[i2].v  && (i1 < T1->NumBounds && i2 < T2->NumBounds) )  ||
				(i1 < T1->NumBounds && i2 >= T2->NumBounds) )  {
			NumUp += T1->B[i1].Q == IntOn ? 1 : -1;
			i1++;
		}
		else if( (T1->B[i1].v > T2->B[i2].v  && (i1 < T1->NumBounds && i2 < T2->NumBounds) )  ||
				(i1 >= T1->NumBounds && i2 < T2->NumBounds) ) {
			NumUp += T2->B[i2].Q == IntOn ? 1 : -1;
			i2++;
		}
		if(NumUp > 2)  {
			printf("\n\nSorry.  Num Up > 2 in IntervalsIntersection. \n\nExiting to System...\n\n");
			exit(1);
		}
		else if(NumUp == 1  && Active == 0)  {
			Active = 1;
			NewNumBounds++;
		}
		else if(NumUp == 0 && Active == 1) {
			Active = 0;
			NewNumBounds++;
		}
	}
	
	/*  Now that we know how many boundaries are needed.  Allocate space */
	/*  and assign values to those boundaries */
	T3 = AllocIntervals(NewNumBounds);
	
	
	Active = 0; NumUp = 0; NewNumBounds = 0; i1 = 0; i2 = 0;
	for(i=0;i<N;i++)  {
		if( (T1->B[i1].v <= T2->B[i2].v  && (i1 < T1->NumBounds && i2 < T2->NumBounds) )  ||
				(i1 < T1->NumBounds && i2 >= T2->NumBounds) )  {
			NumUp += T1->B[i1].Q == IntOn ? 1 : -1;
			CurrentV = T1->B[i1].v;
			i1++;
		}
		else if( (T1->B[i1].v > T2->B[i2].v  && (i1 < T1->NumBounds && i2 < T2->NumBounds) )  ||
				(i1 >= T1->NumBounds && i2 < T2->NumBounds) ) {
			NumUp += T2->B[i2].Q == IntOn ? 1 : -1;
			CurrentV = T2->B[i2].v;
			i2++;
		}
		if(NumUp > 2)  {
			printf("\n\nSorry.  Num Up > 2 in IntervalsUnion. \n\nExiting to System...\n\n");
			exit(1);
		}
		else if(NumUp == 1  && Active == 0)  {
			Active = 1;
			T3->B[NewNumBounds].Q = IntOn;
			T3->B[NewNumBounds].v = CurrentV;
			NewNumBounds++;
		}
		else if(NumUp == 0 && Active == 1) {
			Active = 0;
			T3->B[NewNumBounds].Q = IntOff;
			T3->B[NewNumBounds].v = CurrentV;
			NewNumBounds++;
		}
	}
	
	/*  that should have gotten all the information set in there.  Now we just need to  */
	/*  set the lo and the hi.  These should, by definition, be the values of the first  */
	/*  and the last boundaries */
	T3->lo = T3->B[0].v;
	T3->hi = T3->B[T3->NumBounds-1].v;
	
	/*  WE NOW MUST CONDENSE THOSE INTERVALS: */
	CondenseIntervals(&T3);
	
	/*  voila!  Now return the result */
	return(T3);
}



/*  
	Subtracts intervals T2 from intervals T1, and returns the result
*/
IntervalsStruct *IntervalsSubtract(IntervalsStruct *T1, IntervalsStruct *T2)
{
	IntervalsStruct *T3;
	int i,i1=0, i2 = 0;
	int N = T1->NumBounds + T2->NumBounds;
	int NumUp1 = 0,NumUp2 = 0;
	int Active = 0;
	int NewNumBounds = 0;
	double CurrentV;
	
	/*  if T1 has NumBounds = 0, then we return an empty set */
	if(T1->NumBounds==0) {
		T3 = AllocIntervals(0);
		return(T3);
	}
	/*  If T2 has NumBounds==0 and T1 doesn't, then simply return  */
	/*  a copy of T1 which has NumBounds > 0. */
	else if(T2->NumBounds == 0)  {
		T3 = CopyIntervals(T1);
		return(T3);
	}
	
	
	/*  first go through it just to count up the number of boundaries that will be required */
	for(i=0;i<N;i++)  {
		
		if( (T1->B[i1].v <= T2->B[i2].v  && (i1 < T1->NumBounds && i2 < T2->NumBounds) )  ||
				(i1 < T1->NumBounds && i2 >= T2->NumBounds) )  {
			NumUp1 += T1->B[i1].Q == IntOn ? 1 : -1;
			i1++;
		}
		else if( (T1->B[i1].v > T2->B[i2].v  && (i1 < T1->NumBounds && i2 < T2->NumBounds) )  ||
				(i1 >= T1->NumBounds && i2 < T2->NumBounds) ) {
			NumUp2 += T2->B[i2].Q == IntOn ? 1 : -1;
			i2++;
		}
		if(NumUp1 < 0 || NumUp1 > 1 || NumUp2 > 1 || NumUp2 < 0)  {
			printf("\n\nSorry.  NumUp1 = %d, NumUp2 = %d.  One of those is neither 0 nor 1. \n\nExiting to System...from IntervalsSubtract()\n\n",NumUp1,NumUp2);
			exit(1);
		}
		/*  and here we deal with the conditions that determine whether we are adding a boundary or not */
		/*  This depends on three variables that may be 0 or 1.  So, we have eight possibilities */
		if(NumUp1 == 0)  {
			if(NumUp2==0)  {
				if(Active==1)  {
					NewNumBounds++;
					Active = 0;
				}
				/*  if active==0 we don't do anything */
			}
			else  {  /*  NumUp2 == 1 */
				if(Active==1)  {
					NewNumBounds++;
					Active = 0;
					printf("\n\nJust saw NumUp1==0, NumUp2==1, Active==1.  Shouldn't happen!!!\n\n)");
				}
				/*  we don't do anything if Active==0 */
			}
		}
		else  {  /*  in this case NumUp1 == 1 */
			if(NumUp2 == 0)  {
				if(Active==0) {
					NewNumBounds++;
					Active = 1;
				}
			}
			else {  /*  NumUp2 = 1 */
				if(Active==1)  {
					NewNumBounds++;
					Active = 0;
				}
				/*  if active == 0 we don't do anything */
			}
		}
		
	}
	
	/*  Now that we know how many boundaries are needed.  Allocate space */
	/*  and assign values to those boundaries */
	T3 = AllocIntervals(NewNumBounds);
	Active = 0; NumUp1 = 0; NumUp2 = 0; NewNumBounds = 0; i1 = 0; i2 = 0;
	for(i=0;i<N;i++)  {
		if( (T1->B[i1].v <= T2->B[i2].v  && (i1 < T1->NumBounds && i2 < T2->NumBounds) )  ||
				(i1 < T1->NumBounds && i2 >= T2->NumBounds) )  {
			NumUp1 += T1->B[i1].Q == IntOn ? 1 : -1;
			CurrentV = T1->B[i1].v;
			i1++;
		}
		else if( (T1->B[i1].v > T2->B[i2].v  && (i1 < T1->NumBounds && i2 < T2->NumBounds) )  ||
				(i1 >= T1->NumBounds && i2 < T2->NumBounds) ) {
			NumUp2 += T2->B[i2].Q == IntOn ? 1 : -1;
			CurrentV = T2->B[i2].v;
			i2++;
		}
		if(NumUp1 < 0 || NumUp1 > 1 || NumUp2 > 1 || NumUp2 < 0)  {
			printf("\n\nSorry.  NumUp1 = %d, NumUp2 = %d.  One of those is neither 0 nor 1. \n\nExiting to System...from IntervalsSubtract()\n\n",NumUp1,NumUp2);
			exit(1);
		}
		/*  and here we deal with the conditions that determine whether we are adding a boundary or not */
		/*  This depends on three variables that may be 0 or 1.  So, we have eight possibilities */
		if(NumUp1 == 0)  {
			if(NumUp2==0)  {
				if(Active==1)  {
					T3->B[NewNumBounds].Q = IntOff;
					T3->B[NewNumBounds].v = CurrentV;
					NewNumBounds++;
					Active = 0;
				}
				/*  if active==0 we don't do anything */
			}
			else  {  /*  NumUp2 == 1 */
				if(Active==1)  {
					T3->B[NewNumBounds].Q = IntOff;
					T3->B[NewNumBounds].v = CurrentV;
					NewNumBounds++;
					Active = 0;
					printf("\n\nJust saw NumUp1==0, NumUp2==1, Active==1.  Shouldn't happen!!!\n\n)");
				}
				/*  we don't do anything if Active==0 */
			}
		}
		else  {  /*  in this case NumUp1 == 1 */
			if(NumUp2 == 0)  {
				if(Active==0) {
					T3->B[NewNumBounds].Q = IntOn;
					T3->B[NewNumBounds].v = CurrentV;
					NewNumBounds++;
					Active = 1;
				}
			}
			else {  /*  NumUp2 = 1 */
				if(Active==1)  {
					T3->B[NewNumBounds].Q = IntOff;
					T3->B[NewNumBounds].v = CurrentV;
					NewNumBounds++;
					Active = 0;
				}
				/*  if active == 0 we don't do anything */
			}
		}
	}

		
	/*  that should have gotten all the information set in there.  Now we just need to  */
	/*  set the lo and the hi.  These should, by definition, be the values of the first  */
	/*  and the last boundaries */
	if(T3->NumBounds > 0) {
		T3->lo = T3->B[0].v;
		T3->hi = T3->B[T3->NumBounds-1].v;
	}
	
	/*  WE NOW MUST CONDENSE THOSE INTERVALS: */
	CondenseIntervals(&T3);
	
	/*  voila!  Now return the result */
	return(T3);
}


/******************************************************************************************/
/*                                                                                        */
/*		            FUNCTIONS FOR QUERIES ABOUT INTERVALS                                 */
/*                                                                                        */  
/******************************************************************************************/

/* 
	Given a sorted array of interval boundaries, A, this
	returns the index of the boundary that is to the left of the 
	point P.  Returns -1 if P is less than A[lo].v
	
	It should be called with Lo and Hi being the min and
	the max index of the array A.
	
	This is based on binary search.
	
	Only call this if P > A[0].  
*/
int RecursiveFindIntBoundary(double P, IntBoundaryStruct *A, int lo, int hi)
{	
	int mid = (lo + hi)/2;
	
	/*  return a -1 if B comes from an IntervalStruct with 0 boundaries.  In this case */
	/*  A will be NULL */
	if(A==NULL)
		return(-1);
	
	if(P<A[lo].v)
		return(-1);
	else if(P>A[hi].v)
		return(hi);
	if(lo>=hi)  {
		printf("\nlo >= hi in RecursiveFindIntBoundary().  Quitting!...\n\n)");
		exit(1);
	}
	if(hi == lo + 1)  /*  if we've whittled it down to a single interval */
		return(lo);
	
	else if(P<A[mid].v)  
		return(RecursiveFindIntBoundary(P,A,lo,mid));
	else if(P>A[mid].v)
		return(RecursiveFindIntBoundary(P,A,mid,hi));
	else if(P==A[mid].v)
		return(mid);
	
	/*  returns a -2 if it got here.  This is a problem!!! It should never get here!! */
	return(-2);
}

/*
	Returns a 1 if the point is contained in the active elements in an interval struct
	and a 0 otherwise
*/
int PointIsInActiveInterval(double P, IntervalsStruct *T)
{
	int temp;
	
	temp = RecursiveFindIntBoundary(P,T->B,0,T->NumBounds-1);
	
	if(temp < 0)
		return(0);
	else if(T->B[temp].Q == IntOn)
		return(1);
	else
		return(0);
		
}

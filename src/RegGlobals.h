/*************************************************************************
* This file is part of AgriPoliS
*
* AgriPoliS: An Agricultural Policy Simulator
*
* Copyright (c) 2021, Alfons Balmann, Kathrin Happe, Konrad Kellermann et al.
* (cf. AUTHORS.md) at Leibniz Institute of Agricultural Development in 
* Transition Economies
*
* SPDX-License-Identifier: MIT
**************************************************************************/

#ifndef RegGlobalsH
#define RegGlobalsH
#include <tchar.h>
#include "math.h"
#include "SimpleOpt.h"
#include <vector>
#define PLUSINF 1E30
#define MINEPS 0.0000000001

#include <string>

//static int rndcounter=0;
using namespace std;
class RegGlobalsInfo {
public:
//TEST DCX
    int tIter;
    int tFarmId;
    string tFarmName;
    int tInd;
//TEST

	//New for Soil service
    bool HAS_SOILSERVICE; 
	double TECH_DEVELOP;  
	double CARBON_MIN;
	double CARBON_MAX;
	double tech_develop_abs;
	vector<double> regCarbons;   //average for the region

    bool PRINT_SEC_RES;
    bool PRINT_SEC_PRICE;
    bool PRINT_SEC_EXP_PRICE;
    bool PRINT_SEC_COSTS;
    bool PRINT_SEC_COND;
    bool PRINT_FARM_RES;
    bool PRINT_FARM_INV;
    bool PRINT_FARM_PROD;
    bool PRINT_FARM_COSTS;
    bool PRINT_CONT_PLOTS;
    bool PRINT_TAC;
    bool PRINT_VA;
    bool PRINT_POLICY;

    bool INIT_OUTPUT;
    bool CALCULATE_EXPECTED_RENTAL_PRICE;
        int STATUS;
    double WERTS;
    double WERTS1;
    double WERTS2;
    virtual RegGlobalsInfo* create();
    virtual RegGlobalsInfo* clone();
    bool LP_CHANGED;
    bool INITIALISATION;
    virtual void backup();
    virtual void restore();
    bool FIXED_BONUS;
    double FIXED_BONUS_VALUE;
    bool VARIABLE_BONUS;
    double VARIABLE_BONUS_VALUE;
    bool PREV_OWNER_BONUS;
    bool LEGAL_TYPE_BONUS;
    bool ADJUST_PAID_RENT;
    bool SWEDEN;
    bool JOENKEPING;
    bool USE_TC_FRAMEWORK;
    bool ASSOCIATE_ACTIVITIES;
    bool USE_VARIABLE_PRICE_CHANGE;
    bool AGE_DEPENDENT;
    bool NO_SUCCESSOR_BY_RANDOM;
    bool ENV_MODELING;
    bool SOIL_TYPE_VARIATION;
    bool INTEGER_SWITCH;
    bool WEIGHTED_PLOT_SEARCH;
    double WEIGHTED_PLOT_SEARCH_VALUE;
    bool FIX_PRICES;
    bool MIN_WITHDRAWAL;
    bool USE_TRIANGULAR_DISTRIBUTED_MANAGEMENT_FACTOR;
    bool USE_TRIANGULAR_DISTRIBUTED_FARM_AGE;
    bool USE_HISTORICAL_CONTIGUOUS_PLOTS;
    bool RELEASE_PLOTS_BEFORE_EXPECTATION_FORMATION;
    vector<int> LEGAL_TYPES;
    bool CALC_LEGAL_TYPES;
    vector<string> NAMES_OF_LEGAL_TYPES;
    bool SET_FREE_PLOTS;
    vector<int> FREE_PLOTS_OF_TYPE;
    bool LP_MOD;

    string INPUTFILEdir; //dcx

    string INPUTFILE;
    string OUTPUTFILE;
    string POLICYFILE;
    int TC;
    int MT;
    int I;
    int P;
    int PC;
    int V;
    bool DEFAULT;
    int SEED;
    int SCENARIO;
    double QUOTA_PRICE;
    int GLOBAL_STRATEGY;
    bool GLOBAL_OPTIMUM_EVERY_PERIOD;
    bool FIRSTPRICE;
    bool SECONDPRICE_REGION;
    const void setNumberOfProducts(int n) {
        MAXPRODUCTS=n;
    }
    // GENERAL GLOBALS
    int number_of_farmtypes;
    vector<string> sheetnames;
    vector<int> number_of_each_type;
    vector<int> farm_class;
    vector<int> ef;
    int AVERAGE_OFFER_BUFFER_SIZE;
    double REGION_OVERSIZE;
    bool    CALCULATE_CONTIGUOUS_PLOTS;

    double REGION_NON_AG_LAND;
    double NON_AG_LANDINPUT;
    int NO_ROWS;
    int NO_COLS;
    int TRANSPORT_COSTS;
    int RUNS;
    int TEILER;
    vector<double> LAND_INPUT_OF_TYPE;
    int NO_OF_SOIL_TYPES;
    vector<string> NAMES_OF_SOIL_TYPES;
    bool OLD_LAND_RENTING_PROCESS;
    bool OLD_LAND_RELEASING_PROCESS;
    double RENT_ADJUST_COEFFICIENT;
    int RENT_ADJUST_COEFFICIENT_N;
    int TC_MACHINERY;
    double WITHDRAWFACTOR;
    int SIM_VERSION;


    //management coefficient
    double LOWER_BORDER;
    double UPPER_BORDER;
    // FARM GLOBALS
    double PLOT_SIZE;
    int GENERATION_CHANGE;
    // labour
    int MAX_H_LU;   // max hours per LU
    double OFF_FARM_LABOUR;
    // capital
    int    WD_FACTOR;      // variability factor for capital withdrawal
    double SHARE_SELF_FINANCE;
    double EQ_INTEREST;
    // not in dialog
    double INTEREST_RATE;
    double REGION_MILK_QUOTA;
    double OVERHEADS;


    // Display
    int APPEARANCE;
    int FARMOUTPUT;
    int SECTOROUTPUT;
    int VISION;
    // Constants
    int DESIGN_POINT;
    int RANDOM;
    int   MAXPRODUCTS;           // max. number of products produced on farm
    int NUMBER_OF_INVESTS;
    int INVEST_GROUPS;
    //Files
    RegGlobalsInfo();
    const void setAppearance(int a) {
        APPEARANCE=a;
    }
    const void setEqInterest(double e) {
        EQ_INTEREST=e;
    }
    const void setInterest(double e) {
        INTEREST_RATE=e;
    }
    const void setOffFarmLabour(double e) {
        OFF_FARM_LABOUR=e;
    }
    void initGlobals();
    virtual void readFromCommandLine();

    //DECOUPLING
    int MIN_CONTRACT_LENGTH;
    int MAX_CONTRACT_LENGTH;

    double LB_LOW_TRANCH;
    double UB_LOW_TRANCH;
    double LB_MIDDLE_TRANCH;
    double UB_MIDDLE_TRANCH;
    double LB_HIGH_TRANCH;
    double UB_HIGH_TRANCH;
    double DEG_LOW_TRANCH;
    double DEG_MIDDLE_TRANCH;
    double DEG_HIGH_TRANCH;

    double TRANCH_1_WIDTH;
    double TRANCH_2_WIDTH;
    double TRANCH_3_WIDTH;
    double TRANCH_4_WIDTH;
    double TRANCH_5_WIDTH;
    double TRANCH_1_DEG;
    double TRANCH_2_DEG;
    double TRANCH_3_DEG;
    double TRANCH_4_DEG;
    double TRANCH_5_DEG;
    int FIX_REFERENCE_PERIOD;

    int REGIONAL_DECOUPLING;
    int FULLY_DECOUPLING;
    int FARMSPECIFIC_DECOUPLING;
    int REGIONAL_DECOUPLING_SWITCH;
    int FULLY_DECOUPLING_SWITCH;
    int FARMSPECIFIC_DECOUPLING_SWITCH;
    string OUTPUT_FOLDER;

    int PLOTSN;
    int NUMBER_OF_INVESTTYPES;
    double PIGLETS_PER_SOW;
    double PIGS_PER_PLACE;
    double MILKPROD;
    double ESU;
    double INCREASEPRICE;
    double BONUS;
    double REFINCOME;
    int PRODUCTGROUPS;
    int PRODGROUPLAB;
    int PRODTYPE;
    double CHANGEPERHA;
    double CHANGEUP    ;
    double CHANGEDOWN   ;
    double MILKUPPERLIMIT;
    double MILKLOWERLIMIT ;

    int OFFFARMLABTYPE;
    int VAROFFARMLABTYPE;
    int VARHIREDLABTYPE  ;
    int ST_BOR_INTERESTTYPE;
    int ST_EC_INTERESTTYPE;

    string premiumName;

    string  sFATTENINGPIGS;
    string  sSERVICES;
    string  sSOW;
    string  sMILK;
    string  sMANSELL;
    string  sMANBUY;
    string  sGETQUOTA;
    string  sLETQUOTA;
    string  sPREMIUM;
    string  sTOTAL_PREMIUM;

//DCX
	string sBUYCALF_SUCK;
	string sSELLCALF_SUCK;
	string sBUYCALF_DAIRY;
	string sSELLCALF_DAIRY;
//


    string  sCOUPLED_PREM_UNMOD;
    string  sDECOUPLED_PREM_UNMOD;
    string  sTOTAL_PREM_MODULATED;
    string  sTRANCH_1;
    string  sTRANCH_2;
    string  sTRANCH_3;
    string  sTRANCH_4;
    string  sTRANCH_5;
    string  sIDLE_ARABLE;
    string  sIDLE_ARABLE2;
    string  sIDLE_GRASS;
    string  sEXCESS_LU;
    string  sLU_UPPER_LIMIT;
    string  sFIXED_OFFFARM_LAB;
    string  sFIXED_HIRED_LAB;


    int   FATTENINGPIGS;
    int   SERVICES;
    int   SOW;
    int   MILK;
    int   MANSELL;
    int   MANBUY;
    int   GETQUOTA;
    int   LETQUOTA;
    int   PREMIUM;
    int   TOTAL_PREMIUM;

//DCX
	int BUYCALF_SUCK;
	int SELLCALF_SUCK;
	int BUYCALF_DAIRY;
	int SELLCALF_DAIRY;
//

    int   COUPLED_PREM_UNMOD;
    int   DECOUPLED_PREM_UNMOD;
    int   TOTAL_PREM_MODULATED;
    int   TRANCH_1;
    int   TRANCH_2;
    int   TRANCH_3;
    int   TRANCH_4;
    int   TRANCH_5;
    int   IDLE_ARABLE;
    int   IDLE_ARABLE2;
    int   IDLE_GRASS;
    int   EXCESS_LU;
    int   LU_UPPER_LIMIT;
    int   FIXED_OFFFARM_LAB;
    int   FIXED_HIRED_LAB;


    bool FAST_PLOT_SEARCH;
    bool PRINT_REMOVED_FARMS;
    void setNumberOfInvestType(int t) {
        NUMBER_OF_INVESTTYPES=t;
    };
    void setEnvModeling(bool n) {
        ENV_MODELING=n;
    };
    virtual ~RegGlobalsInfo();
    int ARGC;
    char ** ARGV;
protected:
    RegGlobalsInfo* obj_backup;

};



double triangular(double min, double ml, double max);
double capitalReturnFactor(double,int);
double averageReturn(double,int);
/*template<class A,class B>
void swap(A a,B b,int l,int r);
template<class A,class B>
int partition( A a,B b, int low, int high );
template<class A,class B>
void quicksort(A a,B b, int low, int high );
//extern int randown();*/
template<class A,class B>
void swap(A* a,B* b,int l,int r) {
    A tmp1=a[l];
    B tmp2=b[l];
    a[l]=a[r];
    b[l]=b[r];
    a[r]=tmp1;
    b[r]=tmp2;
}
template<class A,class B>
int partition( A* a,B* b, int low, int high ) {
    int left, right;
    A pivot_item;
    B pivot_item2;
    pivot_item = a[low];
    pivot_item2 = b[low];
    left = low;
    right = high;
    while ( left < right ) {
        /* Move left while item < pivot */
        while ( a[left] <= pivot_item && left<high) left++;
        /* Move right while item > pivot */
        while ( a[right] > pivot_item && right>low) right--;
        if ( left < right ) swap(a,b,left,right);
    }
    /* right is final position for the pivot */
    a[low] = a[right];
    a[right] = pivot_item;
    b[low] = b[right];
    b[right] = pivot_item2;
    return right;
}
template<class A,class B>
void quicksort(A* a,B* b, int low, int high ) {
    int pivot;
    /* Termination condition! */
    if ( high > low )   {
        pivot = partition( a, b ,low, high );
        quicksort( a, b, low, pivot-1 );
        quicksort( a, b, pivot+1, high );
    }
}

template<class A>
void swap(A* a,int l,int r) {
    A tmp1=a[l];
    a[l]=a[r];
    a[r]=tmp1;
}
template<class A>
int partition( A* a, int low, int high ) {
    int left, right;
    A pivot_item;
    pivot_item = a[low];
    left = low;
    right = high;
    while ( left < right ) {
        /* Move left while item < pivot */
        while ( a[left] <= pivot_item && left<high) left++;
        /* Move right while item > pivot */
        while ( a[right] > pivot_item && right>low) right--;
        if ( left < right ) swap(a,left,right);
    }
    /* right is final position for the pivot */
    a[low] = a[right];
    a[right] = pivot_item;
    return right;
}
template<class A>
void quicksort(A* a, int low, int high ) {
    int pivot;
    /* Termination condition! */
    if ( high > low )   {
        pivot = partition( a, low, high );
        quicksort( a, low, pivot-1 );
        quicksort( a, pivot+1, high );
    }
}


#endif

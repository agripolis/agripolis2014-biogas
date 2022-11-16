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

// RegManager.cpp
#include <fstream>
#include <sstream>
#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <algorithm>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "RegManager.h"
#include "RegLpD.h"
#include "RegStructure.h"
#include "RegFarm.h"
#include "RegPlot.h"

#include <iterator>

#include "textinput.h"
#include "random.h"


using namespace std;

long int mtRandMin = 0 ;
long int mtRandMax= 0x7FFFFFFFUL ;

 struct farmsdata farmsdata  ;
 map <string, farminvestdata > farmsIinvest;

 struct globdata globdata;
 struct transdata transdata  ;
 struct matrixdata matrixdata  ;

 struct objlinkdata objlinkdata   ;
 struct caplinkdata caplinkdata   ;
 struct matlinkdata   matlinkdata   ;

 struct envmarketdata  envmarketdata    ;
 struct investdata investdata     ;
 struct marketdata marketdata    ;
 struct envdata  envdata       ;

 vector <oneyield> yielddata;

//soil service 
void RegManagerInfo::UpdateSoilserviceP(){
	if (g->HAS_SOILSERVICE) {
		list<RegFarmInfo* >::iterator farms_iter;
		for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end(); farms_iter++) {
 				(*farms_iter)->calDeltaCarbons();
				(*farms_iter)->updateCarbons();
		}
	}
	return;
}

void RegManagerInfo::UpdateSoilserviceLA(){
	if (g->HAS_SOILSERVICE) {
		list<RegFarmInfo* >::iterator farms_iter;
		for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end(); farms_iter++) {
				for (int i=0; i<g->NO_OF_SOIL_TYPES; ++i){
					if ((*farms_iter)->getNewRentedLandOfType(i) !=0)  {
						(*farms_iter)->calAvCarbons();
						break;
					}
				}
		}
	}
	return;
}



RegManagerInfo::RegManagerInfo(RegGlobalsInfo* G)
        : g(G) {
    obj_backup=NULL;
    flat_copy= false;
    name="0";

    // thread priority
    iteration = 0;
    evaluator= new Evaluator("policy_switching");
    randInit(g->SEED);
    //srand(g->SEED);
//  srand(0) for keeping random numbers unchanged, srand(g->SEED) if not;   50

}
RegManagerInfo*
RegManagerInfo::clone(string name) {
    RegManagerInfo *n=create();
    (*n).name=name;
    (*n).iteration=iteration;
    (*n).g=g->clone();
    (*n).Sector=new RegSectorResultsInfo(*Sector,(*n).g);
    if (g->CALC_LEGAL_TYPES) {
        for (unsigned int i=0;i<g->LEGAL_TYPES.size();i++) {
            (*n).sector_type.push_back(new RegSectorResultsInfo(*(sector_type[i]),(*n).g));
        }
    }

    (*n).Env=new RegEnvInfo(*Env,(*n).g);
    (*n).Region=new RegRegionInfo(*Region,(*n).g);
	(*n).Data=new RegDataInfo((*n).g, (*n).Market);
    (*n).Market=new RegMarketInfo(*Market,(*n).g);
    (*n).Mip= Mip->clone((*n).g);
    (*n).Policyoutput=new OutputControl(*Policyoutput,(*n).g);
    (*n).evaluator=new Evaluator(*evaluator);
    for (unsigned int i=0;i<InvestCatalog.size();i++) {
        (*n).InvestCatalog.push_back(InvestCatalog[i]);
    }
    (*n).FarmList.clear();
    (*n).RemovedFarmList.clear();
    list<RegFarmInfo* >::const_iterator farms;
    for (farms = FarmList.begin();
            farms != FarmList.end();
            farms++) {
        RegFarmInfo* tmp=(*farms)->clone((*n).g,(*n).Region,(*n).Market->getProductCat(),(*n).InvestCatalog);
        (*n).FarmList.push_back(tmp);
    }
    for (farms = RemovedFarmList.begin();
            farms != RemovedFarmList.end();
            farms++) {
        RegFarmInfo* tmp=(*farms)->clone((*n).g,(*n).Region,(*n).Market->getProductCat(),(*n).InvestCatalog);
        (*n).RemovedFarmList.push_back(tmp);
    }
    return n;
}

RegManagerInfo::~RegManagerInfo() {
    list<RegFarmInfo* >::iterator farms;
    if (!flat_copy) {
        for (farms = FarmList.begin();
                farms != FarmList.end();
                farms++) {
            delete (*farms);
            (*farms) = NULL;
        }

        for (farms = RemovedFarmList.begin();
                farms != RemovedFarmList.end();
                farms++) {
            delete (*farms);
            (*farms) = NULL;
        }
        delete Env;
        delete Market;
        delete Region;
        delete Sector;
        delete Data;
        delete Policyoutput;
        delete evaluator;
        delete Mip;
    }
    if (g->CALC_LEGAL_TYPES) {
        for (unsigned int i=0;i<g->LEGAL_TYPES.size();i++) {
            delete sector_type[i];
        }
    }
    RemovedFarmList.clear();
    FarmList.clear();
    if (obj_backup) delete obj_backup;

}
//---------------------------------------------------------------------------
//
//      INITIALISATION OF SIMULATION
//
//---------------------------------------------------------------------------

//-----------------------------------
// INITIALISE INVESTMENTS AND CATALOG
//-----------------------------------

void RegManagerInfo::init() {
    // Merke fuer ewige Zeiten: Wenn man in Thread dll's laden will
    // muss CoInitialzie aufgerufen werden.
//    CoInitialize(0);
     f=t=n=0;
//    g->readFromCommandLine();

    Policyoutput = new OutputControl(g);
//    printf("dir: %s\n", g->INPUTFILEdir.c_str());
//    flush(cout);

	readfiles(g->INPUTFILEdir.c_str(), g->HAS_SOILSERVICE);

    randInit(g->SEED);
    //srand(g->SEED);
//srand(0) for keeping random number unchanged, srand(g->SEED) if not;
    int r;
    r=randlong();

    // create market and pass globals
    Market = new RegMarketInfo(g);

	// create output files and pass globals
    Data = new RegDataInfo(g, Market);
    
	initGlobals();

    Sector = new RegSectorResultsInfo(g,0);
    if (g->CALC_LEGAL_TYPES) {
        for (unsigned int i=0;i<g->LEGAL_TYPES.size();i++) {
            sector_type.push_back(new RegSectorResultsInfo(g,g->LEGAL_TYPES[i]));
        }
    }

    // create Region an pass graphics handle
    //g->SEED);
    r=randlong();

    Region = new RegRegionInfo(g);
    initRegion();
    r=randlong();
    Env=new RegEnvInfo(g);
    initEnv();
    initMarket();
    initEnv2();
    initGlobals2();

    r=randlong();
    // investmen objects create themselves and are stored in InvestCatalog vector
    initInvestmentCatalog();
    initMatrix();

    setPremiumColRow();

    initPopulations();
    r=randlong();
    initOutput();

//    CoUninitialize();
	readPolicyChanges0();
    setLpChangesFromPoliySettings();

    cout << "Initialized " << endl;
}

void
RegManagerInfo::initGlobals() {
    g->initGlobals();
}

void
RegManagerInfo::initGlobals2() {
  // nummber für globals nachhole
    map <string,int> prodId;
    map <string, int> invId;
    for (unsigned int i=0; i< marketdata.products.size(); i++)
        prodId[marketdata.products[i].name]=i;
    for (unsigned int i=0; i< investdata.invests.size(); i++)
        invId[investdata.invests[i].name]=i;

#define gl(a) s##a
#define TT(a) {\
	if (g->gl(a).compare("_NO_")==0) g->a = 1000; \
	else g->a = prodId[g->gl(a)]; }

	TT(FATTENINGPIGS)
    TT(SERVICES);
    TT(SOW);
    TT(MILK);
    TT(MANSELL);
    TT(MANBUY);
    TT(GETQUOTA);
    TT(LETQUOTA);

//new
	TT(BUYCALF_SUCK);
	TT(SELLCALF_SUCK);
	TT(BUYCALF_DAIRY);
	TT(SELLCALF_DAIRY);

  if(g->LP_MOD) {
    TT(COUPLED_PREM_UNMOD);
    TT(DECOUPLED_PREM_UNMOD);
    TT(TOTAL_PREM_MODULATED);

    TT(TRANCH_1);
    TT(TRANCH_2);
    TT(TRANCH_3);
    TT(TRANCH_4);
    TT(TRANCH_5);

  } else {
    TT(PREMIUM);
    TT(TOTAL_PREMIUM);
  }
    TT(IDLE_ARABLE);
    TT(IDLE_ARABLE2);
    TT(IDLE_GRASS);

    g->FIXED_OFFFARM_LAB =  invId[g->sFIXED_OFFFARM_LAB];
    g->FIXED_HIRED_LAB =  invId[g->sFIXED_HIRED_LAB];

#undef gl
#undef TT
}

void
RegManagerInfo::initEnv() {
    if (g->ENV_MODELING)
        Env->initEnv();
}

void RegManagerInfo::initEnv2(){
 // nummer  für environmentals nachhole
  if (g->ENV_MODELING) {
     vector<string> sacts;
     int sz = Env->sassociated_activities.size();
     for (int i = 0; i < sz; i++){
        sacts =  Env->sassociated_activities[i];
        int sz2 = sacts.size();
        vector<int> acts;

        for (int j = 0; j<sz2; j++){
            acts.push_back(Market->product_id[sacts[j]]);
        }
        Env->associated_activities.push_back(acts);
     }
  }
}

// initialise investment catalog
void
RegManagerInfo::initInvestmentCatalog() {
    RegInvestObjectInfo readInvest(g);
    // init investment catalog
    // read in values and store in list
    InvestCatalog=readInvest.initInvestmentCatalog();
    g->NUMBER_OF_INVESTS = InvestCatalog.size();
    for (int i=0;i<g->NUMBER_OF_INVESTS;i++) {
        if (InvestCatalog[i].getInvestType()==g->OFFFARMLABTYPE) {
            g->setOffFarmLabour(InvestCatalog[i].getAcquisitionCosts()*
                                ((double)g->MAX_H_LU/InvestCatalog[i].getLabourSubstitution()));
        }
        InvestCatalog[i].setBcInterest(g->INTEREST_RATE);
        InvestCatalog[i].setInterestReduction(0);
    }
    // update values of investments which depend on interest rate
    updateInterestDependentValues();
}
// initialise MIP matrix
void
RegManagerInfo::initMatrix() {
    Mip=createMatrix();
    Mip->setupMatrix(g);
}
RegLpInfo*
RegManagerInfo::createMatrix() {
    return new RegLpInfo();
}
// initialise market
void
RegManagerInfo::initMarket() {
    Market->createMarket(Env);
}


void
RegManagerInfo::setPremiumColRow() {
    //set premium row and col
    int row;
    map <string, int> colindex;

    string premName=g->premiumName;
    vector <string> rownames= matrixdata.rownames;
    vector <string> colnames= matrixdata.colnames;

    for (unsigned int j=0; j<colnames.size();j++)
        colindex[colnames[j]]=j;

    int sz= rownames.size();
    int i;
    for (i=0; i<sz; i++)
        if (rownames[i]==premName) break;

    if (i >= sz ) {
        cout << "PREMIUM ROW NOT FOUND" << endl;
        exit(2);
    }
    row= i;

    vector <RegProductInfo> prodcat;
    prodcat = Market->getProductCat();

    int psz= prodcat.size();
    for (i = 0 ; i< psz; i++) {
        RegProductInfo* p= &(Market->getProductCat()[i]);
        if ((*p).getPremiumLegitimation()){
            (*p).setPremiumCol(colindex[prodcat[i].getName()]);
            (*p).setPremiumRow(row);
        }
    }
}

// INITIALISE REGION AND PLOTS
void
RegManagerInfo::initRegion() {
    Region->initialisation();
}
// INITIALISE DATA OUTPUT
void
RegManagerInfo::initOutput() {
    if(g->INIT_OUTPUT) {
        Data->initialisation(InvestCatalog,Market->getProductCat(),Env);
        if (g->ENV_MODELING)
           Env->initEnvOutput(Market);
		//soil service output
		if (g->HAS_SOILSERVICE)
			Data->initSoilservice(FarmList);
    }

}

// INITIALISE FARM POPULATION(S)
void
RegManagerInfo::initPopulations() {
     // READ INVESTMENT DATA FOR EACH FARM

    vector<double> initial_land_input = farmsdata.land_inputs;
    vector<vector<double> > owned_land_input;
    vector<vector<double> > rented_land_input;
    vector<vector<double> > initial_rent;
	
	owned_land_input.resize(g->NO_OF_SOIL_TYPES);
    rented_land_input.resize(g->NO_OF_SOIL_TYPES);
    initial_rent.resize(g->NO_OF_SOIL_TYPES);

    for (int j=0; j< g->NO_OF_SOIL_TYPES; j++) {
        vector<double> oli =   farmsdata.alllands[g->NAMES_OF_SOIL_TYPES[j]].owned_land;
        vector<double> rli =   farmsdata.alllands[g->NAMES_OF_SOIL_TYPES[j]].rented_land;
        vector<double> ir =    farmsdata.alllands[g->NAMES_OF_SOIL_TYPES[j]].initial_rent_price;

        owned_land_input[j]=  oli;
        rented_land_input[j]= rli;
        initial_rent[j]= ir;
    }
   
    vector<double>  milk_quota = farmsdata.milk_quotas;
    vector<double>  family_labour = farmsdata.fam_lab_units;
    vector<double>  off_farm_labour = farmsdata.off_fam_labs;
    vector<double>  equity_capital = farmsdata.equity_capitals;
    vector<double>  land_assets = farmsdata.land_assets;
    vector<double>  rel_invest_age = farmsdata.rel_invest_ages;

    //Vektoren fuer die Informationen uber die Investitionsbestaende
    vector< vector<int> > cap_vec;
    vector< vector<int> > noi_vec;
    vector< vector<int> > cat_num_vec;
    cap_vec.resize(g->number_of_farmtypes);
    noi_vec.resize(g->number_of_farmtypes);
    cat_num_vec.resize(g->number_of_farmtypes);
    for (int i=0;i<g->number_of_farmtypes;i++) {
        cap_vec[i].resize(g->NUMBER_OF_INVESTS);
        cat_num_vec[i].resize(g->NUMBER_OF_INVESTS);
        noi_vec[i].resize(g->NUMBER_OF_INVESTS);
    }

    //initial investments
    for (int i = 0;i < g->number_of_farmtypes; i++) {
        int index=0;
        unsigned dind=0;
        for (int j=0;j<g->NUMBER_OF_INVESTS;j++) {
            if (dind < farmsIinvest[g->sheetnames[i]].initinvests.size() ) {
                string named =  farmsIinvest[g->sheetnames[i]].initinvests[dind].name;
                string namej= InvestCatalog[j].getName();

                if ( named == namej ) {
                    cap_vec[i][index] = farmsIinvest[g->sheetnames[i]].initinvests[dind].capacity;
                    noi_vec[i][index] =  farmsIinvest[g->sheetnames[i]].initinvests[dind].quant;
                    cat_num_vec[i][index]= j;
                    dind++;
                    index++;
                    continue;
                }
            }
            cap_vec[i][index] = 0;
            noi_vec[i][index] =  0;
            cat_num_vec[i][index]= j;

           index++;
        }
    }

    // create pointer to farm
    RegFarmInfo* newFarm;
    int newnumber = 0;
    int new_farm_id;
    int farmclass;
    int farmef;
    string farmname;

    for (int i = 0;i < g->number_of_farmtypes; i++) {
        farmclass = g->farm_class[i];
        farmname  = g->sheetnames[i];
        farmef    = g->ef[i];
        for (int j = 0;j<g->number_of_each_type[i];j++) {
            new_farm_id = 10000 * iteration + newnumber;
//DCX
g->tFarmId = new_farm_id;

            // create farm and initialise
            newFarm = createFarm(Region,
                                 g,
                                 Market->getProductCat(),
                                 InvestCatalog, // list pass by reference
                                 Mip,
                                 i,
                                 new_farm_id,
                                 farmclass,
                                 farmname,
                                 farmef);
            // set values read from file

                newFarm->setInitialLand(initial_land_input[i]);
            for (int k=0;k<g->NO_OF_SOIL_TYPES;k++) {
                newFarm->setInitialOwnedLandOfType(owned_land_input[k][i],k);
                newFarm->setInitialRentedLandOfType(rented_land_input[k][i],k);
                newFarm->setInitialRentOfType(initial_rent[k][i],k);
            }

			//DCX
			newFarm->setFarmStead();

            newFarm->setInitialFamLu(family_labour[i]+off_farm_labour[i]);
            newFarm->setMilkQuota(milk_quota[i]);
            newFarm->setEquityCapital(equity_capital[i]);
            newFarm->setLandAssets(land_assets[i]);
            newFarm->setRelInvestAge(rel_invest_age[i]);

            for (int k=0;k<g->NUMBER_OF_INVESTS;k++) {
                newFarm->addInvestments(cat_num_vec[i][k],noi_vec[i][k],cap_vec[i][k]);
            }
            // set asynchronous age of farms and of their assets
            newFarm->setAsynchronousFarmAge();
            // store farms in Farm list
            FarmList.push_back(newFarm);
            newnumber++;
        }
    }

    /////////////////////////////
    // ALLOCATION OF INITIAL LAND
    /////////////////////////////
    Region->initPlotSearch();
    list<RegFarmInfo* >::iterator farms_iter;
    bool ready=false;

//srand(g->SEED) for randoming soil types, all other things being equal;
//here srand(g->SEED) after the while loop to get the same random ST at each simulation;
//if srand(g->SEED) before the loop, you get always different random ST distribution for each simulation;

    if (g->SOIL_TYPE_VARIATION) {
        randInit(g->SEED);
        //srand(g->SEED);
        }
    while (!ready) {
        ready=true;
        for (farms_iter = FarmList.begin();
                farms_iter != FarmList.end();
                farms_iter++) {
            if (!(*farms_iter)->allokateInitialLand())
                ready = false;
        }
    }

    // finish up setup of region (create black plots)
    Region->finish();
    Region->initPlotSearch();
    g->INITIALISATION=false;
 //    for (farms_iter = FarmList.begin();
 //               farms_iter != FarmList.end();
 //               farms_iter++) {
 //               (*farms_iter)->releaseRentedPlots()   ;
 //       }

	//init carbon
	if (g->HAS_SOILSERVICE) {
	   for (farms_iter = FarmList.begin();
               farms_iter != FarmList.end();
               farms_iter++) {
			vector <double> c_mean;
			vector <double> c_var;
			for (int i=0;i<g->NO_OF_SOIL_TYPES;++i) {
				c_mean.push_back(farmsdata.alllands[g->NAMES_OF_SOIL_TYPES[i]].carbon_mean[(*farms_iter)->getFarmType()]);
				c_var.push_back(farmsdata.alllands[g->NAMES_OF_SOIL_TYPES[i]].carbon_std_dev[(*farms_iter)->getFarmType()]);
			}
			(*farms_iter)->initCarbons(c_mean, c_var);
       }
	}
}


//---------------------------------------------------------------------------
//
//      EVOLVE SIMULATION
//
//---------------------------------------------------------------------------

//-------------------
// EVOLVE SIMUALATION                                                                                               in
//-------------------

void
RegManagerInfo::simulate() {
    init();

    // as long as thread is running and the max. number of iterations
    // is not reached
    while (iteration < g->RUNS) {
		cout << "Iteration : " << iteration << "\t ( Number of Farms:  " << getNoOfFarms()<< " )"<< endl;

		//TEST
		g->tIter = iteration;
        step();
    }
}
void
RegManagerInfo::stepwhile() {
    // as long as thread is running and the max. number of iterations
    // is not reached
    while (iteration < g->RUNS) {
        step();
    }
}
void RegManagerInfo::step() {
	if(g->GLOBAL_OPTIMUM_EVERY_PERIOD) {
        RegManagerInfo* tmp1=this->clone("tmp1");
        RegManagerInfo* tmp2=this->clone("tmp2");
        RegGlobalsInfo* tmpg1=tmp1->getGlobals();
        RegGlobalsInfo* tmpg2=tmp2->getGlobals();
        tmpg1->GLOBAL_OPTIMUM_EVERY_PERIOD=false;
        tmpg1->GLOBAL_STRATEGY=1;
        tmpg2->GLOBAL_OPTIMUM_EVERY_PERIOD=false;
        tmpg2->GLOBAL_STRATEGY=2;
        tmp1->step();
        tmp2->step();
    }
    PreparationForPeriod();

    // adjust costs by management coefficient
    if (iteration == 0)
        CostAdjustment();
    readPolicyChanges();
    Region->calculateAverageRent();
    Region->calculateAverageNewRent();
    if (g->CALCULATE_CONTIGUOUS_PLOTS) Region->countContiguousPlots();
    g->WERTS1=RentStatistics();
    g->WERTS=-g->WERTS1;
    LandAllocation();
	UpdateSoilserviceLA();
    Region->calculateAverageRent();
    Region->calculateAverageNewRent();
    if(iteration==0)
        Region->setNewRentFirstPeriod();
    f=static_cast<int>(Region->getExpAvNewRentOfType(1)/Region->getAvRentOfType(1));
    g->WERTS2=RentStatistics();
    g->WERTS+=g->WERTS2;
    InvestmentDecision();
    Production();
	UpdateSoilserviceP();  
    UpdateMarket();
    FarmPeriodResults();
    RemovedFarmPeriodResults();
    SectorResults();
    FarmOutput();
    CapacityEstimationForBidding();
    Region->calculateAverageRent();
    Region->calculateAverageNewRent();
    setPolicyChanges();
    Disinvest();
	FutureOfFarms();
	SectorResultsAfterDisinvest();
    ResetPeriodLabour();
    SectorOutput();
    EnvSpeciesCalc();
    RemoveFarms();
    ProcessMessages();
}

void
RegManagerInfo::CapacityEstimationForBidding() {
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        (*farms_iter)->calculateEstimationForBidding();
    }


}

void
RegManagerInfo::ResetPeriodLabour() // actually part of FutureOfFarms
{
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        (*farms_iter)->resetFarmVariables();
    }
}
void
RegManagerInfo::RegionalPerHaPayment() {
    double total_direct_payment = 0;
    double total_ha = 0;
    double average_ha_payment = 0;
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        total_direct_payment += (*farms_iter)->getAveragePremium();
        total_ha += (*farms_iter)->getLandInput();
    }
    if (total_ha != 0)
        average_ha_payment = total_direct_payment/total_ha;
    else
        average_ha_payment = 0;
    Region->setHaPaymentPerPlot(average_ha_payment*g->PLOT_SIZE);
}

void
RegManagerInfo::PreparationForPeriod() {
	if (iteration>0) 
		g->tech_develop_abs *= 1+ g->TECH_DEVELOP;
    if(g->SET_FREE_PLOTS) {
      Region->setIdlePlotsDead();
      for(int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
        Region->setDeadPlotsToType(g->FREE_PLOTS_OF_TYPE[i],i);
      }
    }
    list<RegFarmInfo* >::iterator farms_iter;
    // reset sector output values for new iteration
    Sector->resetSector();
    if (g->CALC_LEGAL_TYPES) {
        for (unsigned int i=0;i<g->LEGAL_TYPES.size();i++) {
            sector_type[i]->resetSector();
        }
    }
    // if (g->WEIGHTED_PLOT_SEARCH)
    // Region->setUpdate();
    // compute new MIP values
    updateInterestDependentValues();

    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
		//cout<< (*farms_iter)->getFarmName() << " " << (*farms_iter)->farm_plot->getSoilName() << "\n"; 
        (*farms_iter)->newRentingProcess(iteration);
        (*farms_iter)->updateLpValues();
    }
    //  if (g->WEIGHTED_PLOT_SEARCH)
    //  Region->resetUpdate();

	//soil service
	if (g->HAS_SOILSERVICE) {
		for (int i=0; i<g->NO_OF_SOIL_TYPES; ++i) {
			double sum=0;
			double area=0;
			for (farms_iter=FarmList.begin(); farms_iter!=FarmList.end(); farms_iter++) {
				double a=(*farms_iter)->getLandInputOfType(i);
				sum+=(*farms_iter)->getAvCarbons()[i]*a;
				area+=a;
			}
			if (area!=0) g->regCarbons[i]=sum/area;
			else g->regCarbons[i]=0; //??
		}
	}
} 

void
RegManagerInfo::CostAdjustment() {
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        // multiply individual costs on farm by management factor
        (*farms_iter)->updateCosts();
        (*farms_iter)->updateLpValues();
    }
}
void
RegManagerInfo::LandAllocation() {
//        if (iteration==0) {
//          printShadowPrices(100);   }
    if (g->GLOBAL_STRATEGY==1) {
        if (iteration>0) {
            if (FarmList.size()>0) {
                 string part2=g->OUTPUTFILE+ string("103.dat");
//                (*(FarmList.begin()))->lp->globalAllocationFromFile(FarmList,Region,part2);
                (*(FarmList.begin()))->lp->globalAllocation(FarmList,Region,iteration);
            }
        }
    }
    if(g->GLOBAL_STRATEGY==0) {
        if (g->USE_TC_FRAMEWORK) {
            released_plots=released_plots_IF=released_plots_CF=rented_plots_CF=rented_plots_IF=0;
            ;
            CF_to_CF=0;
            CF_to_IF=0;
            IF_to_CF=0;

            stay_CF=0;
            stay_IF=0;


            paid_tacs=0;
            total_tacs=0;
            stay_at_prev_owner=0;
            stay_at_prev_owner_because_of_tacs=0;

        }
//    ofstream pa;
//    pa.open("pacht.txt",ios::app);
        int bc=0;
        if (g->OLD_LAND_RENTING_PROCESS) {
            vector<int> count_rented_plots_of_type;
//DCX
            vector<double> max_offer_of_type;
//            vector<int> max_offer_of_type;
            vector<double> av_offer_of_type;
            vector<double> ratio_of_type;
            vector<double> secondprice_region;
            for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                count_rented_plots_of_type.push_back(0);
                max_offer_of_type.push_back(0);
                av_offer_of_type.push_back(0);
                ratio_of_type.push_back(0);
                secondprice_region.push_back(1E30);
            }
            bool stop;
            if (iteration > 0) {
                stop= true;
                bidcount=0;
                do { // as long as offers are positive
                    stop=true;
                    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                        if (Region->getFreeLandPlotsOfType(i)>0) {
                            max_offer_of_type[i]=rentOnePlot(count_rented_plots_of_type,i);
                            if(max_offer_of_type[i]<secondprice_region[i])
                              secondprice_region[i]=max_offer_of_type[i];
//                        pa << bc << "\t" << max_offer_of_type[i] << "\n";
                            bc++;
                        } else {
                            max_offer_of_type[i]=0;
                        }
                        av_offer_of_type[i]+= max_offer_of_type[i];
                    }
                    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                        if (max_offer_of_type[i]>0) stop = false;
                    }
                    bidcount++;
                } while (!stop);
//            pa.close();
                for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                    if (count_rented_plots_of_type[i]>0) {
                        av_offer_of_type[i]/=(double)count_rented_plots_of_type[i];
                        ratio_of_type[i]=(double)count_rented_plots_of_type[i]/(double)Region->getNumberOfLandPlotsOfType(i);
                    }
                }
                if(g->SECONDPRICE_REGION) {
                  list<RegFarmInfo* >::iterator farms_iter;
                        for (farms_iter = FarmList.begin();
                                farms_iter != FarmList.end();
                                farms_iter++) {
                                (*farms_iter)->setSecondPrice(secondprice_region);
                        }
                }

                //  ofstream out;
                //  out.open("neupachten.dat",ios::app);
                //  out << iteration << "\t" << av_offer_of_type[0] << "\t" << av_offer_of_type[1]<< "\n";
                //  out.close();
                ///////////////////
                // ADJUST RENT PAID
                ///////////////////
                // Here, the rent of already rented plots is adjusted according
                // to the ratio of newly rented plots of type to the total
                // number of plots of type
                // The same here. If there is no averageOffer ists not possible to update
                // the rents.
                if (g->ADJUST_PAID_RENT) {
                    stop=false;
                    for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                        if (count_rented_plots_of_type[i]>0) stop = false;
                    }
                    list<RegFarmInfo* >::iterator farms_iter;
                    if (!stop) {
                        for (farms_iter = FarmList.begin();
                                farms_iter != FarmList.end();
                                farms_iter++) {
                            if (iteration > 3)
                            (*farms_iter)->adjustPaidRent(av_offer_of_type,
                                                          ratio_of_type);
                        }
                    }
                }
            }
        } else {
            ////////////////////
            //  LAND ALLOCATION
            ////////////////////
            list<RegFarmInfo* >::iterator farms_iter;
            double sum=0;
            double sum_dist=0;
            Region->calculateAverageRent();
            Region->calculateAverageNewRent();
            for (farms_iter = FarmList.begin();
                    farms_iter != FarmList.end();
                    farms_iter++) {
                sum+=(*farms_iter)->getObjective();
                sum_dist+=(*farms_iter)->getFarmDistanceCosts();
            }
            sum-=sum_dist;

            vector<int> count_rented_plots_of_type;
            for (int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
                count_rented_plots_of_type.push_back(0);
            }
            if (iteration > 0) {
                int pl= Region->getNumberOfFreePlots()*2;
                for (int i=0;i<pl;i++) {
                    rentOnePlot(count_rented_plots_of_type,-1);
                }
            } // end if(iteration > 0)

        }
        if ((g->USE_TC_FRAMEWORK && g->PRINT_TAC)) {
            string o=g->OUTPUTFILE+"tac.dat";
            if (iteration==0 && g->INIT_OUTPUT==true) {
                ofstream out;
                out.open(o.c_str(), ios::trunc);
                out <<"SCENARIO\tDESIGN_POINT\tRANDOM\tITERATION\tPAID_TACS\tTOTAL_TACS\tPAID_TACS_IN_REGION\tTOTAL_TACS_IN_REGION\tSTAY_AT_PREV_OWNER_BECAUSE_OF_TACS\tSTAY_AT_PREV_OWNER\tRELEASED_PLOTS\tRELEASED_PLOTS_CF\tRELEASED_PLOTS_IF\tRENTED_PLOTS_CF\tRENTED_PLOTS_IF\tCF_to_CF\tCF_toIF\tIF_to_CF\tIF_toIF\tstay_CF\tstay_IF\n";
                out.close();
            }

            ofstream out;
            out.open(o.c_str(), ios::app);
            out << g->SCENARIO << "\t" << g->DESIGN_POINT << "\t"<< g->RANDOM << "\t"<< iteration <<  "\t"  << paid_tacs <<  "\t"  << total_tacs <<  "\t" << Region->calcPaidTacs() <<  "\t" << Region->calcTacs() <<  "\t" << stay_at_prev_owner_because_of_tacs <<  "\t" <<  stay_at_prev_owner <<  "\t"  <<released_plots<<  "\t"  << released_plots_CF<<  "\t"  << released_plots_IF<<  "\t"  << rented_plots_CF<<  "\t"  << rented_plots_IF <<  "\t"  << CF_to_CF<< "\t" <<CF_to_IF<< "\t" <<IF_to_CF<< "\t" <<IF_to_IF<< "\t" <<stay_CF<< "\t" <<stay_IF<< "\n";
            out.close();                                                                                                                            

         Region->setTacs();

        }
    }

}
double    RegManagerInfo::RentStatistics() {

//    if (iteration > 0) {
//}
    if(g->SECTOROUTPUT) {
    double sum=0;
    double sum_dist=0;
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        (*farms_iter)->updateLpValues();
#ifndef NDEBUG1
        g->tFarmName=(*farms_iter)->getFarmName();
        g->tFarmId= (*farms_iter)->getFarmId();
#endif
	//cout << g->tFarmId<< endl;
        sum+=    (*farms_iter)->getObjective();
        sum_dist+=(*farms_iter)->getFarmDistanceCosts();
    }
    if(g->PRINT_VA) {
    ofstream out;
    out.open("../outputfiles/wertsagrip.dat", ios::app);
    out << sum << "\t" << sum_dist << "\t" << sum-sum_dist << "\n";
    out.close();
    }
    return sum-sum_dist;
    } else {
      return 0;
    }
}

void
RegManagerInfo::InvestmentDecision() {
    ///////////////////////
    //  INVESTMENT DECISION beginning of period
    ///////////////////////
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
#ifndef NDEBUG1
        g->tFarmName=(*farms_iter)->getFarmName();
        g->tFarmId= (*farms_iter)->getFarmId();
#endif
        (*farms_iter)->doLpInvest();
    }
}
double
RegManagerInfo::Production() {
    /////////////
    // PRODUCTION
    /////////////
    double sum=0;
    // hier muessen die Auswirkungen der Investitionen schon beruecksichtigt werden
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
#ifndef NDEBUG1
        g->tFarmName=(*farms_iter)->getFarmName();
        g->tFarmId= (*farms_iter)->getFarmId();
#endif
        sum+=(*farms_iter)->doProductionLp();
    }
    return sum;
}
void
RegManagerInfo::UpdateMarket() {
    /////////
    // MARKET
    /////////
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        Sector->setTotalProduction(*farms_iter);
        Sector->setTotalLandInput(*farms_iter);
    }
    Market->priceFunction(*Sector, evaluator,iteration);
}

void
RegManagerInfo::FarmPeriodResults() {
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {

        (*farms_iter)->periodResults(iteration);
    }
    if (g->ASSOCIATE_ACTIVITIES && g->ENV_MODELING)
        Env->associateActivities(FarmList);
}
void
RegManagerInfo::RemovedFarmPeriodResults() {
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = RemovedFarmList.begin();
            farms_iter != RemovedFarmList.end();
            farms_iter++) {
        (*farms_iter)->periodResultsForRemovedFarms();
    }
}
    void
RegManagerInfo::increaseLandCapacityOfType(int farm,int type,int no_of_plots) {
f=farm;t=type;n=no_of_plots;
}

    void
RegManagerInfo::increaseLandCapacityOfTypel(int farm,int type,int no_of_plots) {
    list<RegFarmInfo* >::iterator farms_iter;
    int c=0;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        if(c==farm)
        (*farms_iter)->increaseLandCapacityOfType(type,no_of_plots);
        c++;
    }
}

void
RegManagerInfo::FarmOutput() {
    ////////////////////
    /// FARM DATA OUTPUT
    ////////////////////
        list<RegFarmInfo* >::iterator farms_iter;
        if (g->FARMOUTPUT) {
            Data->openFarmOutput();
            for (farms_iter = FarmList.begin();
                    farms_iter != FarmList.end();
                    farms_iter++) {
                if (g->CALCULATE_CONTIGUOUS_PLOTS) {
                if(g->PRINT_CONT_PLOTS)
                    Data->printContiguousPlotsOutput( (*farms_iter),iteration);
                }
                Data->cacheFarmResults((*farms_iter),
                                       InvestCatalog,
                                       Market->getProductCat(),
                                       iteration);
                if(g->PRINT_FARM_INV)
                Data->printFarmInvestment((*farms_iter),
                                          InvestCatalog,
                                          iteration);
                if(g->PRINT_FARM_PROD)
                Data->printFarmProduction((*farms_iter),
                                          Market->getProductCat(),
                                          iteration);
                if(g->PRINT_FARM_COSTS)
                Data->printFarmVarCosts((*farms_iter),
                                          Market->getProductCat(),
                                          iteration);
                //20060426
                if (g->ENV_MODELING) {
                    Data->printEnvDataUsage((*farms_iter),
                                            Market->getProductCat(),
                                            iteration);
                }
            }
            if (g->PRINT_REMOVED_FARMS) {
                for (farms_iter = RemovedFarmList.begin();
                        farms_iter != RemovedFarmList.end();
                        farms_iter++) {
                    Data->cacheFarmResults((*farms_iter),
                                           InvestCatalog,
                                           Market->getProductCat(),
                                           iteration);
                }
            }
            Data->closeFarmOutput();
        }
 //&%$   }
}
void
RegManagerInfo::SectorResults() {
    /////////////////////
    // SECTOR DATA OUTPUT
    /////////////////////
    Sector->periodResultsSector(InvestCatalog, *Region, FarmList, iteration);
    if (g->CALC_LEGAL_TYPES) {
        for (unsigned int i=0;i<g->LEGAL_TYPES.size();i++) {
            sector_type[i]->periodResultsSector(InvestCatalog, *Region, FarmList, iteration);
        }
    }

}
void
RegManagerInfo::SectorResultsAfterDisinvest() {
    /////////////////////
    // SECTOR DATA OUTPUT
    /////////////////////
    Sector->periodResultsSectorAfterDisinvest(InvestCatalog,FarmList);
    if (g->CALC_LEGAL_TYPES) {
        for (unsigned int i=0;i<g->LEGAL_TYPES.size();i++) {
            sector_type[i]->periodResultsSectorAfterDisinvest(InvestCatalog,FarmList);
        }
    }
}
void
RegManagerInfo::Disinvest() {
    list<RegFarmInfo* >::iterator farms_iter;

    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
#ifndef NDEBUG1
        g->tFarmName=(*farms_iter)->getFarmName();
        g->tFarmId= (*farms_iter)->getFarmId();
#endif            
        (*farms_iter)->updateLpValues();
        (*farms_iter)->disInvest();
		if (g->HAS_SOILSERVICE) 
			(*farms_iter)->calAvCarbons();
       (*farms_iter)->updateLpValues();
    }

}
void
RegManagerInfo::FutureOfFarms() {
    //////////////////
    // FUTURE OF FARMS
    //////////////////
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
#ifndef NDEBUG1
        g->tFarmName=(*farms_iter)->getFarmName();
        g->tFarmId= (*farms_iter)->getFarmId();
#endif 
        (*farms_iter)->futureOfFarm(iteration);
    }
}
void
RegManagerInfo::SectorOutput() {
    /////////////////////
    // SECTOR DATA OUTPUT
    /////////////////////
   if (g->CALC_LEGAL_TYPES) {
       Data->printLegalTypeResults(*Sector,sector_type,Market->getProductCat(),InvestCatalog, iteration);


   } 
    if (g->SECTOROUTPUT ) {
        if(g->PRINT_SEC_RES)
        Data->printSectorResults(*Sector,Market->getProductCat(), iteration);
        if(g->PRINT_SEC_COSTS)
        Data->printSectorVarCosts(*Sector,Market->getProductCat(), iteration);
        if(g->PRINT_SEC_PRICE)
        Data->printSectorPrices(*Sector,Market->getProductCat(), iteration);
        if(g->PRINT_SEC_EXP_PRICE)
        Data->printExpectedSectorPrices(*Sector,Market->getProductCat(), iteration);
        if(g->PRINT_SEC_COND)
        Data->printCondensedSectorOutput(*Sector, Market->getProductCat(),InvestCatalog,iteration);
        if (g->CALCULATE_CONTIGUOUS_PLOTS) {
            if(g->PRINT_CONT_PLOTS)
              Data->printRegionContiguousPlotsOutput(Region,iteration);
        }
    }
    if (g->FARMOUTPUT && g->PRINT_FARM_RES) {
        int c=0;
        Data->openFarmStandardOutput();
        list<RegFarmInfo* >::iterator farms_iter;
        for (farms_iter = FarmList.begin();
                farms_iter != FarmList.end();
                farms_iter++) {
            Data->printFarmResults((*farms_iter),
                                   InvestCatalog,
                                   Market->getProductCat(),
                                   iteration,c);
            c++;
        }
        if (g->PRINT_REMOVED_FARMS) {
            for (farms_iter = RemovedFarmList.begin();
                    farms_iter != RemovedFarmList.end();
                    farms_iter++) {
                Data->printFarmResults((*farms_iter),
                                       InvestCatalog,
                                       Market->getProductCat(),
                                       iteration,c);
                c++;
            }
        }
        Data->closeFarmStandardOutput();
    }
    if (g->ENV_MODELING) {
        Data->printSpeciesOut(Env, iteration);
    }

	if (g->HAS_SOILSERVICE) {
        Data->printSoilservice(FarmList, iteration);
    }
//   }
}

void
RegManagerInfo::EnvSpeciesCalc() {
    if (g->ENV_MODELING) {

        Env->resetHaProducedByHabitat();

        for (int i=0;i<Market->getNumProducts();i++) {
            double produced= Sector->getTotalUnitsProduced(i);
            Env->sumHaProducedByHabitat(i, produced);
        }
        if (iteration==0) {
            Env->calculateCCoefficients();
        } else {
            Env->calculateSpeciesByHabitat();
        }
    }
}

void
RegManagerInfo::RemoveFarms() {
    int size;
    size=FarmList.size();
    list<RegFarmInfo* >::iterator farms_iter;
    int df=0;

    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        if ((*farms_iter)->getClosed()) {
            RemovedFarmList.push_back((*farms_iter));
            (*farms_iter)= NULL;
            df++;
        }
    }
    /*    for(farms_iter = RemovedFarmList.begin();
         	farms_iter != RemovedFarmList.end();
      	farms_iter++) {
          if((*farms_iter)->getFarmAge()%g->GENERATION_CHANGE==0) {
    //  delete (*farms_iter);
    (*farms_iter)= NULL;
    drf++;
          }
    }
    */
    list<RegFarmInfo* >::iterator removed =
        remove_if(FarmList.begin(), FarmList.end(), CheckIfClosed());
    FarmList.erase(removed, FarmList.end());
    //    removed =
    //		remove_if(RemovedFarmList.begin(), RemovedFarmList.end(), CheckIfClosed());
    //    RemovedFarmList.erase(removed, RemovedFarmList.end());
    size-=FarmList.size();

    // if pointer farms_iter is NULL remove it from FarmList
    // remove only moves the unwanted elements to the back of the List
    // erase actually erases the elements and changes the size of the list



    /*

    // remove closed farms from list
    // Es gibt auch die Ueberlegung die bereits geschlossenen
    // Betrieb in der List zu lassen und zu sehen, wie sie
    // sich weiter entwickeln, denn sie verpachten ihr Eigenland
    // und bekommen somit neben Arbeitseinkommen auch Pachteinnahmen

    list<RegFarmInfo* >::iterator farms_iter;
    for(farms_iter = FarmList.begin();
    farms_iter != FarmList.end();
    farms_iter++) {
    if((*farms_iter)->getClosed()==true) {
    delete (*farms_iter);
    (*farms_iter)= NULL;
    }
    }
    list<RegFarmInfo* >::iterator removed = remove_if(FarmList.begin(), FarmList.end(), CheckIfClosed());
    FarmList.erase(removed, FarmList.end());
    // if pointer farms_iter is NULL remove it from FarmList
    // remove only moves the unwanted elements to the back of the List
    // erase actually erases the elements and changes the size of the list
    */
}
void
RegManagerInfo::ProcessMessages() {
    iteration++;
}


//---------------------------------------------------------------------------
//
//      UPDATE FUNCTIONS
//
//---------------------------------------------------------------------------

void
RegManagerInfo::updateInterestDependentValues() {
    vector<RegInvestObjectInfo >::iterator invest;

    double acqcosts;
    double maintenance;
    int lifetime;
    int type;

    double aec;       // bound equity capital on average
    double aac;       // average costs
    double le;        // liquidity effect

    // interest on borrowed and equity capital
    double bc_interest = g->INTEREST_RATE;
    vector<RegProductInfo>& prod_cat = Market->getProductCat();
    double ec_interest = prod_cat[g->ST_EC_INTERESTTYPE].getPrice();
    for (invest = InvestCatalog.begin();
            invest != InvestCatalog.end();
            invest++) {
        acqcosts = invest->getAcquisitionCosts();
        lifetime = invest->getEconomicLife();
        maintenance = invest->getMaintenanceCosts();
        type = invest->getInvestType();

        switch (type) {
        case 0:  // hired fix labour
            if (iteration > 0) {
                aac = acqcosts * g->INCREASEPRICE;
                invest->setAcquisitionCosts(aac);
            } else {
                aac = acqcosts;
            }
            break;
        case 1:  // fix off-farm labour
            if (iteration > 0) {
                aac = acqcosts * g->INCREASEPRICE;
                invest->setAcquisitionCosts(aac);
            } else {
                aac = acqcosts;
            }
            break;
        default:
            aac =  g->SHARE_SELF_FINANCE*  ( acqcosts / lifetime)
                   +  (1 - g->SHARE_SELF_FINANCE) * acqcosts * capitalReturnFactor(bc_interest, lifetime)
                   + maintenance;
            break;
        }

        invest->setAverageCost(aac);

        // average annual costs of investment (gross margin, obj. function)
        // includes maintenance costs
        // approximative Kalkulation

        //    aac = acqcosts * (g->SHARE_SELF_FINANCE / lifetime)
        //       + ( (acqcosts + maintenance*(1/capitalReturnFactor(interest,lifetime)))
        //               * (1 - g->SHARE_SELF_FINANCE)
        //               * capitalReturnFactor(interest, lifetime)); //capital costs

        // liquidity effect of investment (equity capital)
        le = (acqcosts)* g->SHARE_SELF_FINANCE;
        invest->setLiqEffect(le);
        // average bound equity (capital for calculatory interest)
        // FK-Anspruch
        aec = g->SHARE_SELF_FINANCE * (acqcosts) * averageReturn(ec_interest, lifetime);
        invest->setBoundEquityCapital(aec);
    }
}
// type 0: arableLand, type 1:grassLand
double
RegManagerInfo::rentOnePlot(vector<int>& count_rented_plots_of_type, int type) {
    //double second_offer;
    if (g->OLD_LAND_RENTING_PROCESS) {
        // maximum offer at a point in time
        double offer;
        double maxoffer;
        double true_second_offer=0;
        // list of farms with equal offer
        list<RegFarmInfo* >  equalbidder;
        list<RegFarmInfo* >::iterator farms_iter;
        list<RegFarmInfo* >::iterator equalbidder_iter;
        list<RegFarmInfo* >::iterator  prev_owner;

        //Um von Durchlauf zu Durchlauf zu wechsel 2 boolsche Variablen
        RegFarmInfo* maxbidder;
        // search for the highest offer
        for (farms_iter = FarmList.begin(), maxoffer = 0, maxbidder = NULL, equalbidder.clear();
                farms_iter != FarmList.end();
                farms_iter++) {
            // determine individual rent offer for plot
            // here,  0: arable, 1: grassland
#ifndef NDEBUG1
        g->tFarmName=(*farms_iter)->getFarmName();
        g->tFarmId= (*farms_iter)->getFarmId();
#endif
            (*farms_iter)->demandForLandOfType(type,bidcount);
            offer=(*farms_iter)-> getRentOffer();

            if (offer>0) {

//DCX
if (offer/(fabs(maxoffer-offer)) < 1E-5 ) {
//                if ((maxoffer) == offer) {
                    equalbidder.push_back(*farms_iter);
                } else {
                    // if current offer is higher than maxoffer
                    if ((maxoffer) < offer) {
                        // set maxbiddr to highest bidding farm
                        maxbidder = *farms_iter;
                        // if there is a higher offer, the list of equal bidders
                        // is senseless and is cleared
                        equalbidder.clear();
                        // the equalbidder list is filled
                        // with new offers
                        equalbidder.push_back(maxbidder);
                        maxoffer = offer;
                    }
                }// end else
            }
        }// end for

        // allocate land to highest bidder
//        if (g->WEIGHTED_PLOT_SEARCH)
//        Region->resetUpdate();
        if (maxoffer > 0) {
            for (equalbidder_iter = equalbidder.begin();
                    equalbidder_iter != equalbidder.end();
                    equalbidder_iter++) {
                // allocate land to highest bidder
                // offercount is incremented by the farm
                RegPlotInfo* pl=(*equalbidder_iter)->getWantedPlotOfType(type);
                if (pl->getState() == 0) {
                    count_rented_plots_of_type[type]+=1;
                }
                maxoffer=(*equalbidder_iter)->getRentOffer();
                if(!g->FIRSTPRICE) {
                  for (farms_iter = FarmList.begin();
                        farms_iter != FarmList.end();
                        farms_iter++) {
                        if ((*farms_iter)->getFarmId()!=(*equalbidder_iter)->getFarmId()) {
                          double tmp_offer=(*farms_iter)->getRentOffer(pl);
                          if (tmp_offer>true_second_offer)
                            true_second_offer=tmp_offer;
                        }

                  }
                }
                if (g->FIRSTPRICE) {

                    offer=(*equalbidder_iter)->getRentOffer();
                } else {

                    offer=true_second_offer;
                }
                double f_tac=0;
                double tac=0;
                if (g->USE_TC_FRAMEWORK)  {
                    f_tac=(*equalbidder_iter)->getFarmTacsOfWantedPlotOfType(pl->getSoilType());
                    tac=(*equalbidder_iter)->getTacsOfWantedPlotOfType(pl->getSoilType());
                }
                (*equalbidder_iter)->setRentedPlot(pl,offer,f_tac);

                if (g->USE_TC_FRAMEWORK) {
                    paid_tacs+=f_tac;
                    total_tacs+=tac;
                    if (pl->getPreviouslyRentedByAgent()==pl->getRentedByAgent())
                        stay_at_prev_owner+=g->PLOT_SIZE;
                    if ( pl->getPreviouslyRentedByAgent()==pl->getRentedByAgent() && ((maxoffer-true_second_offer)<tac))
                        stay_at_prev_owner_because_of_tacs+=g->PLOT_SIZE;
                    released_plots+=g->PLOT_SIZE;

                    if (pl->getPreviouslyRentedByLegalType()==pl->getRentedByLegalType() && pl->getRentedByLegalType()==3 && pl->getPreviouslyRentedByAgent()!=pl->getRentedByAgent())
                        CF_to_CF+=g->PLOT_SIZE;
                    if (pl->getPreviouslyRentedByLegalType()==pl->getRentedByLegalType() && pl->getRentedByLegalType()==1 && pl->getPreviouslyRentedByAgent()!=pl->getRentedByAgent())
                        IF_to_IF+=g->PLOT_SIZE;
                    if (pl->getRentedByLegalType()==3 && pl->getPreviouslyRentedByAgent()==pl->getRentedByAgent())
                        stay_CF+=g->PLOT_SIZE;
                    if (pl->getRentedByLegalType()==1 && pl->getPreviouslyRentedByAgent()==pl->getRentedByAgent())
                        stay_IF+=g->PLOT_SIZE;
                    if (pl->getPreviouslyRentedByLegalType()!=pl->getRentedByLegalType() && pl->getRentedByLegalType()==3)
                        IF_to_CF+=g->PLOT_SIZE;
                    if (pl->getPreviouslyRentedByLegalType()!=pl->getRentedByLegalType() && pl->getRentedByLegalType()==1)
                        CF_to_IF+=g->PLOT_SIZE;
                    if (pl->getPreviouslyRentedByLegalType()==3)
                        released_plots_CF+=g->PLOT_SIZE;
                    else
                        released_plots_IF+=g->PLOT_SIZE;
                    if (pl->getRentedByLegalType()==3)
                        rented_plots_CF+=g->PLOT_SIZE;
                    else
                        rented_plots_IF+=g->PLOT_SIZE;
                }


                //Mark plots which have to be updated;
                // pl->identifyContiguousPlot(true,false,false,-1,true);

            }
        }

        return maxoffer;
    } else {
        // maximum offer at a point in time
        double max_offer=0;
        double second_offer=0;
        // list of farms with equal offer
        list<RegFarmInfo* >  equalbidder;
        list<RegFarmInfo* >::iterator farms_iter;
        list<RegFarmInfo* >::iterator equalbidder_iter;

        //Um von Durchlauf zu Durchlauf zu wechsel 2 boolsche Variablen
        RegFarmInfo* maxbidder;
        // search for the highest offer
        RegPlotInfo* p=Region->getRandomFreePlot();
        if (p==NULL) return 0;
        else {
            for (farms_iter = FarmList.begin(), max_offer = 0, maxbidder = NULL, equalbidder.clear();
                    farms_iter != FarmList.end();
                    farms_iter++) {
                (*farms_iter)->demandForLand(p);
                if ((*farms_iter)->getRentOffer()>0) {
                    if ((max_offer) == ((*farms_iter)->getRentOffer())) {
                        equalbidder.push_back(*farms_iter);
                    } else {
                        // if current offer is higher than maxoffer
                        if ((max_offer) < ((*farms_iter)->getRentOffer())) {
                            // set maxbidder to highest bidding farm
                            maxbidder = *farms_iter;
                            // if there is a higher offer, the list of equal bidders
                            // is senseless and is cleared
                            equalbidder.clear();
                            // the equalbidder list is filled
                            // with new offers
                            equalbidder.push_back(maxbidder);
                            second_offer=max_offer;
                            max_offer = maxbidder->getRentOffer();
                        }
                    }// end else
                }
            }// end for
            // allocate land to highest bidder
            if (max_offer > 0) {
                int w =randlong()%equalbidder.size();

                equalbidder_iter = equalbidder.begin();
                for (int i=0;i<w;i++)
                    equalbidder_iter++;
                // Second Price Auction!!!
                if (g->FIRSTPRICE) {
                    (*equalbidder_iter)->setRentedPlot(p,max_offer,0);
                } else {
                    (*equalbidder_iter)->setRentedPlot(p,second_offer,0);
                }
                count_rented_plots_of_type[p->getSoilType()]++;
            }
         return second_offer;
        }
    }
}


void
RegManagerInfo::readPolicyChanges0() {
    evaluator->clearFunctionBase();
    string e=Policyoutput->getPolicySettings(0);
    evaluator->addFunctionBase(e);
    evaluator->evaluate();
}

void
RegManagerInfo::readPolicyChanges() {
    if (g->PRINT_POLICY) {
        Data->openPolicyOutput();

        if (iteration==0)  {
            Data->initPolicyOutput();
        }
    }
    evaluator->clearFunctionBase();
    if (iteration != 0) {
        vector<double> val = Data->getSectorValues();
        vector<string> n = Data->getSectorNames();
        for (unsigned int i=0;i<n.size();i++) {
            evaluator->setVariable(n[i],val[i]);
        }
    }
    string e=Policyoutput->getPolicySettings(iteration+1);
    evaluator->addFunctionBase(e);
    evaluator->evaluate();
}
void
RegManagerInfo::setPolicyChanges() {
    setPremium();
    setDecoupling();
    setLpChangesFromPoliySettings();
    if (g->PRINT_POLICY) {
        Data->printPolicyOutput("\n");
        Data->closePolicyOutput();
    } 
}


void
RegManagerInfo::setPremium() {
    vector<int> cols=Market->getColsOfPremiumProducts();
    vector<int> rows=Market->getRowsOfPremiumProducts();
    vector<double> premium;
    vector<string> names=Market->getNamesOfPremiumProducts();
    for (unsigned int i=0;i<names.size();i++) {
         premium.push_back(evaluator->getVariable(names[i]+"_premium"));
    }
    //cout << endl;
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        for (unsigned int i=0;i<premium.size();i++) {
            (*farms_iter)->lp->setCellValue(cols[i],rows[i],-premium[i]);
        }
    }
    if (g->SECTOROUTPUT) {
        for (unsigned int i=0;i<premium.size();i++)  {
            stringstream s;
            s<<names[i]<<"_premium="<<premium[i]<<";\t";
            Data->printPolicyOutput(s.str());
        }
    }
}
void
RegManagerInfo::calculateExpectedRentalPriceChange() {
    backup();
    g->backup();
    f=static_cast<int>(Region->getExpAvNewRentOfType(0)/Region->getAvRentOfType(0));
    setRegionalDecoupling();
    setFullyDecoupling();
    setFarmspecificDecoupling();
    setLpChangesFromPoliySettings();
    f=static_cast<int>(Region->getExpAvNewRentOfType(0)/Region->getAvRentOfType(0));

        setLpChangesFromPoliySettings();
    g->FARMOUTPUT=false;
    g->SECTOROUTPUT=false;
    g->ENV_MODELING=false;
    CapacityEstimationForBidding();
    Disinvest();
    Region->calculateAverageRent();
    Region->calculateAverageNewRent();
    FutureOfFarms();
    SectorResultsAfterDisinvest();
    ResetPeriodLabour();
    SectorOutput();
    EnvSpeciesCalc();
    RemoveFarms();

    PreparationForPeriod();
    // adjust costs by management coefficient
    if (iteration == 0)    readPolicyChanges();
    if (g->CALCULATE_CONTIGUOUS_PLOTS) Region->countContiguousPlots();
    g->WERTS1=RentStatistics();
    g->WERTS=-g->WERTS1;
    LandAllocation();
    Region->calculateAverageRent();
    Region->calculateAverageNewRent();
    vector<double> exp;
    vector<double> exp_new;
    for(int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
      exp.push_back(Region->getAvRentOfType(i));
      exp_new.push_back(Region->getAvNewRentOfType(i));
    }
    restore();
    g->restore();
    for(int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
      Region->setExpAvRentOfType(i,exp[i]);
      Region->setExpAvNewRentOfType(i,exp_new[i]);
    }
}
void
RegManagerInfo::setDecoupling() {
    calculateReferencePaymentPerFarm();
    if(g->CALCULATE_EXPECTED_RENTAL_PRICE) {
      if(setModulationData())
          calculateExpectedRentalPriceChange();
    } else {
      setModulationData();
    }
    setRegionalDecoupling();
    setFullyDecoupling();
    setFarmspecificDecoupling();
}

void
RegManagerInfo::calculateReferencePaymentPerFarm() {
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        (*farms_iter)->calculateReferencePeriod();
        if (g->FIX_REFERENCE_PERIOD==iteration) {
            (*farms_iter)->fixReferencePeriod();
        }
    }
    vector<double> reference_premium;
    for (unsigned int i=0;i<Market->getProductCat().size();i++) {
        if (Market->getProductCat()[i].getPremiumLegitimation()) {
            reference_premium.push_back(Market->getProductCat()[i].getReferencePremium());
        }
    }
    vector<string> names=Market->getNamesOfPremiumProducts();
    vector<double> reference_premium_percent;
    for (unsigned int i=0;i<names.size();i++) {
        reference_premium_percent.push_back(evaluator->getVariable(names[i]+"_reference_premium_percent"));
    }


    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        double farm_payment=0;
        vector <double> reference_production=(*farms_iter)->getReferencePeriodProduction();
        for (unsigned int i=0;i<reference_production.size();i++) {
            farm_payment+=reference_production[i]*reference_premium_percent[i]*reference_premium[i];
        }
        if (iteration==0)
            (*farms_iter)->setOldReferenceDirectPayment(farm_payment);
        else
            (*farms_iter)->setOldReferenceDirectPayment((*farms_iter)->getOldReferenceDirectPayment());
        (*farms_iter)->setReferenceDirectPayment(farm_payment);
    }

}
void
RegManagerInfo::ModulateDirectPayments() {
    Region->modulateDirectPayments();
}
void
RegManagerInfo::setLpChangesFromPoliySettings() {
    //go through all variables and select those how affect the farm lp's
    // the following naming convention is used
    // mat_cell_x4y12=0.8;  //to set a cell in the matrix
    // rhs_row4=0;     0: less_equal 1: equal 2: grater equal
    // ub_col5=0;   0:0 1: +INF
    vector<int> mat_x;
    vector<int> mat_y;
    vector<double> mat_val;
    vector<int> rhs_row;
    vector<int> rhs_val;
    vector<int> ub_col;
    vector<int> ub_val;

    int no=evaluator->getNoVariables();
    for (int i=0;i<no;i++) {
        string var =evaluator->getVariableName(i);
        double val=evaluator->getVariable(i);
        int pos;
        if ((pos=var.find("mat_cell_x",0))!=std::string::npos) {
            var.replace(0,10,"");
            if ((pos=var.find("y",0))!=std::string::npos) {
                var.replace(pos,1," ");
                istringstream var_s(var);
                int x;
                var_s>>x;
                int y;
                var_s>>y;
                mat_x.push_back(x);
                mat_y.push_back(y);
                mat_val.push_back(val);
            }
        }
        if ((pos=var.find("rhs_row",0))!=std::string::npos) {
            var.replace(0,7,"");
            istringstream var_s(var);
            int row;
            var_s>>row;
            rhs_row.push_back(row);
            rhs_val.push_back(static_cast<int>(val));
        }
        if ((pos=var.find("ub_col",0))!=std::string::npos) {
            var.replace(0,6,"");
            istringstream var_s(var);
            int col;
            var_s>>col;
            ub_col.push_back(col);
            ub_val.push_back(static_cast<int>(val));
        }

    }
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        for (unsigned int i=0;i<mat_val.size();i++) {
            (*farms_iter)->lp->setCellValue(mat_x[i],mat_y[i],mat_val[i]); //set aside
        }
        for (unsigned int i=0;i<rhs_val.size();i++) {
            switch (rhs_val[i]) {
            case 0:
                (*farms_iter)->lp->setSenseLessEqual(rhs_row[i]);
                break;
            case 1:
                (*farms_iter)->lp->setSenseEqual(rhs_row[i]);
                break;
            case 2:
                (*farms_iter)->lp->setSenseGreaterEqual(rhs_row[i]);
                break;
            default:
                break;
            }
        }
        for (unsigned int i=0;i<ub_val.size();i++) {
            switch (ub_val[i]) {
            case 0:
                (*farms_iter)->lp->setUBoundZero(ub_col[i]);
                break;
            case 1:
                (*farms_iter)->lp->setUBoundInf(ub_col[i]);
                break;
            default:
                break;
            }
        }
        //  (*farms_iter)->updateLpValues();
    }
}


void
RegManagerInfo::setFullyDecoupling() {
    //Switch on fully Decoupling
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
            farms_iter != FarmList.end();
            farms_iter++) {
        if (g->FULLY_DECOUPLING_SWITCH) {
            if (g->FULLY_DECOUPLING) {
                (*farms_iter)->setDirectPayment((*farms_iter)->getReferenceDirectPayment()/*-50*land_input*/);
				if(!g->LP_MOD)
                (*farms_iter)->modulateIncomePayment();
            } else {
                (*farms_iter)->setDirectPayment(0);
				if(!g->LP_MOD)
                (*farms_iter)->modulateIncomePayment();
            }
        } else {
            if (g->FULLY_DECOUPLING) {
                (*farms_iter)->setDirectPayment((*farms_iter)->getReferenceDirectPayment()/*-50*land_input*/);
				if(!g->LP_MOD)
                (*farms_iter)->modulateIncomePayment();
            }
        }
    }
}

void
RegManagerInfo::setRegionalDecoupling() {
    if (g->REGIONAL_DECOUPLING_SWITCH) {
        //Switch on regional decoupling
        if (g->REGIONAL_DECOUPLING) {
            double total_payment=0;
            double total_land_input=0;
            list<RegFarmInfo* >::iterator farms_iter;
            for (farms_iter = FarmList.begin();
                    farms_iter != FarmList.end();
                    farms_iter++) {
                total_payment+=(*farms_iter)->getReferenceDirectPayment();
                total_land_input+=(*farms_iter)->getLandInput();
            }
            total_land_input+=Region->getNumberOfFreePlots()*g->PLOT_SIZE;
            double payment_per_ha=total_payment/total_land_input;
            for (unsigned int i=0;i<Region->plots.size();i++) {
                Region->plots[i]->setPaymentEntitlement(payment_per_ha*g->PLOT_SIZE);
            }
            for (farms_iter = FarmList.begin();
                    farms_iter != FarmList.end();
                    farms_iter++) {
                double land_input =(*farms_iter)->getLandInput();
                (*farms_iter)->setDirectPayment(land_input*payment_per_ha);
				if(!g->LP_MOD)
                (*farms_iter)->modulateIncomePayment();
            }
        } else {
            list<RegFarmInfo* >::iterator farms_iter;
            for (unsigned int i=0;i<Region->plots.size();i++) {
                Region->plots[i]->setPaymentEntitlement(0);
            }
            for (farms_iter = FarmList.begin();
                    farms_iter != FarmList.end();
                    farms_iter++) {
                (*farms_iter)->setDirectPayment(0);
				if(!g->LP_MOD)
                (*farms_iter)->modulateIncomePayment();
            }
        }
    } else {
        if (g->REGIONAL_DECOUPLING) {
            //Bei sich ändernder Modulierung ist alles etwas komplizierter: also später
        }
    }
}
void
RegManagerInfo::setFarmspecificDecoupling() {
    if (g->FARMSPECIFIC_DECOUPLING_SWITCH) {
        //Switch on regional decoupling
        if (g->FARMSPECIFIC_DECOUPLING) {
            list<RegFarmInfo* >::iterator farms_iter;
            for (farms_iter = FarmList.begin();
                    farms_iter != FarmList.end();
                    farms_iter++) {
                double payment =(*farms_iter)->getReferenceDirectPayment();
                double land_input =(*farms_iter)->getLandInput();
                double payment_per_ha=payment/land_input;

                (*farms_iter)->setDirectPaymentPerPlot(payment_per_ha*g->PLOT_SIZE);

                (*farms_iter)->setDirectPayment(payment);
				if(!g->LP_MOD)
                (*farms_iter)->modulateIncomePayment();
            }
        } else {
            list<RegFarmInfo* >::iterator farms_iter;
            for (unsigned int i=0;i<Region->plots.size();i++) {
                Region->plots[i]->setPaymentEntitlement(0);
            }
            for (farms_iter = FarmList.begin();
                    farms_iter != FarmList.end();
                    farms_iter++) {
                (*farms_iter)->setDirectPayment(0);
				if(!g->LP_MOD)
                (*farms_iter)->modulateIncomePayment();
            }
        }
    } else {
        if (g->FARMSPECIFIC_DECOUPLING) {
            //Bei sich ändernder Modulierung ist alles etwas komplizierter: also später
        }
    }
}
bool
RegManagerInfo::setModulationData() {
bool exp_rent=false;

    // Switch bezeichnet ob sich eine Änderung zur Vorperiode ergeben hat.
    if (g->FULLY_DECOUPLING==evaluator->getVariable("fully_decoupling")) {
        g->FULLY_DECOUPLING_SWITCH=0;
    } else {
        g->FULLY_DECOUPLING_SWITCH=1;
        g->FULLY_DECOUPLING=static_cast<int>(evaluator->getVariable("fully_decoupling"));
        if(g->FULLY_DECOUPLING) exp_rent=true;
    }

    if (g->REGIONAL_DECOUPLING==evaluator->getVariable("regional_decoupling")) {
        g->REGIONAL_DECOUPLING_SWITCH=0;
    } else {
        g->REGIONAL_DECOUPLING_SWITCH=1;
        g->REGIONAL_DECOUPLING=static_cast<int>(evaluator->getVariable("regional_decoupling"));
        if(g->REGIONAL_DECOUPLING) exp_rent=true;
    }

    if (g->FARMSPECIFIC_DECOUPLING==evaluator->getVariable("farmspecific_decoupling")) {
        g->FARMSPECIFIC_DECOUPLING_SWITCH=0;
    } else {
        g->FARMSPECIFIC_DECOUPLING_SWITCH=1;
        g->FARMSPECIFIC_DECOUPLING=static_cast<int>(evaluator->getVariable("farmspecific_decoupling"));
        if(g->FARMSPECIFIC_DECOUPLING) exp_rent=true;
    }
	if(g->LP_MOD) {
    g->TRANCH_1_DEG=evaluator->getVariable("tranch_1_deg")-1;
    g->TRANCH_2_DEG=evaluator->getVariable("tranch_2_deg")-1;
    g->TRANCH_3_DEG=evaluator->getVariable("tranch_3_deg")-1;
    g->TRANCH_4_DEG=evaluator->getVariable("tranch_4_deg")-1;
    g->TRANCH_5_DEG=evaluator->getVariable("tranch_5_deg")-1;
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
         farms_iter != FarmList.end();
         farms_iter++) {
        (*farms_iter)->updateLpValues();
    }

} else {
    g->DEG_LOW_TRANCH=evaluator->getVariable("degression_low_tranch");
    g->DEG_MIDDLE_TRANCH=evaluator->getVariable("degression_middle_tranch");
    g->DEG_HIGH_TRANCH=evaluator->getVariable("degression_high_tranch");
    list<RegFarmInfo* >::iterator farms_iter;
    for (farms_iter = FarmList.begin();
         farms_iter != FarmList.end();
         farms_iter++) {
        (*farms_iter)->modulateIncomePayment();
    }
}
    return exp_rent;

}

void
RegManagerInfo::backup() {
    obj_backup=clone();
    Sector->backup();
    if (g->ENV_MODELING)
        Env->backup();
    Region->backup();
    Market->backup();
    evaluator->backup();
    list<RegFarmInfo* >::const_iterator farms;
    for (farms = FarmList.begin();
            farms != FarmList.end();
            farms++) {
        (*farms)->backup();
    }
    for (farms = RemovedFarmList.begin();
            farms != RemovedFarmList.end();
            farms++) {
        (*farms)->backup();
    }
}
void RegManagerInfo::assign() {
    *((RegManagerInfo*)this)=*obj_backup;
}
;
void
RegManagerInfo::restore() {
    RegManagerInfo* tmp=obj_backup;
    assign();
    obj_backup=tmp;

    list<RegFarmInfo* >::const_iterator farms;
    for (farms = FarmList.begin();
            farms != FarmList.end();
            farms++) {
        (*farms)->restore();
    }
    for (farms = RemovedFarmList.begin();
            farms != RemovedFarmList.end();
            farms++) {
        (*farms)->restore();
    }
    Sector->restore();
    if (g->ENV_MODELING)
        Env->restore();
    Region->restore();
    Market->restore();
    evaluator->restore();
}
void
RegManagerInfo::printShadowPrices(int nop)  {
    ofstream out;
    string file=g->OUTPUTFILE + string("avshadow.dat");
    out.open(file.c_str(),ios::ate);
    ofstream out2;
    string file2=g->OUTPUTFILE + string("shadow.dat");
    out2.open(file2.c_str(),ios::ate);
    ofstream out3;
    string file3=g->OUTPUTFILE + string("dk.dat");
    out2.open(file2.c_str(),ios::ate);

    list<RegFarmInfo* >::const_iterator farms;

    for(int i=0;i<=g->NO_OF_SOIL_TYPES;i++) {
      for (farms = FarmList.begin();
           farms != FarmList.end();
           farms++) {
              out << (*farms)->getFarmId() << "\t";
              out2 << (*farms)->getFarmId() << "\t";
              out3 << (*farms)->getFarmId() << "\t";
      }
    }
    out << "\n";
    out2 << "\n";
    for(int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
      for (farms = FarmList.begin();
           farms != FarmList.end();
           farms++) {
              out << (*farms)->getLandInputOfType(i) << "\t";
              out2 << (*farms)->getLandInputOfType(i) << "\t";
              out3 << (*farms)->getLandInputOfType(i) << "\t";
      }
    }
    for (farms = FarmList.begin();
         farms != FarmList.end();
         farms++) {
         double sum=0;
        for(int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
            sum+=(*farms)->getLandInputOfType(i);
        }
        out << sum << "\t";
        out2 << sum << "\t";
        out3 << sum << "\t";

    }
    out << "\n";
    out2 << "\n";
    out3 << "\n";
    vector<int> soils;
    for(int i=0;i<g->NO_OF_SOIL_TYPES;i++) {
      soils.push_back(0);
    }

    for(int i=0;i<nop;i++) {
      for(int k=0;k<g->NO_OF_SOIL_TYPES;k++) {
        for (farms = FarmList.begin();
             farms != FarmList.end();
            farms++) {
              for(int j=0;j<g->NO_OF_SOIL_TYPES;j++) {
                if(k==j)
                  soils[j]=i;
                else
                  soils[j]=0;
              }
              double val=(*farms)->getValueOfPlots(soils);
              out << ((i==0)? 0 :(val/(i*g->PLOT_SIZE))) << "\t";
              out2 << ( (i==0)? 0 :val) << "\t";

        }
      }
      for(int j=0;j<g->NO_OF_SOIL_TYPES;j++) {
        soils[j]=i/2;
      }
      for (farms = FarmList.begin();
           farms != FarmList.end();
           farms++) {
           if((i%2)==0) {
              double val=(*farms)->getValueOfPlots(soils);
              out << ((i==0)? 0 :(val/(i*g->PLOT_SIZE))) << "\t";
              out2 << ((i==0)? 0 :val) << "\t";
           } else {
             out <<  "\t";
             out2 <<  "\t";
           }
      }
      out << "\n";
      out2 << "\n";
    }
    out.close();
    out2.close();
//DCX
out3.close();
}


RegFarmInfo* RegManagerInfo::createFarm(RegRegionInfo * reg,
                                        RegGlobalsInfo* G,
                                        vector<RegProductInfo >& PCat,
                                        vector<RegInvestObjectInfo >& ICat ,
                                        RegLpInfo* lporig,
                                        short int pop,
                                        int number,
                                        int fc,
                                        string farmname,
                                        int farmerwerbsform) {

    return new RegFarmInfo(reg,G,PCat,ICat ,lporig,pop,number,fc,farmname,farmerwerbsform);
}
RegManagerInfo* RegManagerInfo::clone() {
    RegManagerInfo* m=new RegManagerInfo(*this);
    m->flat_copy=true;
    return m;
}
RegManagerInfo* RegManagerInfo::create() {
    return new RegManagerInfo();
}



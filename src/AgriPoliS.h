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

//  functions for reading options from options.txt
/* Dong Changxing, 2011  */
//==================================================
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <iterator>
#include <vector>
#include <string>

#include "RegGlobals.h"

const string OPTFILE = "options.txt";

RegGlobalsInfo* gg;

static map<string,string> optionsdata;
static string optiondir;

static void tokenize(const string& str,
                      vector<string>& tokens,
                      const string& delimiters = " \t=;")
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find next "delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);
    int n= 0;
    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        n++;

        lastPos = str.find_first_not_of(delimiters, pos);
        pos = str.find_first_of(delimiters, lastPos);
    }
    if (n<2) tokens.push_back("=");
}

void readoptions() {
    ifstream ins;
    stringstream gfile;
    gfile << optiondir <<OPTFILE;
    ins.open(gfile.str().c_str(),ios::in);
	if ( ! ins.is_open() ) {
       cerr << "Error while opening: " << gfile.str() << "\n";
	   exit(2);
    }
    string s, s2, sback;

    vector <string> tokens;

    while (!ins.eof()){
        getline(ins,s);
        sback = s;
        if (s.compare("")==0) continue;
        if (s[0]=='#') continue;
        tokens.clear();
        tokenize(s, tokens);

		if (tokens.size()==1) continue;
        s2=tokens[0];
        std::transform(s2.begin(), s2.end(), s2.begin(),
               (int(*)(int)) std::toupper);

        optionsdata.insert(pair<string, string>(s2,tokens[1]));
    }
    ins.close();

    return;
}

void setoptions(){
//eralier from GUI
gg->RUNS=atoi(optionsdata["RUNS"].c_str());

string tstr = optionsdata["INPUTFILEDIR"];

if (tstr.compare("=")==0) gg->INPUTFILEdir= optiondir;
else if (tstr[tstr.length()-1]!='\\' ) gg->INPUTFILEdir = tstr+ "\\";
else gg->INPUTFILEdir = tstr;

string updir = gg->INPUTFILEdir;
size_t pos =updir.find_last_not_of('\\');
updir = updir.substr(0,pos);
pos = updir.find_last_of('\\');
updir = updir.substr(0,pos+1);

tstr = gg->POLICYFILE;
if (tstr.compare("")==0)
    gg->POLICYFILE = gg->INPUTFILEdir+"policy_settings.txt";
else
    gg->POLICYFILE = gg->INPUTFILEdir+tstr;

string extstr=gg->POLICYFILE;
pos = extstr.find_last_of('\\');

if ( pos != string::npos)
    extstr= extstr.substr(pos+1,string::npos);

pos = extstr.find("policy_settings");
size_t pos1 = extstr.find('.');

string ext;
if (pos==string::npos){
    if (pos1==string::npos) ext = extstr;
    else ext = extstr.substr(0,pos1);
}else {
    if (pos1==string::npos) ext = extstr.substr(pos+15);
    else ext =  extstr.substr(pos+15,pos1-pos-15) ;
}

tstr = optionsdata["OUTPUTFILE"];
if (tstr.compare("=")==0) gg->OUTPUTFILE = updir + "outputfiles"+ ext + "\\";
else if (tstr[tstr.length()-1]!='\\' ) gg->OUTPUTFILE = tstr+ "\\"  ;
else gg->OUTPUTFILE = tstr;

gg->FARMOUTPUT = atoi(optionsdata["FARMOUTPUT"].c_str());
gg->SECTOROUTPUT=atoi(optionsdata["SECTOROUTPUT"].c_str());

//---  Region dependent Globals ------------
//Standard: 9, CZ   =   5, Saxony   =   12
gg->MIN_CONTRACT_LENGTH   = atoi(optionsdata["MIN_CONTRACT_LENGTH"].c_str());

//Standard: 18, Saxony   =   24
gg->MAX_CONTRACT_LENGTH   =  atoi(optionsdata["MAX_CONTRACT_LENGTH"].c_str());

//Standard: false, Sweden: true (calf and milk quota market)
gg->SWEDEN   =  (optionsdata["SWEDEN"]).compare("true")==0 ? true : false;

// flase means the settings for Västerbotton are used. THis is only relevant if Sweden is true.
gg->JOENKEPING   =  optionsdata["JOENKEPING"].compare("true")==0 ? true : false;

//Standard: false, CZ, Västerbotten, Jönköping: true
gg->ENV_MODELING = optionsdata["ENV_MODELING"].compare("true")==0 ? true : false;

//true only for regions with various soil qualities of arable and/or grassland, like OPR.
//If true, then shares of soil types will be varried among the farms
gg->SOIL_TYPE_VARIATION   = optionsdata["SOIL_TYPE_VARIATION"].compare("true")==0 ? true : false;

//ACHTUNG: INTEGER_SWITCH soll IMMER auf false gesetzt werden !!!
//Introduced for OPR, not necessary for the new version
//gg->INTEGER_SWITCH   =  optionsdata["INTEGER_SWITCH"].compare("true")==0 ? true : false;
gg->INTEGER_SWITCH   =  false;

//Standard: false, Brittany: true
gg->OLD_LAND_RELEASING_PROCESS   = optionsdata["OLD_LAND_RELEASING_PROCESS"].compare("true")==0 ? true : false;

//standard: true, OPR false farm areas are initialised as a circle around the farm  they search
//for the plot with the lowest costs, however this only matters when transaction costs are considered
gg->WEIGHTED_PLOT_SEARCH   =   optionsdata["WEIGHTED_PLOT_SEARCH"].compare("true")==0 ? true : false;

//Standard: false, CZ, Lithuania, Slovakia: true
gg->AGE_DEPENDENT   =  optionsdata["AGE_DEPENDENT"].compare("true")==0 ? true : false;

//Standard: false, true is necessary for Saxony, Hohenlohe, Brittany and CZ because here we have
//different input files to calculate the modulation within the LP-model
gg->LP_MOD   =  optionsdata["LP_MOD"].compare("true")==0 ? true : false;

//Attention, it is only differentiated between IF   =   1 and CF   =   3. In the input-files of Hohenlohe, Saxony,
//OPR and Brittany more legal types are defined. In Brittany and Saxony CF   =   2. To changes this search above for "Legal_types"
gg->CALC_LEGAL_TYPES   =  optionsdata["CALC_LEGAL_TYPES"].compare("true")==0 ? true : false;

//===================================================
//New 04.04.2011 Soilservice
//====================================================
gg->HAS_SOILSERVICE=optionsdata["HAS_SOILSERVICE"].compare("true")==0 ? true : false;	
gg->TECH_DEVELOP = atof(optionsdata["TECH_DEVELOP"].c_str());
gg->CARBON_MIN = atof(optionsdata["CARBON_MIN"].c_str());
gg->CARBON_MAX = atof(optionsdata["CARBON_MAX"].c_str());

//====================================================
//New  15.03.2011
//====================================================
    gg->REGIONAL_DECOUPLING=atoi(optionsdata["REGIONAL_DECOUPLING"].c_str());
    gg->FULLY_DECOUPLING=atoi(optionsdata["FULLY_DECOUPLING"].c_str());
    gg->FARMSPECIFIC_DECOUPLING=atoi(optionsdata["FARMSPECIFIC_DECOUPLING"].c_str());
    gg->REGIONAL_DECOUPLING_SWITCH=atoi(optionsdata["REGIONAL_DECOUPLING_SWITCH"].c_str());
    gg->FULLY_DECOUPLING_SWITCH=atoi(optionsdata["FULLY_DECOUPLING_SWITCH"].c_str());
    gg->FARMSPECIFIC_DECOUPLING_SWITCH=atoi(optionsdata["FARMSPECIFIC_DECOUPLING_SWITCH"].c_str());
    gg->WITHDRAWFACTOR = atof(optionsdata["WITHDRAWFACTOR"].c_str());
    gg->AVERAGE_OFFER_BUFFER_SIZE=atoi(optionsdata["AVERAGE_OFFER_BUFFER_SIZE"].c_str());

    ///////// not yet ///////////////////
    //ADJUST_PAID_RENT=false;
    //OLD_LAND_RENTING_PROCESS=true;
    //FIRSTPRICE=true;
    //SECONDPRICE_REGION=false;
    ////////////////////////////

    gg->USE_TRIANGULAR_DISTRIBUTED_MANAGEMENT_FACTOR=optionsdata["USE_TRIANGULAR_DISTRIBUTED_MANAGEMENT_FACTOR"].compare("true")==0 ? true : false;
    gg->USE_TRIANGULAR_DISTRIBUTED_FARM_AGE=optionsdata["USE_TRIANGULAR_DISTRIBUTED_FARM_AGE"].compare("true")==0 ? true : false;

    /////////not yet ////////
    //FAST_PLOT_SEARCH=false;
    /////////////////////////

    gg->WEIGHTED_PLOT_SEARCH_VALUE=atoi(optionsdata["WEIGHTED_PLOT_SEARCH_VALUE"].c_str());

    /////////not yet /////////////////////
    //RELEASE_PLOTS_BEFORE_EXPECTATION_FORMATION=true;
    ///////////////////////////////////////

    gg->USE_VARIABLE_PRICE_CHANGE=optionsdata["USE_VARIABLE_PRICE_CHANGE"].compare("true")==0 ? true : false;

    gg->NO_SUCCESSOR_BY_RANDOM=optionsdata["NO_SUCCESSOR_BY_RANDOM"].compare("true")==0 ? true : false;
    gg->ASSOCIATE_ACTIVITIES=optionsdata["ASSOCIATE_ACTIVITIES"].compare("true")==0 ? true : false;
    gg->CALCULATE_CONTIGUOUS_PLOTS=optionsdata["CALCULATE_CONTIGUOUS_PLOTS"].compare("true")==0 ? true : false;
    gg->USE_HISTORICAL_CONTIGUOUS_PLOTS=optionsdata["USE_HISTORICAL_CONTIGUOUS_PLOTS"].compare("true")==0 ? true : false;

    //////////not yet ///////////
    //USE_TC_FRAMEWORK=false;//true;
    //PRINT_REMOVED_FARMS=true;
    //TRANCH_1_DEG=-1;
    //TRANCH_2_DEG=-1;
    //TRANCH_3_DEG=-1;
    //TRANCH_4_DEG=-1;
    //TRANCH_5_DEG=-1;
    //GLOBAL_STRATEGY=0;
    //GLOBAL_OPTIMUM_EVERY_PERIOD=false;
    ////////////////////////////////////

    gg->FIX_PRICES=optionsdata["FIX_PRICES"].compare("true")==0 ? true : false;

    ////////////////not yet /////////////
    //MIN_WITHDRAWAL=false;
    //SET_FREE_PLOTS=false;
    //CALCULATE_EXPECTED_RENTAL_PRICE=true;
    //for(int i=0;i<10;i++)
    //  FREE_PLOTS_OF_TYPE.push_back(0);
    /////////////////////////////////////

    gg->PRINT_SEC_RES=optionsdata["PRINT_SEC_RES"].compare("true")==0 ? true : false;
    gg->PRINT_SEC_PRICE=optionsdata["PRINT_SEC_PRICE"].compare("true")==0 ? true : false;
    gg->PRINT_SEC_EXP_PRICE=optionsdata["PRINT_SEC_EXP_PRICE"].compare("true")==0 ? true : false;
    gg->PRINT_SEC_COSTS=optionsdata["PRINT_SEC_COSTS"].compare("true")==0 ? true : false;
    gg->PRINT_SEC_COND=optionsdata["PRINT_SEC_COND"].compare("true")==0 ? true : false;
    gg->PRINT_FARM_RES=optionsdata["PRINT_FARM_RES"].compare("true")==0 ? true : false;
    gg->PRINT_FARM_INV=optionsdata["PRINT_FARM_INV"].compare("true")==0 ? true : false;
    gg->PRINT_FARM_PROD=optionsdata["PRINT_FARM_PROD"].compare("true")==0 ? true : false;
    gg->PRINT_FARM_COSTS=optionsdata["PRINT_FARM_COSTS"].compare("true")==0 ? true : false;
    gg->PRINT_CONT_PLOTS=optionsdata["PRINT_CONT_PLOTS"].compare("true")==0 ? true : false;

    gg->PRINT_TAC=optionsdata["PRINT_TAC"].compare("true")==0 ? true : false;
    gg->PRINT_VA=optionsdata["PRINT_VA"].compare("true")==0 ? true : false;
    gg->PRINT_POLICY=optionsdata["PRINT_POLICY"].compare("true")==0 ? true : false;

	//derived from replication
    //gg->INIT_OUTPUT=optionsdata["INIT_OUTPUT"].compare("true")==0 ?  true : false;
return;
}

void options(string idir){
	if (idir[idir.length()-1]!='\\' )
		optiondir = idir + '\\';
	else 
		optiondir= idir;
    readoptions();
    setoptions();
    return;
}


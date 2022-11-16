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

// RegGlobals.cpp
//---------------------------------------------------------------------------
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "RegGlobals.h"
#include "textinput.h"
#include "random.h"

RegGlobalsInfo::RegGlobalsInfo() {
//DCX
	V = 0;
	SCENARIO = 0;
    NUMBER_OF_INVESTTYPES= 0;
//DCX

	tech_develop_abs=1;   //

 STATUS=0;
    LEGAL_TYPES.push_back(1);
    LEGAL_TYPES.push_back(3);
    NAMES_OF_LEGAL_TYPES.push_back("IF");
    NAMES_OF_LEGAL_TYPES.push_back("CF");
    SEED=0;
    INITIALISATION=true;
    obj_backup=NULL;
    REGIONAL_DECOUPLING=0;
    FULLY_DECOUPLING=0;
    FARMSPECIFIC_DECOUPLING=0;
    REGIONAL_DECOUPLING_SWITCH=0;
    FULLY_DECOUPLING_SWITCH=0;
    FARMSPECIFIC_DECOUPLING_SWITCH=0;
    WITHDRAWFACTOR = 0.7;
    REGION_OVERSIZE=1.15;
    AVERAGE_OFFER_BUFFER_SIZE=5;
    MIN_CONTRACT_LENGTH=9;                        //Standard: 9, CZ=5, Saxony=12
    MAX_CONTRACT_LENGTH=18;                       //Standard: 18, Saxony=24
    SWEDEN=false;                                 //Standard: false, Sweden: true (calf and milk quota market)
    // flase means the settings for Västerbotton are used. THis is only relevant if Sweden is true.
    JOENKEPING=false;//true;
    ENV_MODELING=false;//true;                           //Standard: false, CZ, Västerbotten, Jönköping: true
    SOIL_TYPE_VARIATION=false;                    //true only for regions with various soil qualities of arable and/or grassland, like OPR.
                                                  //If true, then shares of soil types will be varried among the farms
    INTEGER_SWITCH=false;                          //In the region Ostprignitz-Ruppin (OPR) a switch is implemented in the MIP. This switch
                                                  //has to be integer, but normally only investment activities are integer. Thus, in sheet
                                                  //format in the input-file it is counted as investment activity. But investment activities
                                                  //are not considered when the production is calculated within AgriPoliS. Their upper bound
                                                  //is set to zero. Thus, in RegLPD the upper bound is set to zero only for columns > prodcols
                                                  //are set to zero and not the columns >= prodcols as usual. This is not a proper solution
                                                  //it is planned to extend AgrPoliS that each column can be defined as integer or continuous
                                                  //in the MIP-matrix
    ADJUST_PAID_RENT=false;
    OLD_LAND_RENTING_PROCESS=true;
    OLD_LAND_RELEASING_PROCESS=false;             //Standard: false, Brittany: true
    FIRSTPRICE=true;
    SECONDPRICE_REGION=false;
    USE_TRIANGULAR_DISTRIBUTED_MANAGEMENT_FACTOR=false;
    USE_TRIANGULAR_DISTRIBUTED_FARM_AGE=false;
    FAST_PLOT_SEARCH=false;
    WEIGHTED_PLOT_SEARCH=true;                        //standard: true, OPR false farm areas are initialised as a circle around the farm; they search for the plot with the lowest costs, however this only matters when transaction costs are considered
    WEIGHTED_PLOT_SEARCH_VALUE=50;
    RELEASE_PLOTS_BEFORE_EXPECTATION_FORMATION=true;
    USE_VARIABLE_PRICE_CHANGE=true;
    AGE_DEPENDENT=false;//true;                          //Standard: false, CZ, Lithuania, Slovakia: true
    NO_SUCCESSOR_BY_RANDOM=false;

    ASSOCIATE_ACTIVITIES=false;
    CALCULATE_CONTIGUOUS_PLOTS=false;
    USE_HISTORICAL_CONTIGUOUS_PLOTS=false;
    LP_MOD=true;//false;                                 //Standard: false, true is necessary for Saxony, Hohenlohe, Brittany and CZ because here we have different input files to calculate the modulation within the LP-model
    USE_TC_FRAMEWORK=false;//true;

    REGION_OVERSIZE=1.15;
    REGION_NON_AG_LAND=0;
    OUTPUTFILE="../outputfiles/";
    POLICYFILE="";
    PRINT_REMOVED_FARMS=true;
    TRANCH_1_DEG=-1;
    TRANCH_2_DEG=-1;
    TRANCH_3_DEG=-1;
    TRANCH_4_DEG=-1;
    TRANCH_5_DEG=-1;
    CALC_LEGAL_TYPES=true;                      //Attention, it is only differentiated between IF=1 and CF=3. In the input-files of Hohenlohe, Saxony, OPR and Brittany more legal types are defined. In Brittany and Saxony CF=2. To changes this search above for "Legal_types"
    GLOBAL_STRATEGY=0;
    GLOBAL_OPTIMUM_EVERY_PERIOD=false;
    FIX_PRICES=false;
    MIN_WITHDRAWAL=false;
    SET_FREE_PLOTS=false;
    CALCULATE_EXPECTED_RENTAL_PRICE=true;
    for(int i=0;i<10;i++)
      FREE_PLOTS_OF_TYPE.push_back(0);
  //    FREE_PLOTS_OF_TYPE[0]=20;
  //    FREE_PLOTS_OF_TYPE[1]=20;
    PRINT_SEC_RES=true;
    PRINT_SEC_PRICE=true;
    PRINT_SEC_EXP_PRICE=false;
    PRINT_SEC_COSTS=false;
    PRINT_SEC_COND=false;
    PRINT_FARM_RES=true;
    PRINT_FARM_INV=true;
    PRINT_FARM_PROD=true;
    PRINT_FARM_COSTS=false;
    PRINT_CONT_PLOTS=false;
    PRINT_TAC=false;
    PRINT_VA=false;
    PRINT_POLICY=false;
    INIT_OUTPUT=true;
}



// show the usage of this program
void ShowUsage() {
    _tprintf(_T("Usage: basicSample [-a] [-b] [-f FILE] [-?] [--help] FILES\n"));
}

// define the ID values to indentify the option
enum { OPT_HELP = 1000 };

// declare a table of CSimpleOpt::SOption structures. See the SimpleOpt.h header
// for details of each entry in this structure. In summary they are:
//  1. ID for this option. This will be returned from OptionId() during processing.
//     It may be anything >= 0 and may contain duplicates.
//  2. Option as it should be written on the command line
//  3. Type of the option. See the header file for details of all possible types.
//     The SO_REQ_SEP type means an argument is required and must be supplied
//     separately, e.g. "-f FILE"
//  4. The last entry must be SO_END_OF_OPTIONS.
//
CSimpleOpt::SOption g_rgOptions[] = {
                                        { 1,  _T("--FIXED_BONUS"),     SO_REQ_SEP },
                                        { 2,  _T("--FIXED_BONUS_VALUE"),     SO_REQ_SEP },
                                        { 3,  _T("--VARIABLE_BONUS"),     SO_REQ_SEP },
                                        { 4,  _T("--VARIABLE_BONUS_VALUE"),     SO_REQ_SEP },
                                        { 5,  _T("--PREV_OWNER_BONUS"),     SO_REQ_SEP },
                                        { 6,  _T("--LEGAL_TYPE_BONUS"),     SO_REQ_SEP },
                                        { 7,  _T("--ADJUST_PAID_RENT"),     SO_REQ_SEP },
                                        { 8,  _T("--SWEDEN"),     SO_REQ_SEP },
                                        { 9,  _T("--JOENKEPING"),     SO_REQ_SEP },
                                        { 10,  _T("--USE_TC_FRAMEWORK"),     SO_REQ_SEP },
                                        { 11,  _T("--ASSOCIATE_ACTIVITIES"),     SO_REQ_SEP },
                                        { 12,  _T("--PRINT_OUTPUT"),     SO_REQ_SEP },
                                        { 13,  _T("--USE_VARIABLE_PRICE_CHANGE"),     SO_REQ_SEP },
                                        { 14,  _T("--AGE_DEPENDENT"),     SO_REQ_SEP },
                                        { 15,  _T("--ENV_MODELING"),     SO_REQ_SEP },
                                        { 16,  _T("--WEIGHTED_PLOT_SEARCH"),     SO_REQ_SEP },
                                        { 17,  _T("--USE_TRIANGULAR_DISTRIBUTED_MANAGEMENT_FACTOR"),     SO_REQ_SEP },
                                        { 18,  _T("--USE_HISTORICAL_CONTIGUOUS_PLOTS"),     SO_REQ_SEP },
                                        { 19,  _T("--RELEASE_PLOTS_BEFORE_EXPECTATION_FORMATION"),     SO_REQ_SEP },
                                        { 20,  _T("--INPUTFILE"),     SO_REQ_SEP },
                                        { 21,  _T("--OUTPUTFILE"),     SO_REQ_SEP },
                                        { 22,  _T("--POLICYFILE"),     SO_REQ_SEP },
                                        { 23,  _T("--TC"),     SO_REQ_SEP },
                                        { 24,  _T("--MT"),     SO_REQ_SEP },
                                        { 25,  _T("--I"),     SO_REQ_SEP },
                                        { 26,  _T("--P"),     SO_REQ_SEP },
                                        { 27,  _T("--PC"),     SO_REQ_SEP },
                                        { 28,  _T("--V"),     SO_REQ_SEP },
                                        { 29,  _T("--INIT_OUTPUT"),     SO_REQ_SEP },
                                        { 30,  _T("--DEFAULT"),     SO_REQ_SEP },
                                        { 31,  _T("--SEED"),     SO_REQ_SEP },
                                        { 32,  _T("--SCENARIO"),     SO_REQ_SEP },
                                        { 33,  _T("--FIRSTPRICE"),     SO_REQ_SEP },
                                        { 34,  _T("--REGION_OVERSIZE"),     SO_REQ_SEP },
                                        { 35,  _T("--CALCULATE_CONTIGUOUS_PLOTS"),     SO_REQ_SEP },
                                        { 36,  _T("--REGION_NON_AG_LAND"),     SO_REQ_SEP },
                                        { 37,  _T("--TRANSPORT_COSTS"),     SO_REQ_SEP },
                                        { 38,  _T("--RUNS"),     SO_REQ_SEP },
                                        { 39,  _T("--OLD_LAND_RENTING_PROCESS"),     SO_REQ_SEP },
                                        { 40,  _T("--OLD_LAND_RELEASING_PROCESS"),     SO_REQ_SEP },
                                        { 41,  _T("--WITHDRAWFACTOR"),     SO_REQ_SEP },
                                        { 42,  _T("--LOWER_BORDER"),     SO_REQ_SEP },
                                        { 43,  _T("--UPPER_BORDER"),     SO_REQ_SEP },
                                        { 44,  _T("--WD_FACTOR"),     SO_REQ_SEP },
                                        { 45,  _T("--APPEARANCE"),     SO_REQ_SEP },
                                        { 46,  _T("--FARMOUTPUT"),     SO_REQ_SEP },
                                        { 47,  _T("--SECTOROUTPUT"),     SO_REQ_SEP },
                                        { 48,  _T("--MIN_CONTRACT_LENGTH"),     SO_REQ_SEP },
                                        { 49,  _T("--MAX_CONTRACT_LENGTH"),     SO_REQ_SEP },
                                        { 50,  _T("--PLOTSN"),     SO_REQ_SEP },
                                        { 51,  _T("--BONUS"),     SO_REQ_SEP },
                                        { 52,  _T("--PRODUCTGROUPS"),     SO_REQ_SEP },
                                        { 53,  _T("--PRODGROUPLAB"),     SO_REQ_SEP },
                                        { 54,  _T("--PRODTYPE"),     SO_REQ_SEP },
                                        { 55,  _T("--FAST_PLOT_SEARCH"),     SO_REQ_SEP },
                                        { 56,  _T("--NOTUSED"),     SO_REQ_SEP },
                                        { 57,  _T("--CALC_LEGAL_TYPES"),     SO_REQ_SEP },
                                        { 58,  _T("--DESIGN_POINT"),     SO_REQ_SEP },
                                        { 59,  _T("--RANDOM"),     SO_REQ_SEP },
                                        { 60,  _T("--GLOBAL_STRATEGY"),     SO_REQ_SEP },
                                        { 61,  _T("--PRINT_REMOVED_FARMS"),     SO_REQ_SEP },
                                        { 62,  _T("--SET_FREE_PLOTS=false"),     SO_REQ_SEP },
                                        { 63,  _T("--FREE_PLOTS_TYPE0"),     SO_REQ_SEP },
                                        { 64,  _T("--FREE_PLOTS_TYPE1"),     SO_REQ_SEP },
                                        { 65,  _T("--FREE_PLOTS_TYPE2"),     SO_REQ_SEP },
                                        { 66,  _T("--FREE_PLOTS_TYPE3"),     SO_REQ_SEP },
                                        { 67,  _T("--FREE_PLOTS_TYPE4"),     SO_REQ_SEP },
                                        { 68,  _T("--FREE_PLOTS_TYPE5"),     SO_REQ_SEP },
                                        { 69,  _T("--FREE_PLOTS_TYPE6"),     SO_REQ_SEP },
                                        { 70,  _T("--FREE_PLOTS_TYPE7"),     SO_REQ_SEP },
                                        { 71,  _T("--FREE_PLOTS_TYPE8"),     SO_REQ_SEP },
                                        { 72,  _T("--FREE_PLOTS_TYPE9"),     SO_REQ_SEP },
                                        { 73,  _T("SECONDPRICE_REGION"),     SO_REQ_SEP },
                                        { 74,  _T("GLOBAL_OPTIMUM_EVERY_PERIOD"),     SO_REQ_SEP },
                                        { 75,  _T("--USE_TRIANGULAR_DISTRIBUTED_FARM_AGE"),     SO_REQ_SEP },
                                        { 76,  _T("--NO_SUCCESSOR_BY_RANDOM"),     SO_REQ_SEP },
                                        { 77,  _T("--CALCULATE_EXPECTED_RENTAL_PRICE"),     SO_REQ_SEP },
                                        { 78,  _T("--PRINT_SEC_RES"),     SO_REQ_SEP },
                                        { 79,  _T("--PRINT_SEC_PRICE"),     SO_REQ_SEP },
                                        { 80,  _T("--PRINT_SEC_EXP_PRICE"),     SO_REQ_SEP },
                                        { 81,  _T("--PRINT_SEC_COSTS"),     SO_REQ_SEP },
                                        { 82,  _T("--PRINT_SEC_COND"),     SO_REQ_SEP },
                                        { 83,  _T("--PRINT_FARM_RES"),     SO_REQ_SEP },
                                        { 84,  _T("--PRINT_FARM_INV"),     SO_REQ_SEP },
                                        { 85,  _T("--PRINT_FARM_PROD"),     SO_REQ_SEP },
                                        { 86,  _T("--PRINT_FARM_COSTS"),     SO_REQ_SEP },
                                        { 87,  _T("--PRINT_CONT_PLOTS"),     SO_REQ_SEP },
                                        { 88,  _T("--PRINT_TAC"),     SO_REQ_SEP },
                                        { 89,  _T("--PRINT_VA"),     SO_REQ_SEP },
                                        { 90,  _T("--PRINT_POLICY"),     SO_REQ_SEP },
                                        { 91,  _T("--LP_MOD"),     SO_REQ_SEP },
                                        { 92,  _T("--SOIL_TYPE_VARIATION"),     SO_REQ_SEP },
                                        { 93,  _T("--INTEGER_SWITCH"),     SO_REQ_SEP },

                                        SO_END_OF_OPTIONS                       // END
                                    };



RegGlobalsInfo::~RegGlobalsInfo() {
    if (obj_backup) delete obj_backup;
}


void
RegGlobalsInfo::readFromCommandLine() {
   /* CSimpleOpt args(ARGC,ARGV, g_rgOptions);
    while (args.Next()) {
        if (args.LastError() != SO_SUCCESS) {
            TCHAR * pszError = _T("Unknown error");
            switch (args.LastError()) {
            case SO_OPT_INVALID:
                pszError = _T("Unrecognized option");
                break;
            case SO_OPT_MULTIPLE:
                pszError = _T("Option matched multiple strings");
                break;
            case SO_ARG_INVALID:
                pszError = _T("Option does not accept argument");
                break;
            case SO_ARG_INVALID_TYPE:
                pszError = _T("Invalid argument format");
                break;
            case SO_ARG_MISSING:
                pszError = _T("Required argument is missing");
                break;
            case SO_SUCCESS:
                pszError = NULL;
            }
            _tprintf(
                _T("%s: '%s' (use --help to get command line help)\n"),
                pszError, args.OptionText());
            continue;
        }

        if (args.OptionId() == OPT_HELP) {
            ShowUsage();
            exit(0);
        }

        switch (args.OptionId()) {
        case 1:
            this->FIXED_BONUS=atoi(args.OptionArg());
            break;
        case 2:
            this->FIXED_BONUS_VALUE=atof(args.OptionArg());
            break;
        case 3:
            this->VARIABLE_BONUS=atoi(args.OptionArg());
            break;
        case 4:
            this->VARIABLE_BONUS_VALUE=atof(args.OptionArg());
            break;
        case 5:
            this->PREV_OWNER_BONUS=atoi(args.OptionArg());
            break;
        case 6:
            this->LEGAL_TYPE_BONUS=atoi(args.OptionArg());
            break;
        case 7:
            this->ADJUST_PAID_RENT=atoi(args.OptionArg());
            break;
        case 8:
            this->SWEDEN=atoi(args.OptionArg());
            break;
        case 9:
            this->JOENKEPING=atoi(args.OptionArg());
            break;
        case 10:
            this->USE_TC_FRAMEWORK=atoi(args.OptionArg());
            break;
        case 11:
            this->ASSOCIATE_ACTIVITIES=atoi(args.OptionArg());
            break;
        case 13:
            this->USE_VARIABLE_PRICE_CHANGE=atoi(args.OptionArg());
            break;
        case 14:
            this->AGE_DEPENDENT=atoi(args.OptionArg());
            break;
        case 15:
            this->ENV_MODELING=atoi(args.OptionArg());
            break;
        case 16:
            this->WEIGHTED_PLOT_SEARCH=atoi(args.OptionArg());
            break;
        case 17:
            this->USE_TRIANGULAR_DISTRIBUTED_MANAGEMENT_FACTOR=atoi(args.OptionArg());
            break;
        case 18:
            this->USE_HISTORICAL_CONTIGUOUS_PLOTS=atoi(args.OptionArg());
            break;
        case 19:
            this->RELEASE_PLOTS_BEFORE_EXPECTATION_FORMATION=atoi(args.OptionArg());
            break;
        case 20:
            this->INPUTFILE=string(args.OptionArg());
            break;
        case 21:
            this->OUTPUTFILE=string(args.OptionArg());
            break;
        case 22:
            this->POLICYFILE=string(args.OptionArg());
            break;
        case 23:
            this->TC=atoi(args.OptionArg());
            break;
        case 24:
            this->MT=atoi(args.OptionArg());
            break;
        case 25:
            this->I=atoi(args.OptionArg());
            break;
        case 26:
            this->P=atoi(args.OptionArg());
            break;
        case 27:
            this->PC=atoi(args.OptionArg());
            break;
        case 28:
            this->V=atoi(args.OptionArg());
            break;
        case 29:
            this->INIT_OUTPUT=atoi(args.OptionArg());
            break;
        case 30:
            this->DEFAULT=atoi(args.OptionArg());
            break;
        case 31:
            this->SEED=atoi(args.OptionArg());
            break;
        case 32:
            this->SCENARIO=atoi(args.OptionArg());
            break;
        case 33:
            this->FIRSTPRICE=atoi(args.OptionArg());
            break;
        case 34:
            this->REGION_OVERSIZE=atoi(args.OptionArg());
            break;
        case 35:
            this->CALCULATE_CONTIGUOUS_PLOTS=atoi(args.OptionArg());
            break;
        case 36:
            this->REGION_NON_AG_LAND=atoi(args.OptionArg());
            break;
        case 37:
            this->TRANSPORT_COSTS=atoi(args.OptionArg());
            break;
        case 38:
            this->RUNS=atoi(args.OptionArg());
            break;
        case 39:
            this->OLD_LAND_RENTING_PROCESS=atoi(args.OptionArg());
            break;
        case 40:
            this->OLD_LAND_RELEASING_PROCESS=atoi(args.OptionArg());
            break;
        case 41:
            this->WITHDRAWFACTOR=atoi(args.OptionArg());
            break;
        case 42:
            this->LOWER_BORDER=atoi(args.OptionArg());
            break;
        case 43:
            this->UPPER_BORDER=atoi(args.OptionArg());
            break;
        case 44:
            this->WD_FACTOR=atoi(args.OptionArg());
            break;
        case 45:
            this->APPEARANCE=atoi(args.OptionArg());
            break;
        case 46:
            this->FARMOUTPUT=atoi(args.OptionArg());
            break;
        case 47:
            this->SECTOROUTPUT=atoi(args.OptionArg());
            break;
        case 48:
            this->MIN_CONTRACT_LENGTH=atoi(args.OptionArg());
            break;
        case 49:
            this->MAX_CONTRACT_LENGTH=atoi(args.OptionArg());
            break;
        case 50:
            this->PLOTSN=atoi(args.OptionArg());
            break;
        case 51:
            this->BONUS=atoi(args.OptionArg());
            break;
        case 52:
            this->PRODUCTGROUPS=atoi(args.OptionArg());
            break;
        case 53:
            this->PRODGROUPLAB=atoi(args.OptionArg());
            break;
        case 54:
            this->PRODTYPE=atoi(args.OptionArg());
            break;
        case 55:
            this->FAST_PLOT_SEARCH=atoi(args.OptionArg());
            break;
        case 56:
            break;
        case 57:
            this->CALC_LEGAL_TYPES=atoi(args.OptionArg());
            break;
        case 58:
            this->DESIGN_POINT=atoi(args.OptionArg());
            break;
        case 59:
            this->RANDOM=atoi(args.OptionArg());
            break;
        case 60:
            this->GLOBAL_STRATEGY=atoi(args.OptionArg());
            break;
        case 61:
            this->PRINT_REMOVED_FARMS=atoi(args.OptionArg());
            break;
        case 62:
            this->SET_FREE_PLOTS=atoi(args.OptionArg());
            break;
        case 63:
            this->FREE_PLOTS_OF_TYPE[0]=atoi(args.OptionArg());
            break;
        case 64:
            this->FREE_PLOTS_OF_TYPE[1]=atoi(args.OptionArg());
            break;
        case 65:
            this->FREE_PLOTS_OF_TYPE[2]=atoi(args.OptionArg());
            break;
        case 66:
            this->FREE_PLOTS_OF_TYPE[3]=atoi(args.OptionArg());
            break;
        case 67:
            this->FREE_PLOTS_OF_TYPE[4]=atoi(args.OptionArg());
            break;
        case 68:
            this->FREE_PLOTS_OF_TYPE[5]=atoi(args.OptionArg());
            break;
        case 69:
            this->FREE_PLOTS_OF_TYPE[6]=atoi(args.OptionArg());
            break;
        case 70:
            this->FREE_PLOTS_OF_TYPE[7]=atoi(args.OptionArg());
            break;
        case 71:
            this->FREE_PLOTS_OF_TYPE[8]=atoi(args.OptionArg());
            break;
        case 72:
            this->FREE_PLOTS_OF_TYPE[9]=atoi(args.OptionArg());
            break;
        case 73:
            this->SECONDPRICE_REGION=atoi(args.OptionArg());
            break;
        case 74:
            this->GLOBAL_OPTIMUM_EVERY_PERIOD=atoi(args.OptionArg());
            break;
        case 75:
            this->USE_TRIANGULAR_DISTRIBUTED_FARM_AGE=atoi(args.OptionArg());
            break;
        case 76:
            this->NO_SUCCESSOR_BY_RANDOM=atoi(args.OptionArg());
            break;
        case 77:
            this->CALCULATE_EXPECTED_RENTAL_PRICE=atoi(args.OptionArg());
            break;
        case 78:
            this->PRINT_SEC_RES=atoi(args.OptionArg());
            break;
        case 79:
            this->PRINT_SEC_PRICE=atoi(args.OptionArg());
            break;
        case 80:
            this->PRINT_SEC_EXP_PRICE=atoi(args.OptionArg());
            break;
        case 81:
            this->PRINT_SEC_COSTS=atoi(args.OptionArg());
            break;
        case 82:
            this->PRINT_SEC_COND=atoi(args.OptionArg());
            break;
        case 83:
            this->PRINT_FARM_RES=atoi(args.OptionArg());
            break;
        case 84:
            this->PRINT_FARM_INV=atoi(args.OptionArg());
            break;
        case 85:
            this->PRINT_FARM_PROD=atoi(args.OptionArg());
            break;
        case 86:
            this->PRINT_FARM_COSTS=atoi(args.OptionArg());
            break;
        case 87:
            this->PRINT_CONT_PLOTS=atoi(args.OptionArg());
            break;
        case 88:
            this->PRINT_TAC=atoi(args.OptionArg());
            break;
        case 89:
            this->PRINT_VA=atoi(args.OptionArg());
            break;
        case 90:
            this->PRINT_POLICY=atoi(args.OptionArg());
            break;
        case 91:
            this->LP_MOD=atoi(args.OptionArg());
            break; 
        case 92:
            this->SOIL_TYPE_VARIATION=atoi(args.OptionArg());
            break;
        case 93:
            this->INTEGER_SWITCH=atoi(args.OptionArg());
            break;

        default:
            break;
        }
        _tprintf(
            _T("%s: '%s' \n"),
            args.OptionText(), args.OptionArg());
    }
//*/
}

void
RegGlobalsInfo::initGlobals() {
//DCX
tIter=-1;
tFarmId=-1;
tFarmName="---";
tInd=-1;

    int noft= number_of_farmtypes = farmsdata.numOfFarms;

    for (int i=0; i<noft; i++){
		int t ;
       ef.push_back(t= farmsdata.formsOrgs[i]);
	}

    for (int i=0;i<noft;i++)
       farm_class.push_back(farmsdata.farm_types[i]);

    for (int i=0;i<noft;i++) {
        int e=farmsdata.weightFacs[i];
        number_of_each_type.push_back(e);
    }

     for (int i=0;i<noft;i++) {
        sheetnames.push_back(farmsdata.names[i]);
    }

    TEILER=atoi(globdata.globs["TEILER"].c_str());
    REGION_OVERSIZE= atof(globdata.globs["OVERSIZE"].c_str());
    REGION_NON_AG_LAND=atof(globdata.globs["NON_AG_LAND"].c_str());
    for (int i=0;i<noft;i++) {
        int n=number_of_each_type[i];
        int number = (int)((double)n/(int)TEILER+0.5);
        number_of_each_type[i]=number;
    }

    // GLOBALS
    PLOTSN=             atoi(globdata.globs["PLOTSN"].c_str());
    PIGLETS_PER_SOW =   atof(globdata.globs["PIGLETS_PER_SOW"].c_str());
    PIGS_PER_PLACE   =   atof(globdata.globs["PIGS_PER_PLACE"].c_str());
    MILKPROD        =   atof(globdata.globs["MILKPROD"].c_str());
    ESU              =   atof(globdata.globs["ESU"].c_str());
    INCREASEPRICE    =   atof(globdata.globs["INCREASEPRICE"].c_str());
    BONUS            =   atof(globdata.globs["BONUS"].c_str());
    REFINCOME         =   atof(globdata.globs["REFINCOME"].c_str());
    PRODUCTGROUPS     =   atoi(globdata.globs["PRODUCTGROUPS"].c_str());
    PRODGROUPLAB      =   atoi(globdata.globs["PRODGROUPLAB"].c_str());
    PRODTYPE          =   atoi(globdata.globs["PRODTYPE"].c_str());
    CHANGEPERHA      =   atof(globdata.globs["CHANGEPERHA"].c_str());
    CHANGEUP         =   atof(globdata.globs["CHANGEUP"].c_str());
    CHANGEDOWN       =   atof(globdata.globs["CHANGEDOWN"].c_str());

    MILKUPPERLIMIT   =   atof(globdata.globs["MILKUPPERLIMIT"].c_str());
    MILKLOWERLIMIT   =   atof(globdata.globs["MILKLOWERLIMIT"].c_str());

    OFFFARMLABTYPE     =   atoi(globdata.globs["OFFFARMLABTYPE"].c_str());
    VAROFFARMLABTYPE   =   atoi(globdata.globs["VAROFFFARMLABTYPE"].c_str());
    VARHIREDLABTYPE    =   atoi(globdata.globs["VARHIREDLABTYPE"].c_str());
    ST_BOR_INTERESTTYPE  =   atoi(globdata.globs["ST_BOR_INTERESTTYPE"].c_str());
    ST_EC_INTERESTTYPE =   atoi(globdata.globs["ST_EC_INTERESTTYPE"].c_str());

    //================
    sFATTENINGPIGS =   "FATTENINGPIGS";
    sSERVICES =   "SERVICES";
    sSOW = "SOW";

    sMILK =   "MILK";
    sMANSELL =   "MANSELL";
    sMANBUY =   "MANBUY";
    sGETQUOTA =   "GETQUOTA";
    sLETQUOTA =   "LETQUOTA";

//DCX
	sBUYCALF_SUCK = "BUYCALF_SUCK";
	sSELLCALF_SUCK = "SELLCALF_SUCK" ;
	sBUYCALF_DAIRY  = "BUYCALF_DAIRY";
	sSELLCALF_DAIRY  = "SELLCALF_DAIRY";
//

 if(LP_MOD) {
    sCOUPLED_PREM_UNMOD =   "COUPLED_PREM_UNMOD";
    sDECOUPLED_PREM_UNMOD =   "DECOUPLED_PREM_UNMOD";
    sTOTAL_PREM_MODULATED =   "TOTAL_PREM_MODULATED";
    sTRANCH_1 =   "TRANCH_1";
    sTRANCH_2 =   "TRANCH_2";
    sTRANCH_3 =   "TRANCH_3";
    sTRANCH_4 =   "TRANCH_4";
    sTRANCH_5 =   "TRANCH_5";
} else {
    sPREMIUM =   "PREMIUM";
    sTOTAL_PREMIUM =   "TOTAL_PREMIUM";
}

    sIDLE_ARABLE =   "IDLE_ARABLE";
    sIDLE_ARABLE2 =   "IDLE_ARABLE2";
    sIDLE_GRASS =   "IDLE_GRASS";
    sEXCESS_LU =   "EXCESS_LU";
    sLU_UPPER_LIMIT =   "LU_UPPER_LIMIT";
    sFIXED_OFFFARM_LAB =   "FIXED_OFFFARM_LAB";
    sFIXED_HIRED_LAB =   "FIXED_HIRED_LAB";

    //===========

    if (transdata.trans.find(sFATTENINGPIGS)!=transdata.trans.end())
        sFATTENINGPIGS = transdata.trans["FATTENINGPIGS"].c_str();

    if (transdata.trans.find(sSERVICES)!=transdata.trans.end())
        sSERVICES =   transdata.trans["SERVICES"].c_str();

    if (transdata.trans.find(sSOW)!=transdata.trans.end())
        sSOW = transdata.trans["SOW"].c_str();

    if (transdata.trans.find(sMILK)!=transdata.trans.end())
        sMILK =   transdata.trans["MILK"].c_str();

    if (transdata.trans.find(sMANSELL)!=transdata.trans.end())
        sMANSELL =   transdata.trans["MANSELL"].c_str();

    if (transdata.trans.find(sMANBUY)!=transdata.trans.end())
        sMANBUY =   transdata.trans["MANBUY"].c_str();

    if (transdata.trans.find(sGETQUOTA)!=transdata.trans.end())
        sGETQUOTA =   transdata.trans["GETQUOTA"].c_str();

    if (transdata.trans.find(sLETQUOTA)!=transdata.trans.end())
        sLETQUOTA =   transdata.trans["LETQUOTA"].c_str();

//new
	if (transdata.trans.find(sBUYCALF_SUCK)!=transdata.trans.end())
        sBUYCALF_SUCK =   transdata.trans["BUYCALF_SUCK"].c_str();

    if (transdata.trans.find(sSELLCALF_SUCK)!=transdata.trans.end())
        sSELLCALF_SUCK =   transdata.trans["SELLCALF_SUCK"].c_str();

    if (transdata.trans.find(sBUYCALF_DAIRY)!=transdata.trans.end())
        sBUYCALF_DAIRY =   transdata.trans["BUYCALF_DAIRY"].c_str();

    if (transdata.trans.find(sSELLCALF_DAIRY)!=transdata.trans.end())
       sSELLCALF_DAIRY =   transdata.trans["SELLCALF_DAIRY"].c_str();

 if(LP_MOD) {
     if (transdata.trans.find(sCOUPLED_PREM_UNMOD)!=transdata.trans.end())
        sCOUPLED_PREM_UNMOD =   transdata.trans["COUPLED_PREM_UNMOD"].c_str();

    if (transdata.trans.find(sDECOUPLED_PREM_UNMOD)!=transdata.trans.end())
        sDECOUPLED_PREM_UNMOD =   transdata.trans["DECOUPLED_PREM_UNMOD"].c_str();
    if (transdata.trans.find(sTOTAL_PREM_MODULATED)!=transdata.trans.end())
        sTOTAL_PREM_MODULATED =   transdata.trans["TOTAL_PREM_MODULATED"].c_str();
    if (transdata.trans.find(sTRANCH_1)!=transdata.trans.end())
        sTRANCH_1 =   transdata.trans["TRANCH_1"].c_str();
    if (transdata.trans.find(sTRANCH_2)!=transdata.trans.end())
        sTRANCH_2 =   transdata.trans["TRANCH_2"].c_str();
    if (transdata.trans.find(sTRANCH_3)!=transdata.trans.end())
        sTRANCH_3 =   transdata.trans["TRANCH_3"].c_str();
    if (transdata.trans.find(sTRANCH_4)!=transdata.trans.end())
        sTRANCH_4 =   transdata.trans["TRANCH_4"].c_str();
    if (transdata.trans.find(sTRANCH_5)!=transdata.trans.end())
        sTRANCH_5 =   transdata.trans["TRANCH_5"].c_str();
} else {
    if (transdata.trans.find(sPREMIUM)!=transdata.trans.end())
        sPREMIUM =   transdata.trans["PREMIUM"].c_str();
    if (transdata.trans.find(sTOTAL_PREMIUM)!=transdata.trans.end())
        sTOTAL_PREMIUM =   transdata.trans["TOTAL_PREMIUM"].c_str();
}
    if (transdata.trans.find(sIDLE_ARABLE)!=transdata.trans.end())
        sIDLE_ARABLE =   transdata.trans["IDLE_ARABLE"].c_str();
    if (transdata.trans.find(sIDLE_ARABLE2)!=transdata.trans.end())
        sIDLE_ARABLE2 =   transdata.trans["IDLE_ARABLE2"].c_str();
    if (transdata.trans.find(sIDLE_GRASS)!=transdata.trans.end())
        sIDLE_GRASS =   transdata.trans["IDLE_GRASS"].c_str();
    if (transdata.trans.find(sEXCESS_LU)!=transdata.trans.end())
        sEXCESS_LU =   transdata.trans["EXCESS_LU"].c_str();
    if (transdata.trans.find(sLU_UPPER_LIMIT)!=transdata.trans.end())
        sLU_UPPER_LIMIT =   transdata.trans["LU_UPPER_LIMIT"].c_str();
    if (transdata.trans.find(sFIXED_OFFFARM_LAB)!=transdata.trans.end())
        sFIXED_OFFFARM_LAB =   transdata.trans["FIXED_OFFFARM_LAB"].c_str();
    if (transdata.trans.find(sFIXED_HIRED_LAB)!=transdata.trans.end())
        sFIXED_HIRED_LAB =   transdata.trans["FIXED_HIRED_LAB"].c_str();

//======================================

    PLOT_SIZE   =   atof(globdata.globs["PLOTSIZE"].c_str());
    MAX_H_LU   =   atoi(globdata.globs["LABOUR_HOURS_PER_UNIT"].c_str());
    INTEREST_RATE   =   atof(globdata.globs["INTEREST"].c_str());
    REGION_MILK_QUOTA   =   atof(globdata.globs["REGIONAL_MILK_QUOTA"].c_str());
    OVERHEADS  =   atof(globdata.globs["OVERHEADS"].c_str());
    LOWER_BORDER   =   atof(globdata.globs["LOWER_BORDER"].c_str());
    UPPER_BORDER  =   atof(globdata.globs["UPPER_BORDER"].c_str());

    SHARE_SELF_FINANCE   =   atof(globdata.globs["SELF_FINANCE_SHARE"].c_str());
    TRANSPORT_COSTS   =   atoi(globdata.globs["TRANSPORT_COSTS"].c_str());
    WD_FACTOR   =   atoi(globdata.globs["CAPITAL_WITHDRAW_FACTOR"].c_str());
    GENERATION_CHANGE  =   atoi(globdata.globs["GENERATION_CHANGE"].c_str());
    INVEST_GROUPS   =   atoi(globdata.globs["INVEST_GROUPS"].c_str());
    RENT_ADJUST_COEFFICIENT_N   =   atoi(globdata.globs["RENT_ADJUST_NEIGHBOURS"].c_str());
    RENT_ADJUST_COEFFICIENT   =   atof(globdata.globs["RENT_ADJUST_FACTOR"].c_str());
    TC_MACHINERY  =   atoi(globdata.globs["TC_MACHINERY"].c_str());
    SIM_VERSION   =   atoi(globdata.globs["SIMVERSION"].c_str());


 if(LP_MOD) {
    TRANCH_1_WIDTH = atof(globdata.globs["TRANCH_1_WIDTH"].c_str());
    TRANCH_2_WIDTH =  atof(globdata.globs["TRANCH_2_WIDTH"].c_str());
    TRANCH_3_WIDTH =  atof(globdata.globs["TRANCH_3_WIDTH"].c_str());
    TRANCH_4_WIDTH =  atof(globdata.globs["TRANCH_4_WIDTH"].c_str());
    TRANCH_5_WIDTH =  atof(globdata.globs["TRANCH_5_WIDTH"].c_str());
} else {
    LB_LOW_TRANCH =  atof(globdata.globs["LOWER_BOUND_LOW_TRANCH"].c_str());
    UB_LOW_TRANCH =  atof(globdata.globs["UPPER_BOUND_LOW_TRANCH"].c_str());
    LB_MIDDLE_TRANCH =  atof(globdata.globs["LOWER_BOUND_MIDDLE_TRANCH"].c_str());
    UB_MIDDLE_TRANCH =  atof(globdata.globs["UPPER_BOUND_MIDDLE_TRANCH"].c_str());
    LB_HIGH_TRANCH =  atof(globdata.globs["LOWER_BOUND_HIGH_TRANCH"].c_str());
    UB_HIGH_TRANCH = atof(globdata.globs["UPPER_BOUND_HIGH_TRANCH"].c_str());
}

    FIX_REFERENCE_PERIOD =  atoi(globdata.globs["FIX_REFERENCE_PERIOD"].c_str());
    if (USE_TC_FRAMEWORK) {
        LEGAL_TYPE_BONUS= atoi(globdata.globs["LEGAL_TYPE_BONUS"].c_str())?true:false;
        PREV_OWNER_BONUS= atoi(globdata.globs["PREV_OWNER_BONUS"].c_str())?true:false;
        FIXED_BONUS= atoi(globdata.globs["FIXED_BONUS"].c_str())?true:false;
        if (FIXED_BONUS)
            FIXED_BONUS_VALUE= atoi(globdata.globs["FIXED_BONUS_VALUE"].c_str());
        VARIABLE_BONUS= atoi(globdata.globs["VARIABLE_BONUS"].c_str())?true:false;
        if (VARIABLE_BONUS)
            VARIABLE_BONUS_VALUE= atoi(globdata.globs["VARIABLE_BONUS_VALUE"].c_str());
        MIN_CONTRACT_LENGTH= atoi(globdata.globs["MIN_CONTRACT_LENGTH"].c_str());
        MAX_CONTRACT_LENGTH= atoi(globdata.globs["MAX_CONTRACT_LENGTH"].c_str());
    }

    NO_OF_SOIL_TYPES =  atoi(globdata.globs["NUMBER_OF_SOIL_TYPES"].c_str());
    for (int i=0;i<NO_OF_SOIL_TYPES;i++) {
        string s = globdata.namesOfSoilTypes[i];
        NAMES_OF_SOIL_TYPES.push_back(s);
    }

    // Berechnen des Flaechenbedarfs der Region
    for (int i=0;i<NO_OF_SOIL_TYPES;i++) {
        LAND_INPUT_OF_TYPE.push_back(0);
    }

    for (int i=0;i<noft;i++) {
        for (int k=0;k<NO_OF_SOIL_TYPES;k++) {
            LAND_INPUT_OF_TYPE[k] +=(farmsdata.alllands[globdata.namesOfSoilTypes[k]].owned_land[i])
                            *number_of_each_type[i];
            LAND_INPUT_OF_TYPE[k] += (farmsdata.alllands[globdata.namesOfSoilTypes[k]].rented_land[i])
                            *number_of_each_type[i];
         }
    }

    double landinput=0;
    for (int i=0;i<NO_OF_SOIL_TYPES;i++) {
        landinput+=LAND_INPUT_OF_TYPE[i];
    }
    NON_AG_LANDINPUT=landinput*REGION_NON_AG_LAND;
    landinput+=NON_AG_LANDINPUT;
    landinput*=REGION_OVERSIZE;
    int cols = static_cast<int>( sqrt(landinput/PLOT_SIZE));
    while ((cols*cols*PLOT_SIZE) < landinput) {
        cols++;
    }
    NO_ROWS=cols;
    NO_COLS=cols;
    // total region
    VISION=NO_COLS/2+1;

	//soil service
	if (HAS_SOILSERVICE) regCarbons.resize(NO_OF_SOIL_TYPES);

}    // annuityFactor


double triangular(double min, double ml, double max) {
    double r=randlong()/(double)mtRandMax;//RAND_MAX;
    if (r <= (ml - min) / (max - min)) {
        return min + sqrt(r * (ml - min) * (max - min));
    } else {
        return max - sqrt((1 - r) * (max - ml) * (max - min));
    }
}
double capitalReturnFactor(double p, int t) {
    double q = 1 + p;
    double crf = 0;

    if (q <= 1) {
        crf = 1 / ((double) t);
    } else {
        crf = pow(q,t) * (q - 1) / ( pow(q,t) - 1);
    }
    return crf;
}
// average return

double averageReturn(double p, int t) {
    double q = 1 + p;
    double ar = 0;

    ar = ( ( pow(q,t) / ( pow(q,t)-1) ) - ( 1 / (t*(q-1)) ) );
    return ar;
}

//#undef rand()
//int randown() {
//   ofstream out;
//   out.open("rand.dat", ios::app);
//   int r=std::rand();
//   out << r << "\n";
//   out.close();
//   return r;
//}
//#define rand() randown()
void
RegGlobalsInfo::backup() {
    obj_backup=clone();
}
void
RegGlobalsInfo::restore() {
    RegGlobalsInfo* tmp=obj_backup;
    *this=*obj_backup;
    obj_backup=tmp;
}
RegGlobalsInfo* RegGlobalsInfo::clone() {
    return new RegGlobalsInfo(*this);
}
RegGlobalsInfo* RegGlobalsInfo::create() {
    return new RegGlobalsInfo();
}


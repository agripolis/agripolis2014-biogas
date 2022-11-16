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

// RegProduct.cpp
//---------------------------------------------------------------------------
#include <iostream>
#include <sstream>
#include "RegProduct.h"

//---------------------------------------------------------------------------
// RegProduct methods
//---------------------------------------------------------------------------
//soil service
void RegProductInfo::setSoilservice(double aa, double bb, double cc, double dd, double gam, int soil, bool hasSoil, bool dyn, 
	       double pp, double kk, double pest, double energy){
	a=aa;
	b=bb;
	c=cc;
	d=dd;
	gamma=gam;
	soiltype=soil;
	hasSoilservice = hasSoil;
	dynSoilservice = dyn;

	//neu Gss
	p=pp; k=kk; pesticide=pest; energyvar= energy;

	return;
}

void RegProductInfo::setSoilservice(bool hasSoil){
	hasSoilservice = hasSoil;
	dynSoilservice = hasSoil;
	return;
}

vector<double> RegProductInfo::getSScoeffs(){
	vector<double> temp;
	temp.push_back(a);
	temp.push_back(b);
	temp.push_back(c);
	temp.push_back(d);

	temp.push_back(p);
	temp.push_back(k);
	temp.push_back(pesticide);
	temp.push_back(energyvar);
	return temp;
}

void
RegProductInfo::setAttrib(int cn, int pt, int pg, string n,string c) {
    catalog_number = cn;
    product_type = pt;
    product_group = pg;
    name = n;
    cl=c;
    reference_premium=0;
    reference_premium_calc_time=0;
}
//20060426

void
RegProductInfo::setEnvAttrib( double N_h, double P2O5_h, double K2O_h,
                              double fung_h, double herb_h, double insect_h,
                              double wusage_h, double SLossCoeff_h) {
    N_usage = N_h;
    P2O5_usage = P2O5_h;
    K2O_usage = K2O_h;
    Fungicides_usage = fung_h;
    Herbicides_usage = herb_h;
    Insecticides_usage = insect_h;
    Water_usage = wusage_h;
    SLossCoeff = SLossCoeff_h;
}


string
RegProductInfo::debug() {
    stringstream r;
    r << "Name:\t" << name << "\t" <<
                   "Catalog Number:\t" << catalog_number << "\t" <<
                   "Type:\t" << product_type << "\t" <<
                   "Price:\t" << price << "\t" <<
                   "Price Expectation:\t" << price_expectation << "\t" <<
                   "Original Costs:\t" << var_cost << "\t";
    return r.str();
}

//======================================RegProductList =============================

//soil service
// these two product names should be better in yield.txt file
const string priceProduct = "GRAIN_SELL";
const string varcostProduct = "N_FERT_BUY"; //in market the negative price value


double RegProductList::getNoptOfNumber(int i){
	double w, p;
	for (unsigned t=0; t<(*products).size(); ++t){
		if ((*products)[t].getName()==priceProduct){ 
			p=(*products)[t].getPrice();
		}
		if ((*products)[t].getName()==varcostProduct) {
			w=(-1)*(*products)[t].getPrice(); //VarCost();
		}

	}
	double b=(*products)[i].getSScoeffs()[1];
	double c=(*products)[i].getSScoeffs()[2];
	
	return (-1)*0.5*(b-w/p/g->tech_develop_abs)/c;
}


double RegProductList::getPOfNumber(int i){
	return (*products)[i].getSScoeffs()[4];
}

double RegProductList::getKOfNumber(int i){
	return (*products)[i].getSScoeffs()[5];
}

double RegProductList::getPesticideOfNumber(int i){
	return (*products)[i].getSScoeffs()[6];
}

double RegProductList::getEnergyvarOfNumber(int i){
	return (*products)[i].getSScoeffs()[7];
}

double RegProductList::getYieldOfNumber(int i, RegFarmInfo* s){
	double N=getNoptOfNumber(i);
	int soiltype= (*products)[i].getProdSoilType();
	vector<double> coeffs = (*products)[i].getSScoeffs();
	double carbon = s ->getAvCarbons()[soiltype];
	return (coeffs[0]+coeffs[1]*N + coeffs[2]*N*N+coeffs[3]*carbon)*g->tech_develop_abs;
}

//constructor
RegProductList::RegProductList(RegGlobalsInfo* G,vector<RegProductInfo>& P)
        :g(G),products(&P) {
    obj_backup=NULL;
    var_costs.resize((*products).size());
    var_costs_old.resize((*products).size());
    var_costs_original.resize((*products).size());
    var_costs_standard.resize((*products).size());
    units_produced.resize((*products).size());
    units_produced_old.resize((*products).size());
    for (unsigned int i=0;i < (*products).size();i++) {
        var_costs[i] = (*products)[i].getVarCost();
        var_costs_old[i] = var_costs[i];
        var_costs_original[i] = var_costs[i];
        var_costs_standard[i] = var_costs[i];
        units_produced[i]=0;
        units_produced_old[i]=0;
        if ((*products)[i].getPremiumLegitimation()) fixed_reference_production.push_back(0);
    }
    use_price_expectation=true;
    units_produced_for_prem_calc.resize((*products).size());
}

RegProductList::RegProductList(RegProductList& rh,RegGlobalsInfo* G,vector<RegProductInfo>& P)
        :g(G),products(&P) {
    obj_backup=NULL;
    var_costs=rh.var_costs;
    var_costs_original=rh.var_costs_original;
    var_costs_standard=rh.var_costs_standard;
    var_costs_old=rh.var_costs_old;
    units_produced=rh.units_produced;
    units_produced_old=rh.units_produced_old;
    use_price_expectation=rh.use_price_expectation;
    units_produced_for_prem_calc=rh.units_produced_for_prem_calc;
    fixed_reference_production=rh.fixed_reference_production;

}

double
RegProductList::getUnitsProducedOfType(int t) {
    double sum=0;
    for(unsigned int i=0;i<(*products).size();i++) {
      if((*products)[i].getProductType()==t)
        sum+=units_produced[i];
    }
    return sum;
}

double
RegProductList::getUnitsProducedOfGroup(int t) {
    double sum=0;
    for(unsigned int i=0;i<(*products).size();i++) {
      if((*products)[i].getProductGroup()==0)
        sum+=units_produced[i];
    }
    return sum;
}

void
RegProductList::debug(string filename) {
    ofstream out;
    out.open(filename.c_str(),ios::trunc);
    out << "Product List\n";
    for (unsigned int i=0;i<(*products).size();i++) {
        out << (*products)[i].debug().c_str()
        << "Farm Specific Costs:\t" << var_costs[i] << "\t"
        << "Var. Costs:\t" << var_costs[i] << "\t"
        << "Units Prodeced:\t" << units_produced[i] << "\n";
    }
    out.close();
}

void
RegProductList::setUnitsProducedOfNumber(int n,double p) {
    units_produced[n]=p;
}

double
RegProductList::getReturnOfType(int t) {
    double ret=0;
    for (unsigned int i=0;i<(*products).size();i++) {
        if ((*products)[i].getProductType()==t) {

            ret+=units_produced[i]*getPriceOfNumber(i);
        }
    }
    return ret;
}
double
RegProductList::getReturnOfNumber(int n) {
    return (units_produced[n]*getPriceOfNumber(n));
}
double  // quantity
RegProductList::getUnitsProducedOfNumber(int n) {
    return units_produced[n];
}

double  // monetary
RegProductList::getGrossMarginOfType(int t) {
    double gm=0;
    for (unsigned int i=0;i<(*products).size();i++) {
        if ((*products)[i].getProductType()==t) {
            gm+=units_produced[i]*getGrossMarginOfNumber(i);
        }
    }
    return gm;
}
void
RegProductList::updateCosts(double mc) {
    for (unsigned int i=0;i<(*products).size();i++) {
        if ((*products)[i].getProductType()==4) {       // ### ==5; geändert, damit sich der Management Faktor nur bei den Kühen ändert
            var_costs[i]*=mc;
            var_costs_original[i]=var_costs[i];
            var_costs_old[i]=var_costs[i];
        }
    }
}
void
RegProductList::changeVarCosts(double f, int investgroup) {
    // change var costs of selected (*products)
    for (unsigned int i=0;i<(*products).size();i++) {
        int productgroup = (*products)[i].getProductGroup();
        if ((productgroup==investgroup)&&(productgroup!=-1)) { // all products
            if (var_costs[i]>0 && f!=0) {
                if (productgroup==0) { // crop products using machinery; take original values as basis
                    setVarCostsOfNumber(i,(var_costs_original[i]-var_costs_original[i]*f));
                } else { // all other products
                    setVarCostsOfNumber(i,(var_costs[i]-var_costs[i]*f));
                }
            }
        }
    }
}
void
RegProductList::saveProductList() {
    for (unsigned int i=0;i<(*products).size();i++) {
        var_costs_old[i] = var_costs[i];
        units_produced_old[i]=units_produced[i];
    }
}
void
RegProductList::restoreProductList() {
    for (unsigned int i=0;i<(*products).size();i++) {
        var_costs[i] = var_costs_old[i];
        units_produced[i]=units_produced_old[i];
    }
}
double
RegProductList::getVarCostsOfNumber(int n) {
    return var_costs[n];
}

void
RegProductList::setVarCostsOfNumber(int n, double v) {
    var_costs[n] = v;
}

double
RegProductList::getPriceOfNumber(int n) {
    return (*products)[n].getPrice();
}
double
RegProductList::getPriceExpectationOfNumber(int n) {
    return (*products)[n].getPriceExpectation();
}
double
RegProductList::getGrossMarginOfNumber(int n) {
    return (*products)[n].getPrice() - var_costs[n];
//    return products[n].getPriceExpectation() - var_costs[n];
}
double
RegProductList::getStandardGrossMarginOfGroup(int g) {
    double standardgm = 0;
    for (unsigned int i=0;i<(*products).size();i++) {
        if ((*products)[i].getProductGroup()==g) {
            standardgm+=units_produced[i]*getStandardGrossMarginOfNumber(i);
        }
    }
    return standardgm;
}
double
RegProductList::getStandardGrossMarginOfNumber(int i) {
    return  (*products)[i].getPrice() - var_costs_standard[i];
}
double
RegProductList::getGrossMarginOfGroup(int g) {
    double gm = 0;
    for (unsigned int i=0;i<(*products).size();i++) {
        if ((*products)[i].getProductGroup()==g) {
            gm+=units_produced[i]*getGrossMarginOfNumber(i);
        }
    }
    return gm;
}

double
RegProductList::getVarCostsOfType(int n) {
    return var_costs[n];
}
void
RegProductList::setVarCostsOfType(int n,double v) {
    var_costs[n] = v;
}
double
RegProductList::getPriceOfType(int n) {
    return (*products)[n].getPrice();
}
double
RegProductList::getPriceExpectationOfType(int n) {
    return (*products)[n].getPriceExpectation();
}
int
RegProductList::calculateFarmClass() {
    // retrieve the total production yield of a farm class
    double mf = getStandardGmOfFarmType("ARABLE");
    double ve = getStandardGmOfFarmType("PIG/POULTRY");
    double fb = getStandardGmOfFarmType("GRASSLAND");
    double ag = mf + ve + fb;
    // if more than 50% of total gross margin is from pig and poultryp production
    // then the farm class is pig and poultry
    double pp = ve/ag;
    double g = fb/ag;
    double a = mf/ag;
    double x=2;
    double y=3;
    double teiler = x/y;
    if (pp>teiler) return 1;  // pig/poultry
    if (g>teiler) return 2;   // grassland
    if (a>teiler) return 3;   // arable
    if ((pp<teiler)&&(g<teiler)&&(a<teiler))
        return 4;             // mixed
    return -1;
}
double
RegProductList::getGmOfFarmType(string cl) {
    double ret=0;
    for (unsigned int i=0;i<(*products).size();i++) {
        if ((*products)[i].getClass()==cl) {
            ret+=units_produced[i]*getGrossMarginOfNumber(i);
        }
    }
    return ret;
}
double
RegProductList::getStandardGmOfFarmType(string cl) {
    double ret=0;
    for (unsigned int i=0;i<(*products).size();i++) {
        if ((*products)[i].getClass()==cl) {
            ret+=units_produced[i]*getStandardGrossMarginOfNumber(i);
        }
    }
    return ret;
}
double
RegProductList::getRevenueOfClass(string cl) {
    double rev = 0;
    for (unsigned int i=0;i<(*products).size();i++) {
        if ((*products)[i].getClass()==cl) {
            rev+=units_produced[i]*getPriceOfNumber(i);
        }
    }
    return rev;
}
double
RegProductList::getGmOrGmExpectedOfNumber(int n) {
    if (use_price_expectation)
        return (*products)[n].getPriceExpectation() - var_costs[n];
    else
        return (*products)[n].getPrice() - var_costs[n];
}
bool
RegProductList::setUsePriceExpectation(bool set) {
    bool changed;
    if (use_price_expectation==set)
        changed=false;
    else
        changed=true;
    use_price_expectation=set;
    return changed;
}

///////////////////////////////////////////////////////////////////////////////
/// nicht aktiv
void
RegProductList::expectLowerCosts(double factor) {
    // expect lower var costs for all products affected by investment objects
    for (unsigned int i=0;i<(*products).size();i++) {
        var_costs_old[i] = var_costs[i];
        int g = (*products)[i].getProductGroup();
        if ((g!=-1) && (var_costs[i] > 0)) {
            if (var_costs[i]>0 && factor!=0) {
                if (g==0) { // for machinery take original values as basis
                    setVarCostsOfNumber(i,(var_costs_original[i]-var_costs_original[i]*factor));
                } else { // all other products
                    setVarCostsOfNumber(i,(var_costs[i]-var_costs[i]*factor));
                }
            }
        }
    }
}

void RegProductList::fixReferencePeriod() {
    int j=0;
    for (unsigned int i=0;i<(*products).size();i++) {
        if ((*products)[i].getPremiumLegitimation()) {
            double av_prod=0;
            double count=units_produced_for_prem_calc[i].size();
            list<double>::iterator iter;
            for (iter = units_produced_for_prem_calc[i].begin();
                    iter != units_produced_for_prem_calc[i].end();
                    iter++) {
                av_prod+=(*iter);
            }
            av_prod/=count;
            fixed_reference_production[j]=av_prod;
            j++;
        }
    }
}

void RegProductList::calculateReferencePeriod() {
    for (unsigned int i=0;i<(*products).size();i++) {
        if ((*products)[i].getPremiumLegitimation()) {
            units_produced_for_prem_calc[i].push_back(units_produced[i]);
            if (units_produced_for_prem_calc[i].size()>(*products)[i].getReferencePremiumCalcTime())
                units_produced_for_prem_calc[i].pop_front();
        }
    }
}
void
RegProductList::adjustActivityLevel(double leeffect)   {
    if (units_produced[g->ST_EC_INTERESTTYPE]>0)  {
        double diff=leeffect+units_produced[g->ST_EC_INTERESTTYPE];
        if (diff>0) {
            units_produced[g->ST_EC_INTERESTTYPE]=diff;
        } else {
            units_produced[g->ST_EC_INTERESTTYPE]=0;
            units_produced[g->ST_BOR_INTERESTTYPE]=fabs(diff);
        }
    } else {
        units_produced[g->ST_BOR_INTERESTTYPE]+=fabs(leeffect);
    }
}
double
RegProductList::getTotalGrossMargin() {
    double gm=0;
    for (unsigned int i=0;i< ((*products).size());i++) {
        gm+= ((*products)[i].getPrice() - var_costs[i])*units_produced[i];
    }
    return gm;
}

double
RegProductList::getTotalLU(string cl) {
    double lu=0;
    for (unsigned int i=0;i< ((*products).size());i++) {
        if ((*products)[i].getClass()==cl) {
            lu+= (*products)[i].getLU()*units_produced[i];
        }
    }
    return lu;
}
double
RegProductList::getTotalLU() {
    double lu=0;
    for (unsigned int i=0;i< ((*products).size());i++) {
        lu+= (*products)[i].getLU()*units_produced[i];
    }
    return lu;
} 
double
RegProductList::getTotalGrossMarginExpectation() {
    double gm=0;
    for (unsigned int i=0;i< ((*products).size());i++) {
        gm+= ((*products)[i].getPriceExpectation() - var_costs[i])*units_produced[i];
    }
    return gm;
}

void
RegProductList::backup() {
    obj_backup=new RegProductList(*this);
}
void
RegProductList::restore() {
    RegProductList* tmp=obj_backup;
    *this=*obj_backup;
    obj_backup=tmp;
}
RegProductList::~RegProductList() {
    if (obj_backup) delete obj_backup;
}

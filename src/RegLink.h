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

//---------------------------------------------------------------------------
#ifndef RegLinkH
#define RegLinkH
#include "RegProduct.h"
#include "RegFarm.h"
//class RegFarmInfo;
#include "RegInvest.h"

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// hier entstehen in Keurze die Link Objekts mit denen wir Wete des LP's
// also mat_val , rhs und obj mit anderen Werten vernknuepen koennen

// Jeder Link hat eine Type
// 0:Market, 1:Invest, 2:Reference, 3:Number
class RegProductList;
class RegLinkObject {
public:
    RegLinkObject();
    ~RegLinkObject() {};
    virtual bool trigger();
    virtual int getSourceNumber();
    virtual int getDestNumber();
    virtual int getDestKind();
    virtual int getType();
    virtual string debug()=0;
protected:
    double res_value;
    int value_kind;
    int dest_kind;  //0: mat_val;
    //1: rhs;
    //2: obj;
    double *dest;
    int dest_number;
    int source_number;
    double factor;
    int type;
};


//soil service link
class RegLinkYieldObject : public RegLinkObject {
private:
    RegGlobalsInfo* g;
    RegFarmInfo * source;
public:
    ~RegLinkYieldObject() {};
    RegLinkYieldObject(int,int,int,int,double);
    void init(RegFarmInfo*,double*);
    bool trigger();

    string debug();
};

class RegLinkLandObject : public RegLinkObject {
private:
    RegGlobalsInfo* g;
    RegFarmInfo * source;
public:
    ~RegLinkLandObject() {};
    RegLinkLandObject(int,int,int,double);
    void init(RegFarmInfo*,double*);
    bool trigger();

    string debug();
};

class RegLinkMarketObject : public RegLinkObject {
private:
    RegProductList * source;
//    int value_kind; //0: getCostsOfNumber();
    //1: getpriceOfNumber();
    //2: getpriceExpectationOfNumber();
    //3: Grossmargin
public:
    ~RegLinkMarketObject() {};
    RegLinkMarketObject(int,int,int,int,double);
    void init(RegProductList*,double*);
    bool trigger();

    string debug();
};
class RegLinkInvestObject : public RegLinkObject {
private:
    RegInvestList * source;
public:
    ~RegLinkInvestObject() {};
    RegLinkInvestObject(int,int,int,int,double);
    void init(RegInvestList*,double*);
    bool trigger();

    string debug();
};

class RegLinkReferenceObject : public RegLinkObject {
private:
    double *source;
//    int source_number; //0: liquidity
    //1: milk
    //2: land
    //3: labour
    //4: financing_rule
    //5:arable land
    //6: grassLand
    //7: farm payment
    //8: farm payment
    //9: tranch1_width
    //10: tranch2_width
    //11: tranch3_width
    //12: tranch4_width
    //13: tranch5_width
    //14: tranch1_deg
    //15: tranch2_deg
    //16: tranch3_deg
    //17: tranch4_deg
    //18: tranch5_deg
public:
    ~RegLinkReferenceObject() {};
    RegLinkReferenceObject(int,int,int,double);
    void init(double*,double*);
    bool trigger();

    string debug();
};
class RegLinkNumberObject : public RegLinkObject {
private:
    double value;
public:
    ~RegLinkNumberObject() {};
    RegLinkNumberObject(int,int,double);
    void init(double*);
    bool trigger();

    string debug();
};
#endif

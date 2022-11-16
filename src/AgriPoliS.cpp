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
#include "RegManager.h"
#include "Agripolis.h"
//---------------------------------------------------------------------------


int main (int argc, char * argv[]) {
        RegGlobalsInfo *g= new RegGlobalsInfo();

        if (argc<2) {
           cout << "USAGE: agripolis.exe dirOfOptions  [ policyFileName [repeatNum] ] \n";
           return 1;
        }
        if (argc >2 ) g->POLICYFILE = argv[2];

		if (argc > 3 ) g->V = atoi(argv[3]);
		if (g->V !=0 ) { 
			g->INIT_OUTPUT = false;
		} else {
			g->INIT_OUTPUT = true;
		}
		g->SEED = g->V*119;

        optiondir= argv[1];
        gg = g;
        options(optiondir);
        
	   	RegManagerInfo *Manager = new RegManagerInfo(g);
        Manager->simulate();
		 
        return 0;
}

//---------------------------------------------------------------------------








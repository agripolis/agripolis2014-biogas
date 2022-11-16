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
#ifndef RegMessagesH
#define RegMessagesH
#define CM_DRAWPLOT  (WM_APP + 400)
#define CM_MANAGER_READY (WM_APP +410)
#define CM_UPDATE_INFO (WM_APP + 420)
#define CM_IMAGE_INIT (WM_APP + 430)
#define CM_STATE_INFO (WM_APP + 440)
#include <string>
//class RegFarmInfo;
using namespace std;
// attributes of plot necessary for plotting
struct PlA {                  // Plot appearance
    int col;
    int row;
    int number;
    int plot_colour;
    int state;
    int soil_type;
    string soil_name;
    int farm_id;
};
/*
struct TCMDrawPlot {
    Cardinal Msg;       // Erster Parameter: die Botschafts-ID
    Word Unused1;      // Der erste wParam
    Word Unused2;
    struct PlA *PlotAppearance;    // Der lParam
    Longint Result;    // Der Rückgabewert
};
struct TCMStateInfo {
    Cardinal Msg;       // Erster Parameter: die Botschafts-ID
    Word Unused1;      // Der erste wParam
    Word Unused2;
    string *state;    // Der lParam
    Longint Result;    // Der Rückgabewert
};
//*/
//---------------------------------------------------------------------------
#endif

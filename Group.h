#pragma once
#include <vector>
#include <cmath>
#include <string>
#include <iostream>
#include "Calculation.h"
#include "DataExtraction.h"
#include "Stocks.h"
#include <algorithm>
using namespace std;
using namespace fre;

bool cmp(const pair<string, Stocks>& a, const pair<string, Stocks>& b) {
    return a.second.GetSurprisePercent() > b.second.GetSurprisePercent();
}


void StockGroup(map<string, Stocks> &Beat_third,map<string, Stocks> &Meet_third,map<string, Stocks> &Miss_third, String global_skipped_tickers){
    map<string, Stocks> stockData;
    map<string, Stocks>:: iterator itr;
    LoadEarningsData(stockData);
    map<string, Stocks> updatedStockData;
    
    for(itr=stockData.begin(); itr!=stockData.end(); itr++)
    {
        if(std::find(global_skipped_tickers.begin(), global_skipped_tickers.end(), itr->first) != global_skipped_tickers.end()) //skipping stocks which do not have (2N+1) data points
            continue;
            
        updatedStockData[itr->first] = itr->second;
    }
    
    cout<<"Loading Earnings Data...."<<endl;
    // for (const auto& stock : stockData) {
    // // Get the surprise% attribute value of the stock
    //     double surprise_pct = stock.second.GetSurprisePercent();
    //     cout << stock.first << ": " << surprise_pct << endl;
    // }
    cout<<"Sorting based on Surprise Percent...."<<endl;
    // sort the map based on the values
    vector<pair<string, Stocks>> vec(updatedStockData.begin(), updatedStockData.end());
    sort(vec.begin(), vec.end(), cmp);

    // // print the sorted map
    // for (const auto& p : vec) {
    //     cout << p.first << ": " << p.second.GetSurprisePercent() << endl;
    // }
    int size = vec.size();
    int third = size / 3;
    Beat_third.clear();
    Meet_third.clear();
    Miss_third.clear();

    // insert the top third into Beat_third map
    Beat_third.insert(vec.begin(), vec.begin() + third);
    // insert the middle third into Meet_third map
    Meet_third.insert(vec.begin() + third, vec.begin() + 2 * third);
    // insert the bottom third into Miss_third map
    Miss_third.insert(vec.begin() + 2 * third, vec.end());
    
}
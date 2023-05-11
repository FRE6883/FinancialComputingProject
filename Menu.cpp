#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <map>
#include <string>
#include <chrono>
#include <cmath>
#include <sstream>
#include <thread>
#include <algorithm>
#include "Calculation.h"
#include "Stocks.h"
#include "Group.h"
#include "DataExtraction.h"
#include "Bootstrap.h"
#include "gnuplot.h"

using namespace std;
using namespace std::chrono;
using namespace fre;

typedef vector<string> String;
typedef vector<double> Vector;
typedef vector<Vector> Matrix;
typedef vector<Matrix> MatrixList;

String global_skipped_tickers;

String SetDates(String datelist, map<string, Stocks>& stock_map, int N, string group)
{
    String skipped_tickers;
    String tickers;
    Stocks *stock;
    std::string Start_date;
    std::string End_date;
    auto itr = stock_map.begin();
    for(;itr != stock_map.end(); itr++)
    {
        std::string ticker = (itr->first);
        stock = &(itr->second);
        auto itr2 = find(datelist.begin(), datelist.end(), stock->GetEarningsDate());
        int index = std::distance(datelist.begin(),itr2);
        if ((index + N) >= (int)datelist.size() and (index - N) < 0)
        {
            skipped_tickers.push_back(ticker);
            global_skipped_tickers.push_back(ticker);
            continue;
        }
        else if ((index + N) >= (int)datelist.size())
        {
            skipped_tickers.push_back(ticker);
            global_skipped_tickers.push_back(ticker);
            continue;
        }
        else
        {
            tickers.push_back(ticker);
        }
        Start_date = datelist[index - N];
        End_date = datelist[index + N];
        stock->SetStartDate(Start_date);
        stock->SetEndDate(End_date);
    }
    if(skipped_tickers.size()>0)
    {       
            cout<<"\nWarning: No. of stocks in " << group << " with no data for given N (will be skipped): "<<skipped_tickers.size()<<endl;
            cout<<"Tickers for these stocks: "<<endl;
            for(int i = 0; i < (int)skipped_tickers.size(); i++)
            {
                cout<<setw(8)<<setfill(' ')<<skipped_tickers[i]<<" ";
                if((i+1)%10 == 0)
                    cout<<endl;
                
            }
    }
    
    return tickers;
}


void Display(Stocks s, const map<string, Stocks>& IWV_map, string group)
{
    std::cout<<"Ticker: "<<s.GetTicker()<<endl;
	std::cout<<"Earnings Announcement Date: "<<s.GetEarningsDate()<<endl;
	std::cout<<"Earning Estimate: $"<<s.GetEstimatedEarnings()<<endl;
	std::cout<<"Reported Earnings per share: $"<<s.GetReportedEarnings()<<endl;
	std::cout<<"Earnings Surprise: $"<<s.GetSurpriseEarnings()<<endl;
	std::cout<<"Earnings surprise percent: "<<s.GetSurprisePercent()<<"%"<<endl;
    std::cout<<"Group: "<<group<<endl;
    
    std::cout<<"Stock Daily Prices Dates: \n";
    std::cout << "Number of dates: " << s.GetDate().size() << std::endl;
    cout << endl;
    cout<< "Dates : \n" ;
    String dates = s.GetDate();
    for(int i = 0; i < (int)dates.size(); i++)
            {
                cout<<setw(20)<<setfill(' ')<<dates[i]<<"     ";
                if((i+1)%10 == 0)
                    cout<<endl;
                
            }
    
    std::cout<<"\n\nStock Daily Prices: \n";
    Vector prices = s.GetACP();
     for(int i = 0; i < (int)prices.size(); i++)
            {
                cout<<setw(20)<<setfill(' ')<<prices[i]<<"     ";
                if((i+1)%10 == 0)
                    cout<<endl;
                
            }
            
    Vector Daily_return = cal_DailyRet(prices);
    std::cout<<"\n\nStock Daily Returns: \n";
    for(int i = 0; i < (int)Daily_return.size(); i++)
            {
                cout<<setw(20)<<setfill(' ')<<Daily_return[i]<<"  ";
                if((i+1)%10 == 0)
                    cout<<endl;
                
            }
            
    Vector Cumm_return = cal_CumulativeRet(prices);
    std::cout<<"\n\nStock Cumulative Returns: \n";
    for(int i = 0; i < (int)Cumm_return.size(); i++)
            {
                cout<<setw(20)<<setfill(' ')<<Cumm_return[i]<<"  ";
                if((i+1)%10 == 0)
                    cout<<endl;
                
            }
    
    map<string, double> stock_prices = s.GetPrices();
    map<string, double> IWV_prices = IWV_map.at("IWV").GetPrices();

    map<string, double> filtered_IWV_prices;
    for (const auto& date_price_pair : stock_prices) {
        if (IWV_prices.find(date_price_pair.first) != IWV_prices.end()) {
            filtered_IWV_prices[date_price_pair.first] = IWV_prices.at(date_price_pair.first);
        }
    }

    Vector stock_prices_values(stock_prices.size());
    Vector IWV_prices_values(filtered_IWV_prices.size());

    transform(stock_prices.begin(), stock_prices.end(), stock_prices_values.begin(), [](const pair<string, double>& p) { return p.second; });
    transform(filtered_IWV_prices.begin(), filtered_IWV_prices.end(), IWV_prices_values.begin(), [](const pair<string, double>& p) { return p.second; });
    Vector abnormal_returns = cal_ABRet(stock_prices_values, IWV_prices_values);
    std::cout<<"\n\nStock Abnormal Returns: \n";
    
     for(int i = 0; i < (int)abnormal_returns.size(); i++)
            {
                cout<<setw(20)<<setfill(' ')<<abnormal_returns[i]<<" ";
                if((i+1)%10 == 0)
                    cout<<endl;
                
            }
}


int main() 
{
    
    string val; //Menu input value to select 1 among 5 options
    int N = 0; //Number of days
    int integer_val = 0;
    int integer_grp = 0;
    string t, group;
    int g = 0;
    int success = 1;
    int num_samples = 40;
    String Dates;
    map<string, Stocks> IWVData;
    map<string, Stocks> Beat_third;
    map<string, Stocks> Meet_third;
    map<string, Stocks> Miss_third;

    
    
    while (true) 
    {
        cout << endl;
        cout << "\nMenu:" << endl;
        cout << "1. Enter N to retrieve 2N+1 historical price data for all stocks. (Allow x mins to load) " << endl;
        cout << "2. Pull information for one stock from one group. " << endl;
        cout << "3. Display AAR, AAR-STD, CAAR and CAAR-STD for one group. " << endl;
        cout << "4. Display gnuplot plot for CAAR of all 3 groups. " << endl;
        cout << "5. Exit program." << endl;
        cin >> val;
    
            if(val=="1")
            {
                while(true)
                {
                    cout << "Enter N value between 60 and 100: " << endl;
                    cin >> N;
                    integer_val = (int) N;
                    if(integer_val!= N)
                    {
                        cout<<"Error, only integer values are allowed, please enter an integer.";
                    }
                    else
                    {
                        if(N>=60 && N<=100 && !cin.fail())
                        {
                            cout<<"Data is loading, please allow around 2 mins to load!"<<endl;
        
                            FetchIWVData(IWVData, Dates);
                            StockGroup(Beat_third, Meet_third, Miss_third, global_skipped_tickers);
                            group="Beat Group";
                            String beat_tickers = SetDates(Dates, Beat_third, N, group); //Set end and start dates & return group tickers
                            group="Meet Group";
                            String meet_tickers = SetDates(Dates, Meet_third, N, group); //Set end and start dates & return group tickers
                            group="Miss Group";
                            String miss_tickers = SetDates(Dates, Miss_third, N, group); //Set end and start dates & return group tickers
                            
                            auto start = high_resolution_clock::now();
                            thread t1(FetchStockData,std::ref(Beat_third),beat_tickers);
                            thread t2(FetchStockData,std::ref(Meet_third),meet_tickers);
                            thread t3(FetchStockData,std::ref(Miss_third),miss_tickers);
                            t1.join();
                            t2.join();
                            t3.join();
                            auto stop = high_resolution_clock::now();
                            auto duration = duration_cast<seconds>(stop - start);
                            cout << endl;
                            cout <<"\nFetched data in seconds: "<<duration.count()<<endl;
                            
                            break;
                        }
                        else
                        {
                            cout << "Error, please enter an integer value between 60 and 100 " << endl;
                            N = 0;
                            cin.clear();
                            break;
                        }
                    } 
                }

            }
            else if(val=="2")
            {
                if(N>=60 && N<=100)
                {
                    while(true)
                    {
                        cout << "Please provide ticker of a stock: " << endl;
                        cin >> t;
                        cout << endl;
                        if(Beat_third.find(t) != Beat_third.end())
                        {
                            // Display Stock Details
                            std::string group = "Beat";
                            Display(Beat_third[t], IWVData, group);
                            std::cout << std::endl;
                            break;
                        }
                        else if (Meet_third.find(t) != Meet_third.end())
                        {
                            // Display Stock Details
                            std::string group = "Meet";
                            Display(Meet_third[t], IWVData, group);
                            std::cout << std::endl;
                            break;
                        }
                        else if (Miss_third.find(t) != Miss_third.end())
                        {
                            // Display Stock Details
                            std::string group = "Miss";
                            Display(Miss_third[t], IWVData, group);
                            std::cout << std::endl;
                            break;
                        }
                        
                        else
                        {
                            cout <<"Error, The ticker is not in our stock pool. Please enter other tickers." << endl;
                        }
                    }
                }
                else
                {
                    cout << "Data is not retrived yet, Retrieve data from Option 1." << endl;
                }
            }
            else if(val=="3")
            {
                if(N>=60 && N<=100)
                {
                    while(true)
                    {
                        cout <<endl <<  "1. Beat \t 2. Meet \t 3. Miss"<< endl;
                        cout << "Enter appropriate group no: " << endl;
                        cin >> g;
                        integer_grp = (int) g;
                        if(integer_grp != g)
                        {
                            cout<< "Input value isn't an integer." << endl;
                            cin.clear();
                        }
                        else
                        {
                            switch(g)
                            {
                                case 1:
                                {
                                    cout <<"Beat Group Bootstrap construct"<<endl;
                                    Bootstrap bootstrap(Beat_third, IWVData, N, num_samples);
                                    cout <<"Beat Group Bootstrap start"<<endl;
                                    bootstrap.PerformBootstrap();
                                    Vector avg_aar = bootstrap.GetAvgAAR();
                                    Vector avg_caar = bootstrap.GetAvgCAAR();
                                    Vector std_aar = bootstrap.GetStdAAR();
                                    Vector std_caar = bootstrap.GetStdCAAR();
                                    
                                    Vector test;
                                    
                                    cout <<"Summary: Beat Group: "<<endl;
                                    std::cout << "Average AAR:\n";
                                    for(int i = 0; i < (int)avg_aar.size(); i++)
                                    {
                                        cout<<setw(20)<<setfill(' ')<<avg_aar[i]<<"     ";
                                        if((i+1)%10 == 0)
                                            cout<<endl;
                                    }
                                    std::cout << "\n\nStandard Deviation AAR:\n";
                                     for(int i = 0; i < (int)std_aar.size(); i++)
                                    {
                                        cout<<setw(20)<<setfill(' ')<<std_aar[i]<<"     ";
                                        if((i+1)%10 == 0)
                                            cout<<endl;
                                    }
                                    std::cout << "\n\nAverage CAAR:\n";
                                    for(int i = 0; i < (int)avg_caar.size(); i++)
                                    {
                                        cout<<setw(20)<<setfill(' ')<<avg_caar[i]<<"     ";
                                        if((i+1)%10 == 0)
                                            cout<<endl;
                                    }
                                    std::cout << "\n\nStandard Deviation CAAR:\n";
                                    for(int i = 0; i < (int)std_caar.size(); i++)
                                    {
                                        cout<<setw(20)<<setfill(' ')<<std_caar[i]<<"     ";
                                        if((i+1)%10 == 0)
                                            cout<<endl;
                                    }
                                    std::cout << '\n';
                                    break;
                                    
                                }
                                case 2:
                                {
                                    Bootstrap bootstrap(Meet_third, IWVData, N, num_samples);
                                    bootstrap.PerformBootstrap();
                                    Vector avg_aar = bootstrap.GetAvgAAR();
                                    Vector avg_caar = bootstrap.GetAvgCAAR();
                                    Vector std_aar = bootstrap.GetStdAAR();
                                    Vector std_caar = bootstrap.GetStdCAAR();
                                    cout <<"Summary: Meet Group: "<<endl;
                                    std::cout << "Average AAR:\n";
                                    for(int i = 0; i < (int)avg_aar.size(); i++)
                                    {
                                        cout<<setw(20)<<setfill(' ')<<avg_aar[i]<<"     ";
                                        if((i+1)%10 == 0)
                                            cout<<endl;
                                    }
                                    std::cout << "\n\nStandard Deviation AAR:\n";
                                     for(int i = 0; i < (int)std_aar.size(); i++)
                                    {
                                        cout<<setw(20)<<setfill(' ')<<std_aar[i]<<"     ";
                                        if((i+1)%10 == 0)
                                            cout<<endl;
                                    }
                                    std::cout << "\n\nAverage CAAR:\n";
                                    for(int i = 0; i < (int)avg_caar.size(); i++)
                                    {
                                        cout<<setw(20)<<setfill(' ')<<avg_caar[i]<<"     ";
                                        if((i+1)%10 == 0)
                                            cout<<endl;
                                    }
                                    std::cout << "\n\nStandard Deviation CAAR:\n";
                                    for(int i = 0; i < (int)std_caar.size(); i++)
                                    {
                                        cout<<setw(20)<<setfill(' ')<<std_caar[i]<<"     ";
                                        if((i+1)%10 == 0)
                                            cout<<endl;
                                    }
                                    std::cout << '\n'; 
                                    break;
                                }
                                case 3:
                                {
                                    Bootstrap bootstrap(Miss_third, IWVData, N, num_samples);
                                    bootstrap.PerformBootstrap();
                                    Vector avg_aar = bootstrap.GetAvgAAR();
                                    Vector avg_caar = bootstrap.GetAvgCAAR();
                                    Vector std_aar = bootstrap.GetStdAAR();
                                    Vector std_caar = bootstrap.GetStdCAAR();
                                    cout <<"Summary: Miss Group: "<<endl;
                                    std::cout << "Average AAR:\n";
                                     for(int i = 0; i < (int)avg_aar.size(); i++)
                                    {
                                        cout<<setw(20)<<setfill(' ')<<avg_aar[i]<<"     ";
                                        if((i+1)%10 == 0)
                                            cout<<endl;
                                    }
                                    std::cout << "\n\nStandard Deviation AAR:\n";
                                      for(int i = 0; i < (int)std_aar.size(); i++)
                                    {
                                        cout<<setw(20)<<setfill(' ')<<std_aar[i]<<"     ";
                                        if((i+1)%10 == 0)
                                            cout<<endl;
                                    }
                                    std::cout << "\n\nAverage CAAR:\n";
                                    for(int i = 0; i < (int)avg_caar.size(); i++)
                                    {
                                        cout<<setw(20)<<setfill(' ')<<avg_caar[i]<<"     ";
                                        if((i+1)%10 == 0)
                                            cout<<endl;
                                    }
                                    std::cout << "\n\nStandard Deviation CAAR:\n";
                                     for(int i = 0; i < (int)std_caar.size(); i++)
                                    {
                                        cout<<setw(20)<<setfill(' ')<<std_caar[i]<<"     ";
                                        if((i+1)%10 == 0)
                                            cout<<endl;
                                    }
                                    std::cout << '\n';
                                    break;
                                }
                                default:
                                    cout << "Error, Please select valid output, 1-3" << endl;
                                    cin.clear();
                                    success = 0;
                            }
                            if(success == 1)
                            {
                            break;
                            }
                        }
                    }
                }
                else
                {
                    cout << "Data is not retrived yet, Retrieve data from Option 1." << endl;
                }
            }
            else if(val=="4")
            {
                if(N >= 60 && N<=100)
                {
                   Bootstrap bootstrap_beat(Beat_third, IWVData, N, num_samples);
                   Bootstrap bootstrap_meet(Meet_third, IWVData, N, num_samples);
                   Bootstrap bootstrap_miss(Miss_third, IWVData, N, num_samples);
                   bootstrap_beat.PerformBootstrap();
                   bootstrap_meet.PerformBootstrap();
                   bootstrap_miss.PerformBootstrap();
                   Vector avg_caar_beat = bootstrap_beat.GetAvgCAAR();
                   Vector avg_caar_meet = bootstrap_meet.GetAvgCAAR();
                   Vector avg_caar_miss = bootstrap_miss.GetAvgCAAR();
                   plot Plotter(avg_caar_beat, avg_caar_meet, avg_caar_miss,"CAAR");
                   Plotter.plotResult();
                }   

                else
                {
                    cout << "Data is not retrived yet, Retrieve data from Option 1." << endl;
                }
            }
            else if(val=="5")
                exit(0);
            else
            {
                cout << "Invalid menu input, Select 1-5. " << endl;
                cin.clear();
            }
 
    }
    return 0;
}


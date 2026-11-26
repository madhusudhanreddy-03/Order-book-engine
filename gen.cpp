#include <bits/stdc++.h>
using namespace std;

const long long ticks_per_rupee=100;

string formatPrice(long long price){
    long long whole=price/ticks_per_rupee;
    long long paise=price%ticks_per_rupee;
    string paise_str=to_string(paise);
    if(paise<10){
        paise_str='0'+paise_str;
    }
    return to_string(whole)+'.'+paise_str;
}

vector<string>bad_prices={
    "abc","10a.00","-5.00","0.00","100.555","1..5","."
};

int main(int argc,char* argv[]){
    if(argc<2){
        cerr<<"Usage: .\\gen.exe <seed>\n";
        return 1;
    }
    int seed=atoi(argv[1]);
    mt19937 rng(seed);
    // Generate between 1 and 30 commands
    int n=1+rng()%30;
    // Only BUY and SELL commands get an ID
    int order_count=0;
    for(int i=0;i<n;i++){
        int type=rng()%100;
        // Generate a cancel command with around 18% probability
        if(type<18){
            // The ID may belong to an existing order or may belong to a completed order which results in cancellation failed 
            // The ID may also be greater than the number of orders came so far in that case the order will result in cancellation failed
            int id=1+rng()%(order_count+2);
            cout<<"CANCEL "<<id<<'\n';
        }
        // Generate an invalid order with around 5% probability
        else if(type<23){
            string side;
            if(rng()%2==0){
                side="BUY";
            }
            else{
                side="SELL";
            }
            // Sometimes generate zero quantity, otherwise use an invalid price
            if(rng()%2==0){
                int price=9900+rng()%201;
                cout<<side<<" 0 "<<formatPrice(price)<<'\n';
            }
            else{
                int quantity=1+rng()%50;
                string price=bad_prices[rng()%bad_prices.size()];
                cout<<side<<" "<<quantity<<" "<<price<<'\n';
            }
            // Invalid orders also consume an ID
            order_count++;
        }
        else{
            string side;
            if(rng()%2==0){
                side="BUY";
            }
            else{
                side="SELL";
            }
            int quantity=1+rng()%50;
            // Generate a market order with around 10% probability
            if(rng()%100<10){
                cout<<side<<" "<<quantity<<" MKT\n";
            }
            else{
                // Keep prices close so that orders have a good chance of matching
                long long price=9900+rng()%201;
                cout<<side<<" "<<quantity<<" "<<formatPrice(price)<<'\n';
            }
            order_count++;
        }
    }
    return 0;
}
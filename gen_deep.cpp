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

int main(int argc,char* argv[]){
    if(argc<2){
        cerr<<"Usage: .\\gen_deep.exe <seed>\n";
        return 1;
    }
    int seed=atoi(argv[1]);
    mt19937 rng(seed);
    // Place 100000 orders without any trades
    for(int i=0;i<100000;i++){
        string side;
        long long price;
        if(rng()%2==0){
            side="BUY";
            price=9900+rng()%5;
        }
        else{
            side="SELL";
            price=10100+rng()%5;
        }
        int quantity=1+rng()%50;
        cout<<side<<" "<<quantity<<" "<<formatPrice(price)<<'\n';
    }
    // Prepare all order IDs
    vector<int>ids(100000);
    iota(ids.begin(),ids.end(),1);
    // Shuffle IDs to cancel orders in random order
    shuffle(ids.begin(),ids.end(),rng);
    // Cancel all 100000 orders
    for(int id:ids){
        cout<<"CANCEL "<<id<<'\n';
    }
    return 0;
}
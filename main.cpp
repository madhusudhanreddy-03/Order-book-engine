#include <bits/stdc++.h>
#include <cassert>
#include <list>
using namespace std;

// we are using integer ticks because if 0.1 and 0.2 are added then it will be 0.30000000000000004 in floating point representation but in integer ticks it will be 30 which is correct.
// So even if buyer and seller prices are same the trade will not happen because of floating point precision issues.
// So to avoid such floating point precision issues we are using integer ticks.

// An order simply tells how many stocks a person wants to buy or sell and the price they are willing to pay or accept. A buyer specifies the maximum price they can pay per stock, while a seller specifies the minimum price they are willing to accept.
// A market order is used when someone wants to buy or sell stocks immediately at the best price currently available in the market, without setting any specific price limit. The buyer is ready to pay the available price, while the seller is ready to accept the best available buying price. 

/*
Input:
    BUY|SELL quantity price
    BUY|SELL quantity MKT
    CANCEL id

Output:
    Invalid order/price and failed cancellation messages are printed while reading input.
    After processing all commands, all trades are printed first,
    followed by the remaining order book.
*/

const long long ticks_per_rupee=100;

enum class Side{ 
    Buy,
    Sell
};
struct Order{
    int id;
    Side side;
    long long price;
    int quantity;
};

// As the first priority is given to buyers whose bid is more we are adding a comparator to the bids map as by default it stores in asc order of keys here (prices) ...
// Every price level in the book must contain at least one order.
// Whenever the last order at a price level is removed, we erase that row.
map<long long,list<Order>,greater<long long>>bids;
map<long long,list<Order>>asks;
// This map stores each order using its ID, making cancellation faster without searching the entire order book
// Fully traded orders are removed from this map.
// Any remaining cleanup cases will be handled in week 2.
unordered_map<int,Order>mp;
struct Trade{
    int buyOrderId;
    int sellOrderId;
    long long price;
    int quantity;
};
vector<Trade>Trades;
struct Command{
    int type;        // 0 = order, 1 = cancel, 2 = invalid order, 3 = invalid price
    Order o;         // used when type is 0
    bool is_market;  // used when type is 0
    int cancel_id;   // used when type is 1
};
vector<Command>commands;
string formatPrice(long long price){
    string value;
    long long whole=price/ticks_per_rupee;
    long long paise=price%ticks_per_rupee;
    string paise_str=to_string(paise);
    if(paise<10){
        paise_str='0'+paise_str;
    }
    value=to_string(whole)+'.'+paise_str;
    return value;
}
void printOrder(const Order&o){
    string value=formatPrice(o.price);
    cout<<"#"<<o.id<<' '<<(o.side==Side::Buy?"BUY":"SELL")<<' '<<o.quantity<<' '<<'@'<<' '<<value<<'\n';
}
bool parse_price(const string &price_str,long long &price){
    price=0;
    if(price_str=="MKT"){
        return true;
    }
    for(auto c:price_str){
        if(!isdigit((unsigned char)c) and c!='.'){  // If price is negative or not a number then reject the order
            return false;
        }
    }
    int idx=-1;
    int cnt_dots=0;
    for(int i=0;i<(int)price_str.size();i++){
        if(price_str[i]=='.'){
            idx=i;
            cnt_dots++;
        }
    }
    int n=price_str.size(); 
    if((n-1-idx>2 or cnt_dots>1) and idx!=-1){
        return false; // If price has more than 2 decimal places then reject the order i.e. if price is not a multiple of tick size then reject the order
    }
    bool found=false;
    for(auto c:price_str){
        if(c=='.'){
            continue;
        }
        if(c!='0'){
            found=true;
        }
    }
    if(!found){
        return false; // If price is zero then reject the order
    }
    found=false;
    int cnt=0;
    for(auto c:price_str){
        if(c=='.'){
            found=true;
            continue;
        }
        if(found){
            cnt++;
        }
        if(price>((LLONG_MAX-(c-'0'))/10)){
            return false; // If price is greater than LLONG_MAX then reject the order
        }
        price=price*10+(c-'0');
    }
    if(cnt==1){
        if(price>LLONG_MAX/10){
            return false; // If price is greater than LLONG_MAX then reject the order
        }
        price*=10;
    }
    else if(cnt==0){
        if(price>LLONG_MAX/100){
            return false; // If price is greater than LLONG_MAX then reject the order
        }
        price*=100;
    }
    return true;
}
bool validate_order(const string &side_str,int quantity){
    if(quantity<=0){
        return false; // If quantity is negative or zero then reject the order
    }
    if(side_str!="BUY" and side_str!="SELL"){
        return false; // If side is not Buy or Sell then reject the order
    }
    return true;
}
void addOrder(const Order &o){
    if(o.side==Side::Buy){
        bids[o.price].push_back(o);
    }
    else{
        asks[o.price].push_back(o);
    }
}
// Every trade happens at the price of the order already present in the book. For market orders, we use LLONG_MAX (BUY) or 0 (SELL) so they can match at any available price. If some quantity is still left, we add it to the book only for limit orders.
void processOrder(Order o,bool is_market){
    if(o.side==Side::Buy){
        while(!asks.empty()){
            auto price=asks.begin()->first;
            auto &orders=asks.begin()->second;
            if(o.price<price){
                break;
            }
            while(o.quantity>0 and !orders.empty()){
                Order &ord=orders.front();
                if(o.quantity<=ord.quantity){
                    Trades.push_back({o.id,ord.id,ord.price,o.quantity});
                    ord.quantity-=o.quantity;
                    o.quantity=0;
                    if(!ord.quantity){
                        mp.erase(ord.id);
                        orders.pop_front();
                    }
                    else mp[ord.id].quantity=ord.quantity;                    
                    mp.erase(o.id);
                    break;
                }
                else{
                    Trades.push_back({o.id,ord.id,ord.price,ord.quantity});
                    o.quantity-=ord.quantity;
                    mp.erase(ord.id);
                    orders.pop_front();
                }
            }
            if(orders.empty()){
                asks.erase(asks.begin());
            }
            if(o.quantity<=0){
                break;
            }
        }
    }
    else{
        while(!bids.empty()){
            auto price=bids.begin()->first;
            auto &orders=bids.begin()->second;
            if(o.price>price){
                break;
            }
            while(o.quantity>0 and !orders.empty()){
                Order &ord=orders.front();
                if(o.quantity<=ord.quantity){
                    Trades.push_back({ord.id,o.id,ord.price,o.quantity});
                    ord.quantity-=o.quantity;
                    o.quantity=0;
                    if(!ord.quantity){
                        mp.erase(ord.id);
                        orders.pop_front();
                    }
                    else mp[ord.id].quantity=ord.quantity;
                    mp.erase(o.id);
                    break;
                }
                else{
                    Trades.push_back({ord.id,o.id,ord.price,ord.quantity});
                    o.quantity-=ord.quantity;
                    mp.erase(ord.id);
                    orders.pop_front();
                }
            }
            if(orders.empty()){
                bids.erase(bids.begin());
            }
            if(o.quantity<=0){
                break;
            }
        }
    }
    if(o.quantity>0 and !is_market){
        addOrder(o);
    }
    else{
        mp.erase(o.id);
    }
    return;
}
void cancelOrder(int id){
    if(!mp.count(id)){
        cout<<"Cancel failed: #"<<id<<" not found\n";
        return;
    }
    auto &o=mp[id];
    if(o.side==Side::Buy){
        long long price=o.price;
        auto row=bids.find(price);
        if(row==bids.end()){
            cout<<"Cancel failed: #"<<id<<" not found\n";
            return; // That order has already been fully traded
        }
        auto &orders=row->second;
        if(orders.empty()){
            bids.erase(row);
            cout<<"Cancel failed: #"<<id<<" not found\n";
            return;
        }
        bool found=false;
        for(auto it=orders.begin();it!=orders.end();++it){
            if(it->id==id){
                found=true;
                orders.erase(it);
                break;
            }
        }
        if(!found){
            cout<<"Cancel failed: #"<<id<<" not found\n";
            return;
        }
        if(orders.empty()){
            bids.erase(row);
        }
    }
    else{
        long long price=o.price;
        auto row=asks.find(price);
        if(row==asks.end()){
            cout<<"Cancel failed: #"<<id<<" not found\n";
           return; // That order has already been fully traded
        }
        auto &orders=row->second;
        if(orders.empty()){
            asks.erase(row);
            cout<<"Cancel failed: #"<<id<<" not found\n";
            return;
        }
        bool found=false;
        for(auto it=orders.begin();it!=orders.end();++it){
            if(it->id==id){
                found=true;
                orders.erase(it);
                break;
            }
        }
        if(!found){
            cout<<"Cancel failed: #"<<id<<" not found\n";
            return;
        }
        if(orders.empty()){
            asks.erase(row);
        }
    }
    mp.erase(id);
    return;
}
void print_book(){
    cout<<"BIDS:\n";
    for(auto &[price,orders]:bids){
        for(auto &o:orders){
            printOrder(o);
        }
    }
    cout<<"ASKS:\n";
    for(auto &[price,orders]:asks){
        for(auto &o:orders){
            printOrder(o);
        }
    }
}
void printTrades(){
    for(auto &t:Trades){
        string price=formatPrice(t.price);
        cout<<"TRADE buy #"<<t.buyOrderId<<" sell #"<<t.sellOrderId<<" "<<t.quantity<<" @ "<<price<<'\n';
    }
}

// If the prices are negative or zero the order will be rejected
// Prices that are not multiples of one tick (0.01) will be rejected.
// Prices which are greater than LLONG_MAX will be rejected
// Prices which are not having decimal places or whole number places are considered as valid prices
// A cancel command is not an order, so it does not get a new order ID.

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    long long p;
    assert(parse_price("101.05",p) && p == 10105);
    assert(parse_price("101.5",p)  && p == 10150);
    assert(parse_price("101",p)    && p == 10100);
    assert(parse_price("0.05",p)   && p == 5);
    assert(parse_price(".5",p)     && p == 50);
    assert(parse_price("5.",p)     && p == 500);
    assert(!parse_price("0.00",p));
    assert(!parse_price("1..5",p));
    assert(!parse_price(".",p));
    assert(!parse_price("101.055",p));
    assert(!parse_price("-5.00",p));
    assert(!parse_price("99999999999999999999",p));

    string cmd;
    int id_assign=1;
    commands.reserve(1000000);
    auto start1=chrono::steady_clock::now();
    while(cin>>cmd){
        if(cmd=="CANCEL"){
            int id;
            cin>>id;
            commands.push_back({1,Order{},false,id});
            //cancelOrder(id);
            continue;
        }
        string price_str;
        int quantity;
        cin>>quantity>>price_str;
        string side_str=cmd;
        Order o;
        o.id=id_assign;
        id_assign++;
        // assigning id to every order even if it is not valid  
        if(!validate_order(side_str,quantity)){
            commands.push_back({2,Order{},false,0});
            //cout<<"Invalid order\n";
            continue;
        }
        if(!parse_price(price_str,o.price)){
            commands.push_back({3,Order{},false,0});
            //cout<<"Invalid price\n";
            continue;
        }
        bool is_market=false;
        if(price_str=="MKT"){
            if(side_str=="SELL"){
                o.price=0;
            }
            else o.price=LLONG_MAX;
            is_market=true;
        }
        o.quantity=quantity;
        if(side_str=="BUY"){
            o.side=Side::Buy;
        }
        else if(side_str=="SELL"){
            o.side=Side::Sell;
        }
        commands.push_back({0,o,is_market,0});
        //mp[o.id]=o;
        //processOrder(o,is_market);
    }
    auto end1=chrono::steady_clock::now();
    vector<long long>lat;
    lat.reserve(commands.size());
    auto start2=chrono::steady_clock::now();
    for(auto &c:commands){
        if(c.type==0){
            auto t1=chrono::steady_clock::now();
            mp[c.o.id]=c.o;
            processOrder(c.o,c.is_market);
            auto t2=chrono::steady_clock::now();
            lat.push_back(chrono::duration_cast<chrono::nanoseconds>(t2-t1).count());
        }
        else if(c.type==1){
            cancelOrder(c.cancel_id);
        }
        else if(c.type==2){
            cout<<"Invalid order\n";
        }
        else if(c.type==3){
            cout<<"Invalid price\n";
        }
    }
    auto end2=chrono::steady_clock::now();
    sort(lat.begin(),lat.end());
    int sz=lat.size();
    double p50=lat[(int)(0.50*sz)]/1000.0;
    double p99=lat[(int)(0.99*sz)]/1000.0;
    double p999=lat[(int)(0.999*sz)]/1000.0;
    double mx=lat.back()/1000.0;
    cerr<<"P50 latency: "<<p50<<" us\n";
    cerr<<"P99 latency: "<<p99<<" us\n";
    cerr<<"P99.9 latency: "<<p999<<" us\n";
    cerr<<"Maximum latency: "<<mx<<" us\n";
    printTrades();
    print_book();
    double secs1=chrono::duration<double>(end1-start1).count();
    double secs2=chrono::duration<double>(end2-start2).count();
    long long total_orders=id_assign-1;
    cerr<<"Load time: "<<secs1<<" seconds\n";
    cerr<<"Matching time: "<<secs2<<" seconds\n";
    if(secs2>0){
        cerr<<"Orders per second: "<<(long long)total_orders/secs2<<'\n';
    }
}
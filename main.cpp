#include <bits/stdc++.h>
#include <cassert>
#include <list>
using namespace std;

// we are using integer ticks because if 0.1 and 0.2 are added then it will be 0.30000000000000004 in floating point representation but in integer ticks it will be 30 which is correct.
// So even if buyer and seller prices are same the trade will not happen because of floating point precision issues.
// So to avoid such floating point precision issues we are using integer ticks.

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
map<long long,list<Order>,greater<long long>>bids;
map<long long,list<Order>>asks;
unordered_map<int,Order>mp;
struct Trade{
    int buyOrderId;
    int sellOrderId;
    long long price;
    int quantity;
};
vector<Trade>Trades;
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
                if(o.quantity<ord.quantity){
                    Trades.push_back({o.id,ord.id,ord.price,o.quantity});
                    ord.quantity-=o.quantity;
                    o.quantity=0;
                    break;
                }
                else{
                    Trades.push_back({o.id,ord.id,ord.price,ord.quantity});
                    o.quantity-=ord.quantity;
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
                if(o.quantity<ord.quantity){
                    Trades.push_back({ord.id,o.id,ord.price,o.quantity});
                    ord.quantity-=o.quantity;
                    o.quantity=0;
                    break;
                }
                else{
                    Trades.push_back({ord.id,o.id,ord.price,ord.quantity});
                    o.quantity-=ord.quantity;
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
// Prices which are not multiples of tick_size will be rejected
// Prices which are greater than LLONG_MAX will be rejected
// Prices which are not having decimal places or whole number places are considered as valid prices
// As cancel order is not an new market order we are not assigning a new id to that order

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
    while(cin>>cmd){
        if(cmd=="CANCEL"){
            int id;
            cin>>id;
            cancelOrder(id);
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
            cout<<"Invalid order\n";
            continue;
        }
        if(!parse_price(price_str,o.price)){
            cout<<"Invalid price\n";
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
        mp[o.id]=o;
        processOrder(o,is_market);
    }
    printTrades();
    print_book();
}
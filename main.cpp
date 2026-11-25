#include <bits/stdc++.h>
#include <cassert>
using namespace std;

// we are using integer ticks because if 0.1 and 0.2 are added then it will be 0.30000000000000004 in floating point representation but in integer ticks it will be 30 which is correct.
// So even if buyer and seller prices are same the trade will not happen because of floating point precision issues.
// So to avoid such floating point precision issues we are using integer ticks.
const long long ticks_per_rupee=100;

enum class Side{ 
    Buy,
    Sell
};
struct Order {
    int id;
    Side side;
    long long price;
    int quantity;
};
map<long long,vector<Order>,greater<long long>>bids;
map<long long,vector<Order>>asks;
struct Trade {
    int buyOrderId;
    int sellOrderId;
    long long price;
    int quantity;
};
void printOrder(const Order&o){
    string value;
    long long whole=o.price/ticks_per_rupee;
    long long paise=o.price%ticks_per_rupee;
    string paise_str=to_string(paise);
    if(paise<10){
        paise_str='0'+paise_str;
    }
    value=to_string(whole)+'.'+paise_str;
    cout<<"#"<<o.id<<' '<<(o.side==Side::Buy?"BUY":"SELL")<<' '<<o.quantity<<' '<<'@'<<' '<<value<<'\n';
}
bool parse_price(const string &price_str,long long &price){
    price=0;
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
void addorder(const Order &o){
    if(o.side==Side::Buy){
        bids[o.price].push_back(o);
    }
    else{
        asks[o.price].push_back(o);
    }
}
void print_book(){
    cout<<"BIDS\n";
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


// If the prices are negative or zero the order will be rejected
// Prices which are not multiples of tick_size will be rejected
// Prices which are greater than LLONG_MAX will be rejected
// Prices which are not having decimal places or whole number places are considered as valid prices

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

    string side_str,price_str;
    int quantity;
    int id_assign=1;
    while(cin>>side_str>>quantity>>price_str){
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
        o.quantity=quantity;
        if(side_str=="BUY"){
            o.side=Side::Buy;
        }
        else if(side_str=="SELL"){
            o.side=Side::Sell;
        }
        addorder(o);
    }
    print_book();
}
#include <bits/stdc++.h>
using namespace std;

// brute force version of the order book.
// this one is slow on purpose. all resting orders (buys and sells together) are kept in one vector
// and for every incoming order we just scan the whole vector to find the best order on the other side.
// it is only used to check main.cpp, both programs must print exactly the same output for any input.
// input/output part is same as main.cpp, only the matching and cancel logic is written separately.

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
struct Trade{
    int buyOrderId;
    int sellOrderId;
    long long price;
    int quantity;
};

// every order which is still waiting, in the order they arrived
vector<Order>book;
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
        if(!isdigit((unsigned char)c) and c!='.'){
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
        return false;
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
        return false;
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
            return false;
        }
        price=price*10+(c-'0');
    }
    if(cnt==1){
        if(price>LLONG_MAX/10){
            return false;
        }
        price*=10;
    }
    else if(cnt==0){
        if(price>LLONG_MAX/100){
            return false;
        }
        price*=100;
    }
    return true;
}
bool validate_order(const string &side_str,int quantity){
    if(quantity<=0){
        return false;
    }
    if(side_str!="BUY" and side_str!="SELL"){
        return false;
    }
    return true;
}

// returns the index of the best order on the opposite side of o, or -1 if there is none.
// for a buyer the best seller is the cheapest one, for a seller the best buyer is the one paying most.
// if two orders have the same price the one with smaller id wins, because ids are given in arrival order
// so smaller id means it has been waiting longer.
int findBest(const Order &o){
    int best=-1;
    for(int i=0;i<(int)book.size();i++){
        if(book[i].side==o.side){
            continue;
        }
        if(best==-1){
            best=i;
            continue;
        }
        bool better=false;
        if(o.side==Side::Buy){
            if(book[i].price<book[best].price){
                better=true;
            }
        }
        else{
            if(book[i].price>book[best].price){
                better=true;
            }
        }
        if(book[i].price==book[best].price and book[i].id<book[best].id){
            better=true;
        }
        if(better){
            best=i;
        }
    }
    return best;
}

// keep trading with the best order on the other side until o is filled or prices stop crossing.
// trade always happens at the price of the order which was already waiting.
// market orders already come with price LLONG_MAX (buy) or 0 (sell) from main, so the price check never stops them.
void processOrder(Order o,bool is_market){
    while(o.quantity>0){
        int best=findBest(o);
        if(best==-1){
            break;
        }
        if(o.side==Side::Buy and book[best].price>o.price){
            break;
        }
        if(o.side==Side::Sell and book[best].price<o.price){
            break;
        }
        int qty=min(o.quantity,book[best].quantity);
        if(o.side==Side::Buy){
            Trades.push_back({o.id,book[best].id,book[best].price,qty});
        }
        else{
            Trades.push_back({book[best].id,o.id,book[best].price,qty});
        }
        o.quantity-=qty;
        book[best].quantity-=qty;
        if(book[best].quantity==0){
            book.erase(book.begin()+best);
        }
    }
    // only limit orders wait in the book, whatever is left of a market order is dropped
    if(o.quantity>0 and !is_market){
        book.push_back(o);
    }
}

// just search the whole book for that id
void cancelOrder(int id){
    for(int i=0;i<(int)book.size();i++){
        if(book[i].id==id){
            book.erase(book.begin()+i);
            return;
        }
    }
    cout<<"Cancel failed: #"<<id<<" not found\n";
}

// main.cpp prints bids from highest price and asks from lowest price,
// and inside the same price the older order first. so we sort the same way here.
bool cmp_bids(const Order &a,const Order &b){
    if(a.price!=b.price){
        return a.price>b.price;
    }
    return a.id<b.id;
}
bool cmp_asks(const Order &a,const Order &b){
    if(a.price!=b.price){
        return a.price<b.price;
    }
    return a.id<b.id;
}
void print_book(){
    vector<Order>buys,sells;
    for(auto &o:book){
        if(o.side==Side::Buy){
            buys.push_back(o);
        }
        else{
            sells.push_back(o);
        }
    }
    sort(buys.begin(),buys.end(),cmp_bids);
    sort(sells.begin(),sells.end(),cmp_asks);
    cout<<"BIDS:\n";
    for(auto &o:buys){
        printOrder(o);
    }
    cout<<"ASKS:\n";
    for(auto &o:sells){
        printOrder(o);
    }
}
void printTrades(){
    for(auto &t:Trades){
        string price=formatPrice(t.price);
        cout<<"TRADE buy #"<<t.buyOrderId<<" sell #"<<t.sellOrderId<<" "<<t.quantity<<" @ "<<price<<'\n';
    }
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

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
        // same id rule as main.cpp, every order line gets an id even if it is rejected
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
        processOrder(o,is_market);
    }
    printTrades();
    print_book();
}